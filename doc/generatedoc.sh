rm -rf HTML/HTML
cd ..
htags -g -F --map-file
mv HTML doc/HTML
cd doc
perl map.pl
perl xref.pl
perl convert.pl

