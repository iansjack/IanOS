open(FILEMAP, "HTML/HTML/FILEMAP") || die "Can't open FILEMAP\n";
while (<FILEMAP>) {
	if (/(.*[s]\b)\t(.*)/) {
		chomp $2;
		push(@words, $1);
		$assoc{$1} = $2;
		print "$1 : $assoc{$1}\n";
	}
}
close(FILEMAP);

open (OUTPUT, ">>HTML/HTML/MAP");
foreach $file (@words) {
	$nfile = $assoc{$file};
	open (INPUT, "HTML/HTML/$nfile") || die "Can't open $nfile\n";
	print "Processing $file\n";
	while (<INPUT>) {
		if (/id=\'(\w+)\'.+\s+(\w+):/) {
			print OUTPUT "$2\t$nfile\#$1\n";
		}
	}
	close INPUT;
}
close OUTPUT;
