@files = <*.doc>;
foreach $file (@files) {
	print "Converting $file\n";
	system("/usr/bin/perl convert1.pl $file");
}
