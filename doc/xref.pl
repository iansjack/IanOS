open(FILEMAP, "HTML/HTML/FILEMAP") || die "Can't open FILEMAP\n";
while (<FILEMAP>) {
	if (/(.*s\b)\t(.*)/) {
		push(@assembler, $1);
		$assembler_assoc{$1} = $2;
	}
}
close(FILEMAP);
	
open(FILEMAP, "HTML/HTML/FILEMAP") || die "Can't open FILEMAP\n";
while (<FILEMAP>) {
	if (/(.*h\b)\t(.*)/) {
		chomp($2);
		push(@words, $1);
		$assoc{$1} = $2;
	}
}
close(FILEMAP);
	
open(MAP, "HTML/HTML/MAP") || die "Can't open MAP\n";
while (<MAP>) {
	/(\w+)\t(.*)/;
	chomp($2);
	push(@words, $1);
	$assoc{$1} = $2;
}
close(MAP);

foreach $file (@assembler) {
	$nfile = "HTML/HTML/$assembler_assoc{$file}";
	print "Processing $file  \n";
	open(ASSEMBLY, $nfile) || die "Can't open $nfile\n";
	open(TEMP, ">temp")  || die "Can't open temp\n";
	while(<ASSEMBLY>) {
		foreach $i (@words) {
			if (/\b$i\b/) {
				$sub = "<a href=\"../" . $assoc{$i} . "\">" . $i . "</a>";
				s/\b$i\b/$sub/g;
				}
			}
		print TEMP $_;
	}
	close (TEMP);
	close (ASSEMBLY);
	unlink($nfile) || warn "Can't delete $nfile: $!";
	rename("temp", $nfile) || warn "Can't rename temp to $nfile: $!";
}

