open(SITEMAP, "sitemap.txt") || die "Can't open sitemap.txt\n";
$number = 0;

while (<SITEMAP>) {
	/(\w+)(\s+)(.*)/;
	chomp($3);
	$siteassoc{$1} = $3;
	$number = $number + 1;
	$nameassoc{$1} = $number;
	$nameassoc{$number} = $1;
}
close(SITEMAP);

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

open(HEAD1, "head1.txt") || die "Can't open head1.txt\n";
while (<HEAD1>) {
	$head1 = $head1.$_;
	}
close(HEAD1);


open(HEAD2, "head2.txt") || die "Can't open head2.txt\n";
while (<HEAD2>) {
	$head2 = $head2.$_;
}
close(HEAD2);

$file = @ARGV[0];
$_ = $file;
s/(\w+)\.doc/$1/;

$title = $siteassoc{$_};
$number = $nameassoc{$_};
if ($title eq "") {
	$prev = "";
	$cont = "";
	$next = "";
} else {
	if ($nameassoc{$number - 1} eq "") {
		$prev = "<td><div class=\"prev\"><\/div><\/td>";
	} else {
		$prev = "<td><div class=\"prev\"><a href=\"$nameassoc{$number - 1}\.html\"><span class=\"bold\">Previous: <\/span>$siteassoc{$nameassoc{$number - 1}}<\/a><\/div></td>";
	}
	if ($nameassoc{$number + 1} eq "") {
		$next = "<td><div class=\"next\"><\/div><\/td>";
	} else {
		$next = "<td><div class=\"next\"><a href=\"$nameassoc{$number + 1}\.html\"><span class=\"bold\">Next: <\/span>$siteassoc{$nameassoc{$number + 1}}<\/a><\/div></td>";
	}
	$cont = "<td><div class=\"contents\"><a href=\"code\.html\">Contents<\/a><\/div><\/td>";	
}

open (INPUT, $file) || die "can't open $file";
$file =~ s/(\w+)\.doc/\1\.html/;
open (OUTPUT, ">HTML\/$file") || die "can't open temp";
while (<INPUT>) {
#print $_;
	while (/<</) {
		s/<<A\s+([\w:\/\.\@\-]+)\s+(.+?)>>/<a href=\"$1\">$2<\/a>/;
		s/<<PAR>>/<p>/;
		s/<<HR>>/<hr class=\"normal\"><p>/;
		s/<<CONSTR>>/<span class=\"underconst\">This page is still under construction.<\/span><p>/; # <br>/;
		s/<<C\s+(.+?)>>/<span class="code">$1<\/span>/;
		s/<<S\s+(.+?)>>/<span class="sans">$1<\/span>/;
		s/<<I\s+(.+?)>>/<span class="italic">$1<\/span>/;
		s/<<B\s+(.+?)>>/<span class="bold">$1<\/span>/;
		s/<<H1\s+(.+?)>>/<h1 class=\"normal\">$1<\/h1>/;
		s/<<H([2-9])\s+(.+?)>>/<h$1>$2<\/h$1><p>/;
		s/<<F\s+(.+?)>>/<a STYLE="text-decoration: none" href=\"HTML\/$assoc{$1}\"><span class=\"sans\">$1<\/span><\/a>/;
		s/<<T\s+(.+?)>>/$head1<title>$1<\/title>\n$head2\n<body>\n<div><a class=\"home\" href=\"\/\">Home<\/a><\/div>/;
		s/<<TI\s+(.+?)>>/$head1<title>$1<\/title>\n$head2\n<body>/;
		s/<<X\s+(.+?)>>/<a STYLE="text-decoration: none" href=\"HTML\/$assoc{$1}\"><span class=\"code\">$1<\/span><\/a>/;
		s/<<D\s+(.+?)>>/<div class="displaycode">$1<\/div>/;
		s/<<D\+\s+(.+?)>>/<div class="displaycode">$1<\/div><p>/;
		s/<<IN\s+(.+?)>>/<div class="indent">$1<\/div>/;
		s/<<IN\+\s+(.+?)>>/<div class="indent">$1<\/div><p>/;
		s/<<PREV\s+([\w:\/\.\@\-]+)\s+(.+?)>>/<td><div class="prev"><a href=\"$1\"><span class=\"bold\">Previous: <\/span>$2<\/a><\/div><\/td>/;
		s/<<NEXT\s+([\w:\/\.\@\-]+)\s+(.+?)>>/<td><div class="next"><a href=\"$1\"><span class=\"bold\">Next: <\/span>$2<\/a><\/div><\/td>/;
		s/<<NAV>>/<table border="0"><tr>$prev$cont$next<\/tr><\/table>/;

	}
	print OUTPUT $_;
}
print OUTPUT "<\/body>\n<\/html>";
close INPUT;
close OUTPUT;

