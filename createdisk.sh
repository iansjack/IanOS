#/bin/sh
dd if=/dev/zero of=c.hdd bs=516096c count=200
/sbin/losetup /dev/loop0 c.hdd
/sbin/sfdisk -u -C200 -S63 -H16 /dev/loop0 <<EOF
63,,6
EOF
/sbin/losetup -d /dev/loop0
/sbin/losetup -o32256 /dev/loop1 c.hdd
/sbin/mkdosfs -F16 /dev/loop1 100768
/sbin/losetup -d /dev/loop1
chmod a+rw c.hdd
mformat C: -t200 -h16 -s63
mmd C:/BIN

