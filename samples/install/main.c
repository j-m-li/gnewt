#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>

#include "newt.h"
#include "gnewt.h"

int main (void) 
{
	gnewtData res;

    	newtComponent b1, b2, b3, t1, t2, lb, l1, f, answer;

    	newtInit ();
    	newtCls ();

    	newtDrawRootText (20, 0, "Sample Install program (C) 2000 O'ksi'D");
	res = gnewtCmd("root_xpm:bg.xpm:0:0:-1:-1:");
    	newtPushHelpLine (NULL);
    	newtOpenWindow (2, 2, 75, 19, "Language Selection");
	if (IS_GNEWT(res)) 
		gnewtCmd("win_xpm:logo.xpm:67:6:-1:-1:");

    	f = newtForm (NULL, NULL, 0);
	if (IS_GNEWT(res)) 
		gnewtCmd("set_button_xpm:help.xpm:");
	b1 = newtButton ( 2, 15, "Help       ");
	if (IS_GNEWT(res)) 
		gnewtCmd("set_button_xpm:back.xpm:");
    	b2 = newtButton (40, 15, "Back       ");
	if (IS_GNEWT(res)) 
		gnewtCmd("set_button_xpm:forward.xpm:");
    	b3 = newtButton (59, 15, "Next       ");
    	t1 = newtTextbox (1, 2, 24, 12, NEWT_FLAG_WRAP | NEWT_FLAG_SCROLL);
	l1 = newtLabel (1, 1, "Online Help :");
    	newtTextboxSetText (t1, " Language Selection\n\nWhich language "
			"would you like to use during the installation "
			"process ?\nIf you don't select the right"
			" language then the install software will"
			" talk to you in a language that you don't "
			" understand.\nTo avoid this, choose your "
			"mothertongue language :)\n");

	lb = newtListbox(30, 3, 11,  NEWT_FLAG_BORDER | NEWT_FLAG_SCROLL);

	t2 = newtTextbox (29, 1, 46, 2, NEWT_FLAG_WRAP);
    	newtTextboxSetText (t2, "Which language should be used during the "
			"installation ?");

	newtListboxAppendEntry(lb, "English             (us)", (void *) 1);
	newtListboxAppendEntry(lb, "Français            (fr)", (void *) 2);
	newtListboxAppendEntry(lb, "Suisse Romand       (fr_CH)", (void *) 3);
	newtListboxAppendEntry(lb, "Deutch              (de)", (void *) 4);

	newtListboxSetCurrent(lb, 2);
	
    	newtFormAddComponents (f, lb, b3, b2, t1, b1, l1, t2, NULL);
    	newtRefresh ();

    	do {
		int ret = 0;
		answer = newtRunForm (f);
		if (answer == b1) {
			answer = NULL;
		} else if (answer == b2) {
			answer = NULL;
		} else if (answer == b3) {
		}
    		newtRefresh ();
    	} while (!answer);

    	newtFormDestroy (f);
    	newtPopWindow ();
    	newtFinished ();
    	return 0;
}

