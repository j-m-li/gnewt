#!/usr/bin/tclsh

if [file exists ../utils/tcl/gwhiptcl.so] {
	load ../utils/tcl/gwhiptcl.so
} elseif [file exists  /usr/lib/gwhiptcl.so] {
 	load /usr/lib/gwhiptcl.so
	puts "WARNING : default file loaded\n"
} else {
	load /usr/lib/whiptcl.so
	puts "WARNING : default file loaded\n"
}

whiptcl_init ()

set whiptcl_backtext "This is the background title"
set whiptcl_helpline "This is the help line"
set whiptcl_fullbuttons 0

whiptcl_cmd --msgbox "Hello,\nThis a test for the whiptcl.so tcl dynamicaly\
 loadable module." 20 70  --title "This is a test"

set whiptcl_fullbuttons 1
if {[whiptcl_cmd --yesno [exec cat ../COPYING] 20 70\
	 --title "here we go" --defaultno --scrolltext] == "yes"}  {
	set txt "you choose yes"
} else {
	set txt "you choose no"
}

set txt [whiptcl_cmd --inputbox "$txt \nPlease, add some comments :" 20 70]

set txt [whiptcl_cmd --menu "you wrote $txt" 20 70  \
	2 "1" "first item" "2" "second item" --title "menu test"]

set txt [whiptcl_cmd --radiolist "menu selection : $txt" 22 80\
	 3 "1st selected" "first item" 0\
	"2nd selected" "second item" 1 "3rd selected" "third item" 3]
if { $whiptcl_canceled == "1" } {
	set txt "$txt\nSelection was canceled.\n"
}

set txt [whiptcl_cmd --checklist "$txt" 22 80 3 "1" "first item" 1\
	"2" "second item" 0 "3" "third item" 1 --nocancel]

whiptcl_finish ()

puts "selected results are : $txt"

