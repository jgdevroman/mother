#!/usr/bin/perl -w

use strict;
use warnings;

print "HTTP/1.0 200 OK\n";
print "\n";

print "<html><head>\n";
print "<title>Recursive Directory Listing</title>\n";
print "</head><body>\n";
print "<h1>Recursive Directory Listing</h1>\n";

sub showDir {
	my $dir = shift;
	my $basePath = shift;

	my @files = glob("$dir/*");
	for ( sort @files ) {
		my $file = $_;
		if ( -l "$file" ) {
			next;
		}

		if ( -d "$file" || -f "$file" ) {
			my $url = substr $file, length($basePath)+1;
			my $filename = substr $file, length($dir)+1;
			my $spaces = "&nbsp;"x(length($dir)-length($basePath));

			print "$spaces<a href=\"$url\">$filename</a><br>\n";

			if ( -d "$file" ) {
				showDir($file, $basePath);
			}
		}
	}
}

my $dir = $0;
$dir =~ s/\/[^\/]*$/\//;

showDir($dir, $dir);

print "</body></html>\n";
