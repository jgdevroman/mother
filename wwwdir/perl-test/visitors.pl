#!/usr/bin/perl -w

use strict;
use warnings;

my $visits = 0;
if ( open(VISITS, "< /tmp/$<_visits.txt") ) {
	$visits = <VISITS>;
	chomp($visits);
	close(VISITS);
}

$visits++;

print "HTTP/1.0 200 OK\n";
print "\n";

print <<EOT;
<html>
 <head>
  <title>Visitor Counter</title>
 </head>
 <body>
  <h1>Visitor Counter</h1>
  This page was visited <i>$visits</i> times.
 </body>
</html>
EOT

if ( open(VISITS, "> /tmp/$<_visits.txt") ) {
	print VISITS $visits;
	close(VISITS);
}
