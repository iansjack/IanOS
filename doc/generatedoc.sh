rm -rf HTML/HTML
cd ..
htags -g --map-file --item-order cf --insert-header home.html --insert-footer home.html --tree-view
mv HTML doc/HTML
cd doc
perl map.pl
perl xref.pl
perl convertall.pl
cp *.css HTML
cp *.jpg HTML
cp *.png HTML
