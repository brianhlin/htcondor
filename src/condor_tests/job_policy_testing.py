import time
import textwrap
from pathlib import Path

import htcondor

from ornithology import (
    config,
    standup,
    Condor,
    action,
    ClusterState,
)


@config
def condor_config():
    config = {
        "STARTD_DEBUG": "D_SUB_SECOND D_FULLDEBUG D_JOB",
        "MAXJOBRETIREMENTTIME": 300,
    }
    raw_config = textwrap.dedent(
        """
        use feature : PartitionableSlot
        use policy : HOLD_IF_MEMORY_EXCEEDED
    """
    )
    return {"config": config, "raw_config": raw_config}


@standup
def condor(condor_config, test_dir):
    with Condor(test_dir / "condor", **condor_config) as condor:
        yield condor


@standup
def job_file(test_dir):
    job_file = test_dir / "condition_sleep.py"
    contents = textwrap.dedent(
        """
        import os
        import sys
        import time
        import signal

        signal.signal( signal.SIGTERM, signal.SIG_IGN )
        killFile = sys.argv[1]
        maxSleep = sys.argv[2]

        for i in range( 0, int(maxSleep) ):
            if os.path.isfile( killFile ):
                memoryWaste = list( range( 0, 1024 * 1024 * 16 ) )
            time.sleep( 1 )

        sys.exit( 0 )
        """
    )
    with open(str(job_file), "w") as f:
        f.write(contents)
    return job_file


@action
def kill_file(test_dir):
    return test_dir / "killfile"


@action
def job_parameters(job_file, kill_file):
    return {
        "executable": "/usr/bin/env",
        "transfer_executable": "false",
        "should_transfer_files": "true",
        "transfer_input_files": str(job_file),
        "universe": "vanilla",
        "arguments": "python {} {}.$(CLUSTER).$(PROCESS) 3600".format(
            str(job_file), kill_file
        ),
        "request_memory": 1,
        "log": "test_job_policy.log",
    }



# If we didn't remove the job during tear-down, Ornithology would have to
# wait for its whole shutdown time-out, because HTCondor will be waiting
# for MAX_JOB_RETIREMENT_TIME.  That saves us ~40 seconds per instance.
@action
def job(condor, job_parameters):
    job = condor.submit(job_parameters, count=1)

    yield job

    job.remove()


def ask_to_misbehave(job, proc, kill_file):
    Path("{}.{}.{}".format(kill_file, job.clusterid, proc)).touch()


def issue_drain_command(condor):
    with condor.use_config():
        hostname = htcondor.param["FULL_HOSTNAME"]

    rv = condor.run_command(["condor_drain", "-start", "IsBackfill == true", hostname])
    assert rv.returncode == 0


def wait_for_draining_to_start(condor):
    collector = condor.get_local_collector()
    for delay in range(1, 20):
        time.sleep(1)
        ads = collector.query(
            htcondor.AdTypes.Startd, True, ["Name", "State", "Activity"]
        )

        foundClaimedRetiring = False
        for ad in ads:
            if ad["Name"].startswith("slot1@"):
                continue
            if ad["State"] == "Claimed" and ad["Activity"] == "Retiring":
                foundClaimedRetiring = True
                break

        if foundClaimedRetiring:
            break

    assert foundClaimedRetiring


# See comment about job(), above.
@action
def backfill_job(condor, job, job_parameters):
    assert job.wait(
        condition=ClusterState.all_running,
        timeout=60,
        fail_condition=ClusterState.any_terminal,
    )

    issue_drain_command(condor)
    wait_for_draining_to_start(condor)

    job_parameters["+IsBackfill"] = "true"
    backfill_job = condor.submit(job_parameters, count=1)

    yield backfill_job

    backfill_job.remove()

