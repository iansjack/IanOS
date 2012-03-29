rm -rf HTML/HTML
cd ..
htags -g --map-file --item-order cf --insert-header home.html --insert-footer home.html --tree-view
mv HTML doc/HTML
cd doc
perl map.pl
perl xref.pl
perl convert.pl
cp mystyle.css HTML

