#!/usr/bin/perl

######################################################################
# $Id: remote_post_always.pl,v 1.1.2.4 2004-06-24 19:39:08 wright Exp $
# post script for Condor testsuite that is run regardless of the
# testsuite end status  
######################################################################

######################################################################
# set up path 
######################################################################

my $BaseDir = $ENV{BASE_DIR} || die "BASE_DIR not in environment!\n";
print "PATH is $ENV{PATH}\n";

######################################################################
# kill test suite personal condor daemons
######################################################################

$pid_file = "$BaseDir/condor_master_pid";
# Get master PID from file
open (PIDFILE, "$pid_file") || die "cannot open $pid_file for reading\n";
while (<PIDFILE>){
  chomp;
  $master_pid = $_;
}
close PIDFILE;

# probably try to stop more gracefully, then wait 30 seconds and kill if necessary
print "KILLING Personal Condor master daemon (pid: $master_pid)\n";
kill 15, $master_pid;

sub verbose_system {

  my @args = @_;
  my $rc = 0xffff & system @args;

  printf "system(%s) returned %#04x: ", @args, $rc;

  if ($rc == 0) {
   print "ran with normal exit\n";
   return $rc;
  }
  elsif ($rc == 0xff00) {
   print "command failed: $!\n";
   return $rc;
  }
  elsif (($rc & 0xff) == 0) {
   $rc >>= 8;
   print "ran with non-zero exit status $rc\n";
   return $rc;
  }
  else {
   print "ran with ";
   if ($rc &   0x80) {
       $rc &= ~0x80;
       print "coredump from ";
   return $rc;
   }
   print "signal $rc\n"
  }
}

