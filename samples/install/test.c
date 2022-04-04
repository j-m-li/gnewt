#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>

#include "newt.h"



int main(void)
{
	newtComponent b1, b2, r1, r2, r3, e2, e3, l1, l2, l3, scale;
	newtComponent lb, t, rsf, answer;
	newtComponent cs[10];
	newtComponent f, chklist, e1;
	char results[10];
	char *enr2, *enr3, *scaleVal;
	void **selectedList;
	int i, numsel;
	char buf[20];

	newtInit();
	newtCls();


	newtPushHelpLine(NULL);

	newtOpenWindow(10, 5, 65, 16, "ウィンドウ 2");

	f = newtForm(NULL, NULL, 0);

	lb = newtListbox(45, 1, 16,  NEWT_FLAG_BORDER |
			 NEWT_FLAG_SCROLL);
	newtListboxAppendEntry(lb, "最初", (void *) 1);
	newtListboxAppendEntry(lb, "２番目", (void *) 2);
	newtListboxAppendEntry(lb, "３番目", (void *) 3);
	newtListboxAppendEntry(lb, "４番目", (void *) 4);
	newtListboxAppendEntry(lb, "６番目", (void *) 6);
	newtListboxAppendEntry(lb, "７番目", (void *) 7);
	newtListboxAppendEntry(lb, "８番目", (void *) 8);
	newtListboxAppendEntry(lb, "９番目", (void *) 9);
	newtListboxAppendEntry(lb, "１０番目", (void *) 10);


	t = newtTextbox(45, 10, 17, 5, NEWT_FLAG_WRAP);
	newtTextboxSetText(t,
			   "テキストです。見える？\nこれは一人なはず\n\nこれは表示されるべきでない");

	newtFormAddComponents(f, lb, t, NULL);
	newtRefresh();

	do {
		answer = newtRunForm(f);

		if (answer == b2) {
			newtScaleSet(scale, atoi(scaleVal));
			newtRefresh();
			answer = NULL;
		}
	} while (!answer);

	scaleVal = strdup(scaleVal);
	enr2 = strdup(enr2);
	enr3 = strdup(enr3);

	selectedList = newtListboxGetSelection(lb, &numsel);

	newtFormDestroy(f);

	newtPopWindow();
	newtFinished();

	printf("got string 1: %s\n", scaleVal);
	printf("got string 2: %s\n", enr2);
	printf("got string 3: %s\n", enr3);

	if (selectedList) {
		printf("\nSelected listbox items:\n");
		for (i = 0; i < numsel; i++)
			printf("%i - ", (int) selectedList[i]);
	}

	return 0;
}
