open(FILEMAP, "HTML/HTML/FILEMAP") || die "Can't open FILEMAP\n";
while (<FILEMAP>) {
	if (/(.*[chs]\b)\t(.*)/) {
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

@files = <*.doc>;
foreach $file (@files) {
	print "Converting $file\n";

	open (INPUT, $file) || die "can't open $file";
	$file =~ s/(\w+)\.doc/\1\.html/;
	open (OUTPUT, ">HTML\/$file") || die "can't open temp";
	while (<INPUT>) {
		while (/<</) {
			s/<<A\s+([\w:\/\.\@\-]+)\s+(.+?)>>/<a href=\"$1\">$2<\/a>/;
			s/<<PAR>>/<br><br>/;
			s/<<HR>>/<hr class=\"normal\">/;
			s/<<CONT>>/<div class=\"contents\"><a href=\"code.html">Contents<\/a><\/div>/;
			s/<<CONSTR>>/<span class=\"underconst\">This page is still under construction.<\/span><br>/;
			s/<<C\s+(.+?)>>/<span class="code">$1<\/span>/;
			s/<<S\s+(.+?)>>/<span class="sans">$1<\/span>/;
			s/<<I\s+(.+?)>>/<span class="italic">$1<\/span>/;
			s/<<H1\s+(.+?)>>/<h1 class=\"normal\">$1<\/h1>/;
			s/<<H([2-9])\s+(.+?)>>/<h$1>$2<\/h$1>/;
			s/<<T\s+(.+?)>>/<!DOCTYPE HTML PUBLIC \"-\/\/W3C\/\/DTD HTML 4.01 Transitional\/\/EN\" \"http:\/\/www\.w3\.org\/TR\/html4\/loose.dtd\">\n<html>\n<head>\n<title>$1<\/title>\n<LINK REL=StyleSheet HREF=\"style\.css\" TYPE=\"text\/css\" MEDIA=screen>\n<\/head>\n<body>/;
			s/<<F\s+(.+?)>>/<a STYLE="text-decoration: none" href=\"HTML\/$assoc{$1}\"><span class=\"sans\">$1<\/span><\/a>/;
			s/<<X\s+(.+?)>>/<a STYLE="text-decoration: none" href=\"HTML\/$assoc{$1}\"><span class=\"code\">$1<\/span><\/a>/;
			s/<<D\s+(.+?)>>/<div class="displaycode">$1<\/div>/;
			s/<<D\+\s+(.+?)>>/<div class="displaycode">$1<\/div><br>/;
			s/<<IN\s+(.+?)>>/<div class="indent">$1<\/div>/;
			s/<<IN\+\s+(.+?)>>/<div class="indent">$1<\/div><br>/;
		}
		print OUTPUT $_;
	}
	close INPUT;
	print OUTPUT "<a href=\"index.html\">Home<\/a>\n<\/body>\n</html>\n";
	close OUTPUT;
}
