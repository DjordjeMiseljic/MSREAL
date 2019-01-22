#!/bin/sh
module="Storage"
device="Storage"

# invoke insmod with all arguments we got
# and use a pathname, as newer modutils
# don't look in . by default
/sbin/insmod ./$module.ko $* || exit 1

# remove stale node
rm -f /dev/${device}
# remove nodes [for multiple devices on same driver]
#rm -f /dev/${device}[0-3]

# find first argument in the line where
# second one is ur, this is major number
major=$(awk "\$2==\"$module\" {print \$1}" /proc/devices)
echo $major

#make node
mknod /dev/${device} c $major 0

#make nodes [for multiple devices on same driver]
#mknod /dev/${device}0 c $major 0
#mknod /dev/${device}1 c $major 1
#mknod /dev/${device}2 c $major 2
#mknod /dev/${device}3 c $major 3
