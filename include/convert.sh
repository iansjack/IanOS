gcc -S /home/ian/IanOS/include/convert.c -o - | gawk '($1 == "->") {print $2 " = " substr($3, 2, length($3) - 1)}' 
