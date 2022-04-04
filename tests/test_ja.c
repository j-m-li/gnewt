#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>

#include "newt.h"

struct callbackInfo {
	newtComponent en;
	char *state;
};

void disableCallback(newtComponent co, void *data)
{
	struct callbackInfo *cbi = data;

	if (*cbi->state == ' ') {
		newtEntrySetFlags(cbi->en, NEWT_FLAG_DISABLED,
				  NEWT_FLAGS_RESET);
	} else {
		newtEntrySetFlags(cbi->en, NEWT_FLAG_DISABLED,
				  NEWT_FLAGS_SET);
	}

	newtRefresh();
}

void suspend(void *d)
{
	newtSuspend();
	raise(SIGTSTP);
	newtResume();
}

int main(void)
{
	newtComponent b1, b2, r1, r2, r3, e2, e3, l1, l2, l3, scale;
	newtComponent lb, t, rsf, answer;
	newtComponent cs[10];
	newtComponent f, chklist, e1;
	struct callbackInfo cbis[3];
	char results[10];
	char *enr2, *enr3, *scaleVal;
	void **selectedList;
	int i, numsel;
	char buf[20];

	newtInit();
	newtCls();

	newtSetSuspendCallback(suspend, NULL);

	newtDrawRootText(0, 0, "新しいテストプログラム");
	newtPushHelpLine(NULL);
	newtDrawRootText(-50, 0, "もっとルートテキスト");

	newtOpenWindow(2, 2, 30, 10, "最初のウィンドウ");
	newtOpenWindow(10, 5, 65, 16, "ウィンドウ 2");

	f = newtForm(NULL, NULL, 0);
	chklist = newtForm(NULL, NULL, 0);

	b1 = newtButton(3, 1, "終り");
	b2 = newtButton(18, 1, "アップデート");
	r1 = newtRadiobutton(20, 10, "選択 1", 0, NULL);
	r2 = newtRadiobutton(20, 11, "選 2", 1, r1);
	r3 = newtRadiobutton(20, 12, "選択 3", 0, r2);
	rsf = newtForm(NULL, NULL, 0);
	newtFormAddComponents(rsf, r1, r2, r3, NULL);
	newtFormSetBackground(rsf, NEWT_COLORSET_CHECKBOX);

	for (i = 0; i < 8; i++) {
		sprintf(buf, "チェック %d", i);
		cs[i] =
		    newtCheckbox(3, 10 + i, buf, ' ', NULL, &results[i]);
		newtFormAddComponent(chklist, cs[i]);
	}
	sprintf(buf, "Check %d", i);
	cs[i] = newtCheckbox(3, 10 + i, buf, ' ', " AB", &results[i]);
	newtFormAddComponent(chklist, cs[i]);
	i++;
	sprintf(buf, "チェック %d", i);
	cs[i] = newtCheckbox(3, 10 + i, buf, ' ', " ABCDEFG", &results[i]);
	newtFormAddComponent(chklist, cs[i]);

	l1 = newtLabel(3, 6, "スケール:");
	l2 = newtLabel(3, 7, "スクロール:");
	l3 = newtLabel(3, 8, "隠蔽:");
	e1 = newtEntry(12, 6, "", 20, &scaleVal, 0);
	e2 = newtEntry(12, 7, "既知", 20, &enr2, NEWT_FLAG_SCROLL);
	e3 = newtEntry(12, 8, NULL, 20, &enr3, NEWT_FLAG_HIDDEN);

	cbis[0].state = &results[0];
	cbis[0].en = e1;
	newtComponentAddCallback(cs[0], disableCallback, &cbis[0]);

	scale = newtScale(3, 14, 32, 100);

	newtFormSetHeight(chklist, 3);

	newtFormAddComponents(f, b1, b2, l1, l2, l3, e1, e2, e3, chklist,
			      NULL);
	newtFormAddComponents(f, rsf, scale, NULL);

	lb = newtListbox(45, 1, 6, NEWT_FLAG_MULTIPLE | NEWT_FLAG_BORDER |
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

	newtListboxInsertEntry(lb, "５番目", (void *) 5, (void *) 4);
	newtListboxInsertEntry(lb, "１１番目", (void *) 11, (void *) 10);
	newtListboxDeleteEntry(lb, (void *) 11);

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
