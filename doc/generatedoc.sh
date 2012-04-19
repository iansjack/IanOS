#!/bin/sh
rm -rf HTML/HTML
cd ..
htags -g --map-file --item-order f --insert-header home.html --insert-footer home.html -T 
mv HTML doc/HTML
cd doc
perl map.pl
perl xref.pl
perl convertall.pl
cp *.css HTML
cp style.css HTML/HTML
cp *.jpg HTML
cp *.png HTML
