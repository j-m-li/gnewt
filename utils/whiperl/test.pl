# Before `make install' is performed this script should be runnable with
# `make test'. After `make install' it should work as `perl test.pl'

######################### We start with some black magic to print on failure.

# Change 1..1 below to 1..last_test_to_print .
# (It may become useful if the test is moved to ./t subdirectory.)

BEGIN { $| = 1; print "1..1333\n"; }
END {print "not ok 1\n" unless $loaded;}
use whiperl;
$loaded = 1;
print "ok 1\n";

######################### End of black magic.

# Insert your test code below (better if it prints "ok 13"
# (correspondingly "not ok 13") depending on the success of chunk 13
# of the test code):

whiperl::Init();

whiperl::FullButtons(0);

@xx = whiperl::Cmd($whiperl::INPUTBOX, 
	$whiperl::NOCANCEL | $whiperl::SCROLL_TEXT , 
	"This is the back title", 
	"this is the window title",
        "this is some text.\n\ninput box\n", 70, 18, "initial text");
print "input box : $xx[0] : $xx[1]\n";

whiperl::FullButtons(1);

@xx = whiperl::Cmd($whiperl::RADIOLIST, 
	$whiperl::DEFAULT_NO , 
	"This is the back title", 
	"this is the window title",
        "radio list", 70, 18,
	3,
        "1", "first item", 0,
        "2", "second item", 0,
        "3", "third item", 1);
print "radio list : $xx[0] $xx[1]\n";

@xx = whiperl::Cmd($whiperl::MENU, 0, "This is the back title", 
	"this is the window title",
        "menu", 70, 18, 
        3,
        "1", "first item",
        "2", "second item",
        "3", "third item");
print "menu : $xx[0] $xx[1]\n";

@xx = whiperl::Cmd($whiperl::CHECKLIST, 0, "This is the back title", 
	"this is the window title",
        "check list", 70, 18, 
	3,
	"1", "first item", 0,
	"2", "second item", 0,
	"3", "third item", 1);
$i = 0;
foreach (@xx) {
	 if ($i) { print "check selected : $_\n"; }
	$i++;
}

@xx = whiperl::Cmd($whiperl::YESNO, 0, "This is the back title", 
	"this is the window title",
	"this is some text.\n\nHello world\n", 70, 18);
print "yesno : $xx[0]\n";
if ($xx[0]) {
	print "you've selected NO\n";
}

whiperl::Cmd($whiperl::INFOBOX, 0, "This is the back title", 
	"this is the window title",
        "THIS IS AN INFO BOX", 70, 18);

@xx = whiperl::Cmd($whiperl::MSGBOX, 0, "This is the back title",
        "this is the window title",
        "this is some text.\n\nHello world\n", 70, 18);
print "message : $xx[0]\n";

#whiperl::Cmd($whiperl::GAUGE, 0, "This is the back title", 
#	"this is the window title",
#        "this is a gauge", 70, 18, 45);

whiperl::Finish();

print "ok 13\n";

