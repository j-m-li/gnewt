#ifdef __cplusplus
extern "C" {
#endif
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include <string.h>
#include <stdlib.h>

#include "dialogboxes.h"
#include "newt.h"
#include "popt.h"

enum mode { MODE_NONE, MODE_INFOBOX, MODE_MSGBOX, MODE_YESNO, MODE_CHECKLIST,
                MODE_INPUTBOX, MODE_RADIOLIST, MODE_MENU, MODE_GAUGE };

#define OPT_MSGBOX              1000
#define OPT_CHECKLIST           1001
#define OPT_YESNO               1002
#define OPT_INPUTBOX            1003
#define OPT_FULLBUTTONS         1004
#define OPT_MENU                1005
#define OPT_RADIOLIST           1006
#define OPT_GAUGE               1007
#define OPT_INFOBOX             1008

#ifdef __cplusplus
}
#endif


MODULE = whiperl		PACKAGE = whiperl		

PROTOTYPES: ENABLE

void 
Init ()
	CODE:
    	newtInit();
    	newtCls();
    	newtPushHelpLine("");

void
Finish ()
        CODE:
	newtFinished();

void 
FullButtons (full)
	int full
	CODE :
	useFullButtons(full);

void
Cmd (mode, flags, backtitle, title, text, width, height, ... )
	char* 	backtitle
	char* 	title
	char*	text
	int	flags
	int     width
	int	height
	int 	mode 
	PREINIT:
	char* 	argv[100];
	int 	argc = 0;
	int rc = -1; 
	char*	result = "";
	char ** selections;
	int fd = -1;
	struct poptOption optionsTable[] = {
		{ "mega", '\0', 0, 0, 10},
		{ 0, 0, 0, 0, 0 }
	};
	poptContext optCon;
	PPCODE:
	int i = 6;

	while (i < items ) {
		/*argv[argc] = (char *)SvPV(ST(i), PL_na);*/	
		argv[argc] = (char *)SvPV(ST(i), na);	
		i++;
		argc++;
	}
	argv[argc] = NULL;
	optCon = poptGetContext ("", argc, argv, optionsTable, 0);

	/* printf("%i\n", poptGetNextOpt(optCon));
	printf("%s\n", poptGetArg(optCon)); */

	poptGetNextOpt(optCon);
	width -= 2;
	height -= 2;
	newtOpenWindow((80 - width) / 2, (24 - height) / 2, 
		width, height, title);

	newtDrawRootText(0, 0, backtitle);

	switch (mode) {
	case MODE_MSGBOX:
		rc = messageBox(text, height, width, MSGBOX_MSG, flags);
		break;

	case MODE_INFOBOX:
		rc = messageBox(text, height, width, MSGBOX_INFO, flags);
		sleep (2);
		break;

	case MODE_YESNO:
		rc = messageBox(text, height, width, MSGBOX_YESNO, flags);
		break;

	case MODE_INPUTBOX:
		rc = inputBox(text, height, width, optCon, flags, &result);
		break;

	case MODE_MENU:
		rc = listBox(text, height, width, optCon, flags, &result);
		break;

	case MODE_RADIOLIST:
                rc = checkList (text, height, width, optCon, 1,
			 flags, &selections);
		if (!rc) { 
			result = selections[0];
                        free(selections);
                } 
                break;

        case MODE_CHECKLIST:   
                rc = checkList(text, height, width, optCon, 0, 
			flags, &selections);
		if (!rc) {
			char** next;
			PUSHMARK(sp);
			XPUSHs(sv_2mortal(newSViv(rc)));
			for (next = selections; *next; next++) {
				XPUSHs(sv_2mortal(newSVpv(*next,0)));
			}
		 	PUTBACK;
			return;	
		} 
                break;

        case MODE_GAUGE:
		rc = gauge(text, height, width, optCon, 0, flags);
		sleep (1);
		break;   

	default:
	}
	newtPopWindow();
	PUSHMARK(sp);
	XPUSHs(sv_2mortal(newSViv(rc)));	
	XPUSHs(sv_2mortal(newSVpv(result,0)));
	PUTBACK;


