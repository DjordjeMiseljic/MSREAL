#! /bin/bash
# instantiating led driver into kernel
if [ ! -e /proc/myled ]
then
    insmod /root/drivers/led_driver/myled.ko
fi
#instantiating pushbutton drivers
if [ ! -d /sys/class/gpio/gpio956 ]
then
   echo 956 > /sys/class/gpio/export
   echo in > /sys/class/gpio/gpio956/direction
fi
if [ ! -d /sys/class/gpio/gpio957 ]
then
    echo 957 > /sys/class/gpio/export
    echo in > /sys/class/gpio/gpio957/direction
fi
