/***************************************************************
 *
 * Copyright (C) 1990-2008, Condor Team, Computer Sciences Department,
 * University of Wisconsin-Madison, WI.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you
 * may not use this file except in compliance with the License.  You may
 * obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ***************************************************************/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <stdio.h>
#include "write_user_log.h"
#include "read_user_log.h"
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

int
WriteStateFile( const ReadUserLog::FileState &state, const char *state_file )
{
	int		errors = 0;

	int	fd = open( state_file,
				   O_WRONLY|O_CREAT,
				   S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP );
	if ( fd < 0 ) {
		fprintf( stderr, "ERROR: Failed to open state file %s\n", state_file );
		return 1;
	}
	if ( write( fd, state.buf, state.size ) != state.size ) {
		fputs( "ERROR: Failed writing state file\n", stderr );
		errors++;
	}
	close( fd );
	return errors;
}

int
ReadEventLog( const char *event_log, int num_events, const char *state_file )
{
	int		errors = 0;
	int		status;

	// Create & initialize the state
	ReadUserLog::FileState	state;
	ReadUserLog::InitFileState( state );

	ReadUserLog	reader( event_log );
    reader.GetFileState( state );
	errors += WriteStateFile( state, state_file );

	// This *should* fail
	status = reader.initialize( state );
	if ( !status ) {
		printf( "good: reader won't let us re-initialize\n" );
	}
	else {
		fprintf( stderr, "ERROR: Reader let us re-intialize\n" );
		errors++;
	}

	// Now, try to read the events
	int		num_read = 0;
	for( int i = 0;  i < num_events;  i++ ) {
		ULogEvent			*event = NULL;
		ULogEventOutcome	 outcome = reader.readEvent(event);
		if ( outcome != ULOG_OK ) {
			fprintf( stderr, "ERROR: Failed to read eventlog event %d\n", i );
			errors++;
		}
		else {
			num_read++;
		}

		printf( "Writing to state file %s\n", state_file );
		reader.GetFileState(state);
		errors += WriteStateFile( state, state_file );
	}
	if ( !errors ) {
		printf( "Read %d events from %s\n", num_read, event_log );
	}

	return errors;
}

int
WriteEventLog( const char *event_log, int &num_events )
{
	int		errors = 0;
	UserLog	writer("owner", event_log, 1, 1, 1, false );

	num_events = 0;

	SubmitEvent submit;
	strcpy(submit.submitHost, "<127.0.0.1:1234>");
	submit.submitEventLogNotes = strdup("Log info");
	submit.submitEventUserNotes = strdup("User info");
	if ( !writer.writeEvent(&submit) ) {
		fprintf( stderr, "Failed to write submit event\n");
		errors++;
	}
	else {
		num_events++;
	}

	ExecuteEvent execute;
	strcpy(execute.executeHost, "<127.0.0.1:2345>");
	if ( !writer.writeEvent(&execute) ) {
		fprintf( stderr, "Failed to write execute event\n");
		errors++;
	}
	else {
		num_events++;
	}

	struct rusage ru;
	JobTerminatedEvent jobterminated;
	jobterminated.normal = false;
	jobterminated.signalNumber = 9;
	jobterminated.returnValue = 4;
	jobterminated.run_remote_rusage = ru;
	jobterminated.total_remote_rusage = ru;
	jobterminated.recvd_bytes = 200000;
	jobterminated.sent_bytes = 400000;
	jobterminated.total_recvd_bytes = 800000;
	jobterminated.total_sent_bytes = 900000;
	jobterminated.setCoreFile( "core" );
	if ( !writer.writeEvent(&jobterminated) ) {
		fprintf( stderr, "Failed to write execute event\n");
		errors++;
	}
	else {
		num_events++;
	}

	return errors;
}

int
main(int argc, const char **argv)
{
	int		errors = 0;
	int		e;
	int		num_events = 0;

	// Dirt simple command line handling
	if ( argc != 3 ) {
		fprintf( stderr, "usage: reader <log-file> <state-file>\n" );
		exit(1);
	}
	const char	*event_file = argv[1];
	const char	*state_file = argv[2];

	// Create the eventlog file
	e = WriteEventLog( event_file, num_events );
	errors += e;
	if ( e ) {
		fprintf( stderr, "Failed to write eventlog\n" );
		exit(1);
	}
	printf( "Wrote %d events to %s\n", num_events, event_file );

	// Read the eventlog
	e = ReadEventLog( event_file, num_events, state_file );
	errors += e;
	if ( e ) {
		fprintf( stderr, "Failed to read eventlog\n" );
	}
	
	if ( errors ) {
		fprintf( stderr, "%d Errors detected\n", errors );
		exit( 1 );
	}
	else {
		printf( "Passed all tests\n" );
	}
	exit( 0 );
}
