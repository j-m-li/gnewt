#!/bin/sh

if  test "x$DISPLAY" != "x" ; then
	export LD_PRELOAD=`pwd`"/../src/gtk/.libs/libgnewt.so"

	echo
fi

#if  test "x$DISPLAY" != "x" ; then
#	if  test -x "`which gwhiptail`"  ; then
#		whiptail () { gwhiptail "$@" ; }
#	fi
#fi

WHIP=whiptail

( sleep 1; echo 80; sleep 1; echo 90; sleep 1 ) | $WHIP --title "Hello World"\
         --backtitle "Whiptail is running" \
        --fb --gauge "this is a gauge" 20 70 30 2>> res.txt

$WHIP --title "Hello World" --backtitle "Whiptail is running" \
        --fb --checklist "checklist test" 22 80 3 "1" "first item" 1\
         "2" "second item" 0 "3" "third item" 1 2>> res.txt

$WHIP --title "Hello World" --backtitle "Whiptail is running" \
	--scrolltext --msgbox "`cat Makefile`" 22 76 2>> res.txt

$WHIP --title "Hello World" --backtitle "Whiptail is running" \
	--fb --menu "menu test" 22 80 2 "1" "first item" "2" "second item" \
	2>> res.txt

$WHIP --title "Hello World" --backtitle "Whiptail is running" \
        --fb --inputbox "input box text :" 22 80 2>> res.txt

$WHIP --title "Hello World" --backtitle "Whiptail is running" \
        --fb --yesno "yes or no ?" 10 50 
if [ $? -eq 0 ]; then
	echo YES >> res.txt
fi

$WHIP --title "Hello World" --backtitle "Whiptail is running" \
        --fb --radiolist "radiolist test" 22 80 3 "1" "first item" 0\
	 "2" "second item" 1 "3" "third item" 3 2>> res.txt


$WHIP --title 'Select Category' --backtitle ' ' --menu \
'Modules are loadable device drivers. Please go through the menus 
for each category and look for devices, network protocols, filesystems, 
etc. that you would like to have supported by your system. You should 
not install modules for devices that aren t installed in your system, 
as they will sometimes cause the system to pause for a long time while 
it is searching for the device. Also, drivers for devices that you 
don t have use memory that you could put to better use. 

Please select the category of modules.' \
 22 79 11 \
'Exit' 'Finished with these modules. Return to previous menu.' \
'  '  '   ' \
'block' 'Disks and disk-like devices.'  \
'cdrom' 'Device drivers for CD-ROM drives.'   \
'fs' 'Drivers that allow many different filesystems to be accessed.'  \
'ipv4' 'Internet Protocol drivers.'  \
'ipv6' 'Internet Protocol version 6 drivers.'   \
'misc' 'Drivers that don t fit in the other categories.'   \
'net' 'Drivers for network interface cards and network protocols.'   \
'scsi' 'Drivers for SCSI controller cards and classes of SCSI devices.'  \
'video' 'Drivers for Video4Linux.'  2>> res.txt

cat res.txt
rm -f res.txt
