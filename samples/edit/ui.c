#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>

#include "newt.h"

int menu_file ()
{
	return 10;
}

int menu_edit ()
{
	newtOpenWindow (15, 2, 20, 10, "Edit");
        newtRefresh ();
        sleep (1);
        newtPopWindow ();
	return 0;
}

int menu_find ()
{
	return 0;
}

int menu_view ()
{
	return 0;
}

int menu_options ()
{
	return 0;
}

int menu_help ()
{

}

int main (void) 
{
    	newtComponent b1, b2, b3, b4, b5, b6, l1, l2, t, f, answer;

    	newtInit ();
    	newtCls ();

    	newtDrawRootText (0, 0, "newt EDIT (C) 2000 O'ksi'D");
    	newtPushHelpLine (NULL);
    	newtOpenWindow (0, 0, 80, 24, "EDIT");

    	f = newtForm (NULL, NULL, 0);
    	b1 = newtCompactButton ( 0, 0, "File       ");
    	b2 = newtCompactButton (13, 0, "Edit       ");
    	b3 = newtCompactButton (26, 0, "Find       ");
    	b4 = newtCompactButton (39, 0, "View       ");
    	b5 = newtCompactButton (52, 0, "Options    ");
    	b6 = newtCompactButton (65, 0, "Help       ");
    	l1 = newtLabel (0, 23, "F1=Help");
    	l2 = newtLabel (78, 23, "|  Line:0  Column:0");
    	t = newtTextbox (0, 1, 78, 22, NEWT_FLAG_WRAP | NEWT_FLAG_SCROLL);

    	newtFormAddComponents (f, b1, b2, b3, b4, b5, b6, l1, l2, t, NULL);

    	newtTextboxSetText (t, "This is some \n\n\n\n\n\n\n\nfsdfsdf\n\n"
		"\n\n\n\n\n\n\n\n\n\n\nddasdddadsfsdfsdaffdsfadsdfaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n\nfsdfsadfdsf\n\n\n\n"
		"\n\n\text does it look okay?\nThis should be alone.\n"
		"This shouldn't be printed");

    	newtRefresh ();

    	do {
		int ret = 0;
		answer = newtRunForm (f);
		if (answer == b1) {
			ret = menu_file ();
			if (ret != 10) answer = NULL;
		} else if (answer == b2) {
			menu_edit ();
			answer = NULL;
		} else if (answer == b3) {
			menu_find ();
			answer = NULL;
		} else if (answer == b4) {
			menu_view ();
			answer = NULL;
		} else if (answer == b5) {
			menu_options ();
			answer = NULL;
		} else if (answer == b6) {
			menu_help ();
			answer = NULL;
		}
    	} while (!answer);

    	newtFormDestroy (f);
    	newtPopWindow ();
    	newtFinished ();
    	return 0;
}

