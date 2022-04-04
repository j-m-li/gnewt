/*   Gtk+ version of the newt library
 *
 *   Copyright (C) 1996-1999  Red Hat Software
 *   Copyright (C) 1999-2000  O'ksi'D
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Library General Public
 *   License as published by the Free Software Foundation; either
 *   version 2 of the License, or (at your option) any later version.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   Library General Public License for more details.
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this library; if not, write to the Free
 *   Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *       O'ksi'D <nickasil@linuxave.net>
 *       Jean-Marc Lienher
 *       Rue de la Cheminee 1
 *       CH-2065 Savagnier
 *       Switzerland
 */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/signal.h>
#include <sys/time.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <errno.h>
#include <fcntl.h>

#include "newt.h"
#include "newt_pr.h"
#include "gnewt.h"
#include "gnewt_pr.h"

struct keymap {
	char *str;
	int code;
	char *tc;
};

static struct Window windowStack[20];

/* gNewt special variable */
static gchar *ctbl[44];
static struct Gnewt gnewtObj = {
	000601,		/*version*/
	NULL,		/*currentWindow*/
	0,		/*pressedKey*/
	NULL,		/*gRootStyle*/
	NULL,		/*currentParent*/
	640 / 80,	/*FontSizeW*/
	480 / 24,	/*FontSizeH*/
	0,		/*formLevel*/
	NULL,		/*currentExitComp*/
	NULL,		/*gRootWindow*/
	1,		/*useNewtColor*/
	1,		/*Color4bits*/
	ctbl,		/*colorTable*/
	NULL,		/*globalColors*/
	NULL,		/*gObject*/
	NULL,		/*buttonPixmap*/
	NULL,		/*buttonMask*/
	1,		/*showBackground*/
};
struct Gnewt *gnewt = &gnewtObj;
static int ScreenCols = 80;
static int ScreenRows = 24;
static int ScreenW = 640;
static int ScreenH = 480;

static char *helplineStack[20];
static char **currentHelpline = NULL;
static GtkWidget *currentHelpWidget = NULL;
static GtkWidget *motherWindow = NULL;
static int mainPid = 0;
static struct sigaction signalAction;

static int cursorRow, cursorCol;
static int needResize;
static int threeDbox = 0;

static const char *defaultHelpLine =
    "  <Tab>/<Alt-Tab> between elements   |  <Space> selects   |  <F12> next screen";

struct newtColors newtDefaultColorPalette = {
	"white", "blue",	/* root fg, bg */
	"black", "lightgray",	/* border fg, bg */
	"black", "lightgray",	/* window fg, bg */
	"white", "black",	/* shadow fg, bg */
	"red", "lightgray",	/* title fg, bg */
	"blue", "red",		/* button fg, bg */
	"red", "lightgray",	/* active button fg, bg */
	"yellow", "blue",	/* checkbox fg, bg */
	"blue", "brown",	/* active checkbox fg, bg */
	"yellow", "blue",	/* entry box fg, bg */
	"blue", "lightgray",	/* label fg, bg */
	"black", "lightgray",	/* listbox fg, bg */
	"yellow", "blue",	/* active listbox fg, bg */
	"black", "lightgray",	/* textbox fg, bg */
	"lightgray", "black",	/* active textbox fg, bg */
	"white", "blue",	/* help line */
	"yellow", "blue",	/* root text */
	"blue",			/* scale empty */
	"red",			/* scale full */
	"blue", "lightgray",	/* disabled entry fg, bg */
	"white", "blue",	/* compact button fg, bg */
	"yellow", "red",	/* active & sel listbox */
	"black", "brown",	/* selected listbox */
};

struct {
	GdkColor color;
	gchar *name;
} gtk_colors[] = {
	{ {
	0, 0x0000, 0x0000, 0x0000}, "black"}, { {
	0, 0xFFFF, 0xFFFF, 0xFFFF}, "white"}, { {
	0, 0xFFFF, 0x0000, 0x0000}, "red"}, { {
	0, 0x0000, 0xFFFF, 0x0000}, "green"}, { {
	0, 0x0000, 0x0000, 0xFFFF}, "blue"}, { {
	0, 0x0000, 0xFFFF, 0xFFFF}, "cyan"}, { {
	0, 0xFFFF, 0x0000, 0xFFFF}, "magenta"}, { {
	0, 0xFFFF, 0xFFFF, 0x0000}, "yellow"},
/* { { 0, 0x8000, 0x8000, 0x8000 }, "gray" }, */
	{ {
	0, 0xC000, 0xC000, 0xC000}, "lightgray"}, { {
	0, 0x8000, 0x8000, 0x0000}, "brown"}, { {
	0, 0x8000, 0x0000, 0x8000}, "darkmagenta"}, { {
	0, 0x0000, 0x8000, 0x8000}, "darkcyan"}, { {
	0, 0x0000, 0x8000, 0x0000}, "darkgreen"}, { {
	0, 0x0000, 0x0000, 0x8000}, "darkblue"}, { {
	0, 0xFFFF, 0x8000, 0x0000}, "orange"}, { {
	0, 0xFFFF, 0xFFFF, 0x8000}, "mediumgreen"}, { {
	0, 0, 0, 0}, NULL},
};

static const struct keymap keymap[] = {
	{"\033OA", NEWT_KEY_UP, "kh"},
	{"\033[A", NEWT_KEY_UP, "ku"},
	{"\033OB", NEWT_KEY_DOWN, "kd"},
	{"\033[B", NEWT_KEY_DOWN, "kd"},
	{"\033[C", NEWT_KEY_RIGHT, "kr"},
	{"\033OC", NEWT_KEY_RIGHT, "kr"},
	{"\033[D", NEWT_KEY_LEFT, "kl"},
	{"\033OD", NEWT_KEY_LEFT, "kl"},
	{"\033[H", NEWT_KEY_HOME, "kh"},
	{"\033[1~", NEWT_KEY_HOME, "kh"},
	{"\033Ow", NEWT_KEY_END, "kH"},
	{"\033[4~", NEWT_KEY_END, "kH"},

	{"\033[3~", NEWT_KEY_DELETE, "kl"},

	{"\033\t", NEWT_KEY_UNTAB, NULL},

	{"\033[5~", NEWT_KEY_PGUP, NULL},
	{"\033[6~", NEWT_KEY_PGDN, NULL},
	{"\033V", NEWT_KEY_PGUP, "kH"},
	{"\033v", NEWT_KEY_PGUP, "kH"},

	{"\033[[A", NEWT_KEY_F1, NULL},
	{"\033[[B", NEWT_KEY_F2, NULL},
	{"\033[[C", NEWT_KEY_F3, NULL},
	{"\033[[D", NEWT_KEY_F4, NULL},
	{"\033[[E", NEWT_KEY_F5, NULL},

	{"\033[11~", NEWT_KEY_F1, NULL},
	{"\033[12~", NEWT_KEY_F2, NULL},
	{"\033[13~", NEWT_KEY_F3, NULL},
	{"\033[14~", NEWT_KEY_F4, NULL},
	{"\033[15~", NEWT_KEY_F5, NULL},
	{"\033[17~", NEWT_KEY_F6, NULL},
	{"\033[18~", NEWT_KEY_F7, NULL},
	{"\033[19~", NEWT_KEY_F8, NULL},
	{"\033[20~", NEWT_KEY_F9, NULL},
	{"\033[21~", NEWT_KEY_F10, NULL},
	{"\033[23~", NEWT_KEY_F11, NULL},
	{"\033[24~", NEWT_KEY_F12, NULL},

	{NULL, 0, NULL},	/* LEAVE this one */
};

static char keyPrefix = '\033';

static const char *version =
    "gNewt : GTK wrapper for Newt version " VERSION
    " - (C) 1996 Red Hat Software - (C) 1999 O'ksi'D. "
    "Redistributable under the term of the Library " "GNU Public License. "
    "gNewt : Adapted by O'ksi'D. " "Newt : Written by Erik Troan\n";

static newtSuspendCallback suspendCallback = NULL;
static void *suspendCallbackData = NULL;

void newtSetSuspendCallback(newtSuspendCallback cb, void *data)
{
	suspendCallback = cb;
	suspendCallbackData = data;
}


static void handleSigwinch(int signum)
{
	needResize = 1;
}

static int getkeyInterruptHook(void)
{
	return -1;
}

GdkColor getColorByName(gchar * name)
{
	int i = 0;
	while (gtk_colors[i].name && strcmp(name, gtk_colors[i].name))
		i++;
	return gtk_colors[i].color;
}

/*
 * threeDbox = 0 - no 3D boxes
 *           = 1 - 3D raised textboxes, sunken listboxes
 */
void newtSetThreeD(int val)
{
	threeDbox = val;
}

void newtFlushInput(void)
{
}

void newtRefresh(void)
{
	while (gtk_events_pending())
		gtk_main_iteration();
}

void killChild(void)
{
	if (mainPid != getpid()) {
		raise(SIGKILL);
	}
}

static void ignoreSig(int signo)
{
	return;
}

void newtSuspend(void)
{
	struct sigaction sa_new;

	gtk_widget_hide_all(motherWindow);
	newtRefresh();

	sigemptyset(&sa_new.sa_mask);
	sigaddset(&sa_new.sa_mask, SIGQUIT);
	sigaddset(&sa_new.sa_mask, SIGTSTP);
	sigaddset(&sa_new.sa_mask, SIGINT);
	sigaddset(&sa_new.sa_mask, SIGTTIN);
	sigaddset(&sa_new.sa_mask, SIGCHLD);
	sigaddset(&sa_new.sa_mask, SIGSTOP);
	sa_new.sa_handler = ignoreSig;
	sa_new.sa_flags = 0;
	sigaction(SIGQUIT, &sa_new, &signalAction);
	sigaction(SIGTSTP, &sa_new, &signalAction);
	sigaction(SIGINT, &sa_new, &signalAction);
	sigaction(SIGTTIN, &sa_new, &signalAction);
	sigaction(SIGCHLD, &sa_new, &signalAction);
	sigaction(SIGSTOP, &sa_new, &signalAction);

}

void newtResume(void)
{
	sigaction(SIGQUIT, &signalAction, 0);
	sigaction(SIGTSTP, &signalAction, 0);
	sigaction(SIGINT, &signalAction, 0);
	sigaction(SIGTTIN, &signalAction, 0);
	sigaction(SIGCHLD, &signalAction, 0);
	sigaction(SIGSTOP, &signalAction, 0);

	if (gnewt->showBackground) gtk_widget_show_all(motherWindow);
}

void newtCls(void)
{
}

void newtResizeScreen(int redraw)
{
	if (redraw)
		newtRefresh();
}

static gint keyPressedCallback(gpointer unused, GdkEventKey * event)
{
	/* printf("%s : %c\n", event->string, event->keyval); */
	switch (event->keyval) {
	case 65307:
		gnewt->pressedKey = NEWT_KEY_SUSPEND;
		break;
	case 65481:
		gnewt->pressedKey = NEWT_KEY_F12;
		break;
	case 65480:
		gnewt->pressedKey = NEWT_KEY_F11;
		break;
	case 65479:
		gnewt->pressedKey = NEWT_KEY_F10;
		break;
	case 65478:
		gnewt->pressedKey = NEWT_KEY_F9;
		break;
	case 65477:
		gnewt->pressedKey = NEWT_KEY_F8;
		break;
	case 65476:
		gnewt->pressedKey = NEWT_KEY_F7;
		break;
	case 65475:
		gnewt->pressedKey = NEWT_KEY_F6;
		break;
	case 65474:
		gnewt->pressedKey = NEWT_KEY_F5;
		break;
	case 65473:
		gnewt->pressedKey = NEWT_KEY_F4;
		break;
	case 65472:
		gnewt->pressedKey = NEWT_KEY_F3;
		break;
	case 65471:
		gnewt->pressedKey = NEWT_KEY_F2;
		break;
	case 65470:
		gnewt->pressedKey = NEWT_KEY_F1;
		break;
	default:
		return TRUE;
	}
	gtk_main_quit();
	return TRUE;
}

static void mainQuit(void)
{
	gnewt->pressedKey = NEWT_KEY_F12;
	if (motherWindow) {
		newtRefresh();
		motherWindow = NULL;
		gtk_main_quit();
		/* write(2, "Exiting NOW !!!\n", 17);*/
		exit(-1);
	}
	if (gnewt->currentWindow) {
		gnewt->currentWindow->widget = NULL;
		gnewt->currentWindow = NULL;
	}
}

int newtInit(void)
{
	GtkWidget *win;
	GtkStyle *style;
	char *var = NULL;

	/* use the version variable just to be sure it gets included */
	strlen(version);

	gnewt->currentWindow = NULL;

	(void) keyPrefix;	/* to avoid compiler warning */

	gnewt->globalColors = malloc(sizeof(struct newtColors));


	newtSetColors(newtDefaultColorPalette);
	handleSigwinch(0);
	getkeyInterruptHook();

	/* XIM protocol compliant */
	gtk_set_locale();
	gtk_init(NULL, NULL);

	/* set a good default color contrast for 16 colors display */
	style = gtk_widget_get_default_style();
	if (gnewt->Color4bits) {
		style->bg[0] = getColorByName("lightgray");
	}

	win = motherWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	style = gtk_rc_get_style(win);
	if (!style)
		style = gtk_widget_get_style(win);

	gnewt->gRootStyle = gtk_style_copy(style);


	gnewt->FontSizeH = style->font->ascent + style->font->descent + 2;
	gnewt->FontSizeW = gdk_char_measure(style->font, '_');

	var = getenv("GNEWT_FONT_SIZE");
	if (var) {
		char **size;
		size = g_strsplit(var, "x", -1);
		if (size[0])
			gnewt->FontSizeH = atoi(size[0]);
		if (size[1])
			gnewt->FontSizeW = atoi(size[1]);
	}

	if (gnewt->FontSizeH < 14)
		gnewt->FontSizeH = 14;
	if (gnewt->FontSizeW < 7)
		gnewt->FontSizeW = 7;
	ScreenW = gnewt->FontSizeW * ScreenCols;
	ScreenH = gnewt->FontSizeH * ScreenRows;


	gtk_signal_connect_object(GTK_OBJECT(win), "key_press_event",
				  GTK_SIGNAL_FUNC(keyPressedCallback),
				  NULL);
	gtk_widget_set_events(win, GDK_KEY_PRESS_MASK);
	gtk_widget_set_usize(win, ScreenW, ScreenH);

	GTK_WINDOW(win)->allow_shrink = TRUE;
	gtk_window_set_title(GTK_WINDOW(win), "gNewt window");
	GTK_WINDOW(win)->allow_grow = FALSE;

	gtk_widget_set_uposition(win, (gdk_screen_width() - ScreenW) / 2,
				 (gdk_screen_height() - ScreenH) / 2);

	gtk_container_set_border_width(GTK_CONTAINER(win), 0);
	if (gnewt->showBackground) gtk_widget_show(win);

	gtk_widget_push_style(style);
	style = gtk_style_copy(gnewt->gRootStyle);
	if (gnewt->useNewtColor) {
		style->bg[0] = getColorByName(gnewt->globalColors->rootBg);
	}
	gnewt->gRootWindow = gtk_fixed_new();
	gtk_widget_set_style(gnewt->gRootWindow, style);
	gtk_widget_pop_style();

	gtk_container_set_border_width(GTK_CONTAINER(gnewt->gRootWindow), 0);

	gtk_container_add(GTK_CONTAINER(win), gnewt->gRootWindow);
	gtk_widget_show(gnewt->gRootWindow);
	gnewt->currentParent = gnewt->gRootWindow;

	gtk_signal_connect_object(GTK_OBJECT(gnewt->gRootWindow), "destroy",
				  GTK_SIGNAL_FUNC(mainQuit), NULL);

	/* Kill the child process when it calls exit() because exit() closes
	   all file descriptors and GDK doesn't like that :-) */
	mainPid = getpid();
	atexit(killChild);

	gnewt->pressedKey = 0;
	return 0;
}

int newtFinished(void)
{
	free(gnewt->globalColors);
	return 0;
}

void newtSetColors(struct newtColors colors)
{
	gnewt->colorTable[0] = gnewt->globalColors->rootFg = colors.rootBg;
	gnewt->colorTable[1] = gnewt->globalColors->rootBg = colors.rootBg;
	gnewt->colorTable[2] = gnewt->globalColors->borderFg = colors.borderFg;
	gnewt->colorTable[3] = gnewt->globalColors->borderBg = colors.borderBg;
	gnewt->colorTable[4] = gnewt->globalColors->windowFg = colors.windowFg;
	gnewt->colorTable[5] = gnewt->globalColors->windowBg = colors.windowBg;
	gnewt->colorTable[6] = gnewt->globalColors->shadowFg = colors.shadowFg;
	gnewt->colorTable[7] = gnewt->globalColors->shadowBg = colors.shadowBg;
	gnewt->colorTable[8] = gnewt->globalColors->titleFg = colors.titleFg;
	gnewt->colorTable[9] = gnewt->globalColors->titleBg = colors.titleBg;
	gnewt->colorTable[10] = gnewt->globalColors->buttonFg = colors.buttonFg;
	gnewt->colorTable[11] = gnewt->globalColors->buttonBg = colors.buttonBg;
	gnewt->colorTable[12] = gnewt->globalColors->actButtonFg = 
		colors.actButtonFg;
	gnewt->colorTable[13] = gnewt->globalColors->actButtonBg = 
		colors.actButtonBg;
	gnewt->colorTable[14] = gnewt->globalColors->checkboxFg = 
		colors.checkboxFg;
	gnewt->colorTable[15] = gnewt->globalColors->checkboxBg = 
		colors.checkboxBg;
	gnewt->colorTable[16] = gnewt->globalColors->actCheckboxFg = 
		colors.actCheckboxFg;
	gnewt->colorTable[17] = gnewt->globalColors->actCheckboxBg = 
		colors.actCheckboxBg;
	gnewt->colorTable[18] = gnewt->globalColors->entryFg = colors.entryFg;
	gnewt->colorTable[19] = gnewt->globalColors->entryBg = colors.entryBg;
	gnewt->colorTable[20] = gnewt->globalColors->labelFg = colors.labelFg;
	gnewt->colorTable[21] = gnewt->globalColors->labelBg = colors.labelBg;
	gnewt->colorTable[22] = gnewt->globalColors->listboxFg = 
		colors.listboxFg;
	gnewt->colorTable[23] = gnewt->globalColors->listboxBg = 
		colors.listboxBg;
	gnewt->colorTable[24] = gnewt->globalColors->actListboxFg = 
		colors.actListboxFg;
	gnewt->colorTable[25] = gnewt->globalColors->actListboxBg = 
		colors.actListboxBg;
	gnewt->colorTable[26] = gnewt->globalColors->textboxFg = 
		colors.textboxFg;
	gnewt->colorTable[27] = gnewt->globalColors->textboxBg = 
		colors.textboxBg;
	gnewt->colorTable[28] = gnewt->globalColors->actTextboxFg = 
		colors.actTextboxFg;
	gnewt->colorTable[29] = gnewt->globalColors->actTextboxBg = 
		colors.actTextboxBg;
	gnewt->colorTable[30] = gnewt->globalColors->helpLineFg = 
		colors.helpLineFg;
	gnewt->colorTable[31] = gnewt->globalColors->helpLineBg = 
		colors.helpLineBg;
	gnewt->colorTable[32] = gnewt->globalColors->rootTextFg = 
		colors.rootTextFg;
	gnewt->colorTable[33] = gnewt->globalColors->rootTextBg = 
		colors.rootTextBg;
	gnewt->colorTable[34] = gnewt->globalColors->emptyScale = 
		colors.emptyScale;
	gnewt->colorTable[35] = gnewt->globalColors->fullScale = 
		colors.fullScale;
	gnewt->colorTable[36] = gnewt->globalColors->disabledEntryFg = 
		colors.disabledEntryFg;
	gnewt->colorTable[37] = gnewt->globalColors->disabledEntryBg = 
		colors.disabledEntryBg;
	gnewt->colorTable[38] = gnewt->globalColors->compactButtonFg = 
		colors.compactButtonFg;
	gnewt->colorTable[39] = gnewt->globalColors->compactButtonBg = 
		colors.compactButtonBg;
	gnewt->colorTable[40] = gnewt->globalColors->actSelListboxFg = 
		colors.actSelListboxFg;
	gnewt->colorTable[41] = gnewt->globalColors->actSelListboxBg = 
		colors.actSelListboxBg;
	gnewt->colorTable[42] = gnewt->globalColors->selListboxFg = 
		colors.selListboxFg;
	gnewt->colorTable[43] = gnewt->globalColors->selListboxBg = 
		colors.selListboxBg;
}

int newtGetKey(void)
{
	return 0;
}

void newtWaitForKey(void)
{
	/*newtRefresh();*/
	newtClearKeyBuffer();
}

void newtClearKeyBuffer(void)
{
}

int newtOpenWindow(int left, int top, int width, int height,
		   const char *title)
{
	GtkWidget *window, *win, *bg, *bg1;
	GtkStyle *style;
	GdkPixmap *pixmap, *pixmap1;
	GdkBitmap *mask;
	int titleW, titleO;

	if (!motherWindow)
		exit(-1);
	if (!gnewt->currentWindow) {
		gnewt->currentWindow = windowStack;
	} else {
		gnewt->currentWindow++;
	}

	if (width < 0 && height < 0) {
		top = 1;
		left = 1;
		width = 77 + width;
		height = 24 + height;
	}

	gnewt->currentWindow->left = left;
	gnewt->currentWindow->top = top;
	gnewt->currentWindow->width = width;
	gnewt->currentWindow->height = height;
	gnewt->currentWindow->title = title ? strdup(title) : NULL;
	if (!gnewt->showBackground) {
		GtkWidget *tlevel, *fix;	
		tlevel = gtk_window_new(GTK_WINDOW_TOPLEVEL);
		gtk_widget_set_uposition(tlevel, 
				(gdk_screen_width() - ScreenW) / 2 +
					gnewt->FontSizeW * left,
				(gdk_screen_height() - ScreenH) / 2 +
					gnewt->FontSizeH * top);
		fix = gtk_fixed_new();
		gtk_widget_set_usize(fix, 
				gnewt->FontSizeW * (width + 1),
			     	gnewt->FontSizeH * (height));
		if (top>0) top = 0;
		if (left>0) left = 0;
		gtk_container_add(GTK_CONTAINER(tlevel), fix);
		gnewt->gRootWindow = fix;
		gtk_window_set_title (GTK_WINDOW(tlevel), title);
		GTK_WINDOW(tlevel)->allow_shrink = TRUE;
		GTK_WINDOW(tlevel)->allow_grow = FALSE;
		gtk_widget_show(tlevel);
		gtk_widget_show(fix);
		gnewt->currentWindow->top_widget = tlevel;
	} 
	window = gtk_fixed_new();
	
	gnewt->currentWindow->buffer = (short *) NULL;
	gnewt->currentWindow->widget = window;
	gnewt->currentParent = window;

	/* creat a background for the frame */
	win = gtk_fixed_new();
	style = gtk_style_copy(gnewt->gRootStyle);
	if (gnewt->useNewtColor) {
		style->bg[0] = getColorByName(gnewt->globalColors->borderBg);
		style->fg[0] = getColorByName(gnewt->globalColors->borderFg);
	}
	gtk_widget_set_style(win, style);
	gnewt->currentWindow->bg_widget = win;
	gtk_fixed_put(GTK_FIXED(gnewt->gRootWindow), win,
		      gnewt->FontSizeW * (left - 1), 
		      gnewt->FontSizeH * (top - 1));
	gtk_widget_set_usize(win, gnewt->FontSizeW * (width + 2),
			     gnewt->FontSizeH * (height + 2));

	/* creat the frame */
	style = gtk_style_copy(gnewt->gRootStyle);
	if (gnewt->useNewtColor) {
		style->bg[0] = getColorByName(gnewt->globalColors->windowBg);
		style->fg[0] = getColorByName(gnewt->globalColors->titleFg);
	}
	gtk_widget_set_style(window, style);
	gnewt->currentWindow->bg_widget = win;

	gtk_fixed_put(GTK_FIXED(gnewt->gRootWindow), window, 
		      gnewt->FontSizeW * left, 
		      gnewt->FontSizeH * top);
	gtk_widget_set_usize(window, gnewt->FontSizeW * (width + 2),
			     gnewt->FontSizeH * (height + 2) - 3);

	if (!gnewt->currentWindow->title) {
		gnewt->currentWindow->title = strdup (" ");
	}	
	titleW = strlen(gnewt->currentWindow->title) + 2;
	titleO = gnewt->FontSizeW * (width - titleW + 2) / 2;
	
	gtk_widget_show(win);
	gtk_widget_show(window);
	newtRefresh();
	pixmap = gdk_pixmap_new(win->window, 
			gnewt->FontSizeH * (width + 3),
			gnewt->FontSizeH * (height + 3) - 3, -1);
	pixmap1 = gdk_pixmap_new(window->window, 
			gnewt->FontSizeH * (width + 3),
			gnewt->FontSizeH * (height + 3) - 3, -1);
	bg = gtk_pixmap_new(pixmap, NULL);
	bg1 = gtk_pixmap_new(pixmap1, NULL);

	gnewt->currentWindow->pixmap = pixmap1;

	gtk_fixed_put(GTK_FIXED(win), bg, 0, 0);
	gtk_fixed_put(GTK_FIXED(window), bg1, 0, 0);
	
	gdk_draw_rectangle(pixmap, window->style->bg_gc[0],
			   TRUE, 0, 0,
			   gnewt->FontSizeW * (width + 2),
			   gnewt->FontSizeH * (height + 2));
	gdk_draw_rectangle(pixmap1, 
			   window->style->bg_gc[0],
			   /*window->style->white_gc,*/
			   TRUE, 0, 0,
			   gnewt->FontSizeW * (width + 2),
			   gnewt->FontSizeH * (height + 2) - 3);
	if (gnewt->useNewtColor) {
		gdk_draw_rectangle(pixmap1, win->style->fg_gc[0],
			   TRUE, gnewt->FontSizeW * (width + 1),
			   0,
			   gnewt->FontSizeW, 
			   gnewt->FontSizeH * (height + 2) - 3);
		gdk_draw_rectangle(pixmap1, win->style->fg_gc[0],
			   TRUE, 0,
			   gnewt->FontSizeH * (height + 1),
			   gnewt->FontSizeW * (width + 1), 
			   gnewt->FontSizeH - 3);

		gdk_draw_line(pixmap,
		      win->style->fg_gc[0],
		      3, 8, 3, gnewt->FontSizeH * (height + 2) - 7);
		gdk_draw_line(pixmap,
		      win->style->fg_gc[0],
		      3, gnewt->FontSizeH * (height + 2) - 7,
		      gnewt->FontSizeW * (width + 2) - 3,
		      gnewt->FontSizeH * (height + 2) - 7);
		gdk_draw_line(pixmap, win->style->fg_gc[0],
		      gnewt->FontSizeW * (width + 2) - 3, 8,
		      gnewt->FontSizeW * (width + 2) - 3,
		      gnewt->FontSizeH * (height + 2) - 7);
		gdk_draw_line(pixmap1,
		      win->style->fg_gc[0],
		      0, gnewt->FontSizeH * (height + 1) - 7,
		      gnewt->FontSizeW * (width + 1) - 3,
		      gnewt->FontSizeH * (height + 1) - 7);
		gdk_draw_line(pixmap1, win->style->fg_gc[0],
		      gnewt->FontSizeW * (width + 1) - 3, 0,
		      gnewt->FontSizeW * (width + 1) - 3,
		      gnewt->FontSizeH * (height + 1) - 7);

		gdk_draw_line(pixmap, win->style->fg_gc[0], 3, 8,
		      titleO - gnewt->FontSizeW, 8);
		gdk_draw_line(pixmap, win->style->fg_gc[0],
		      titleO + gnewt->FontSizeW * (titleW - 2), 8,
		      gnewt->FontSizeW * (width + 2) - 3, 8);
	} else {
		gdk_draw_line(pixmap,
		      win->style->white_gc,
		      0, 0, 0, gnewt->FontSizeH * (height + 2) - 1);
		gdk_draw_line(pixmap,
		      win->style->fg_gc[0],
		      1, gnewt->FontSizeH * (height + 2) - 1,
		      gnewt->FontSizeW * (width + 2) - 1,
		      gnewt->FontSizeH * (height + 2) - 1);
		gdk_draw_line(pixmap,
		      win->style->dark_gc[0],
		      3, gnewt->FontSizeH * (height + 2) - 2,
		      gnewt->FontSizeW * (width + 2) - 2,
		      gnewt->FontSizeH * (height + 2) - 2);

		gdk_draw_line(pixmap, win->style->fg_gc[0],
		      gnewt->FontSizeW * (width + 2) - 1, 1,
		      gnewt->FontSizeW * (width + 2) - 1,
		      gnewt->FontSizeH * (height + 2) - 1);
		gdk_draw_line(pixmap, win->style->dark_gc[0],
		      gnewt->FontSizeW * (width + 2) - 2, 3,
		      gnewt->FontSizeW * (width + 2) - 2,
		      gnewt->FontSizeH * (height + 2) - 2);

		gdk_draw_line(pixmap1,
		      win->style->fg_gc[0],
		      0, gnewt->FontSizeH * (height + 1) - 1,
		      gnewt->FontSizeW * (width + 1) - 1,
		      gnewt->FontSizeH * (height + 1) - 1);
		gdk_draw_line(pixmap1,
		      win->style->dark_gc[0],
		      0, gnewt->FontSizeH * (height + 1) - 2,
		      gnewt->FontSizeW * (width + 1) - 2,
		      gnewt->FontSizeH * (height + 1) - 2);

		gdk_draw_line(pixmap1, win->style->fg_gc[0],
		      gnewt->FontSizeW * (width + 1) - 1, 0,
		      gnewt->FontSizeW * (width + 1) - 1,
		      gnewt->FontSizeH * (height + 1) - 1);
		gdk_draw_line(pixmap1, win->style->dark_gc[0],
		      gnewt->FontSizeW * (width + 1) - 2, 0,
		      gnewt->FontSizeW * (width + 1) - 2,
		      gnewt->FontSizeH * (height + 1) - 2);

		gdk_draw_line(pixmap, win->style->white_gc, 0, 0,
		      gnewt->FontSizeW * (width + 2) - 1, 0);

	}
	if (titleO) {
		gdk_draw_string(pixmap, window->style->font,
				window->style->fg_gc[0], titleO,
				window->style->font->ascent + 1,
				gnewt->currentWindow->title);
		if (!gnewt->useNewtColor) {
			gdk_draw_line(pixmap, 
				win->style->black_gc, 
				titleO - 2, 
				gnewt->FontSizeH - 1,
		      		titleO + gdk_string_width(
					window->style->font,
					gnewt->currentWindow->title) + 2,
				gnewt->FontSizeH - 1);
		}
	}
	gtk_widget_show(bg);
	gtk_widget_show(bg1);
	gdk_pixmap_unref(pixmap);
	gdk_pixmap_unref(pixmap1);
	/*newtRefresh();*/
}

int newtCenteredWindow(int width, int height, const char *title)
{
	int top, left;

	top = (ScreenRows - height) / 2;

	/* I don't know why, but this seems to look better */
	if ((ScreenRows % 2) && (top % 2))
		top--;

	left = (ScreenCols - width) / 2;

	newtOpenWindow(left, top, width, height, title);

	return 0;
}

void newtPopWindow(void)
{
	int row, col;

	row = col = 0;
	if (!motherWindow)
		return;

	gtk_widget_destroy(gnewt->currentWindow->widget);
	gnewt->currentWindow->widget = NULL;
	gtk_widget_destroy(gnewt->currentWindow->bg_widget);
	if (!gnewt->showBackground) {
		gtk_widget_destroy(gnewt->currentWindow->top_widget);
	}

	free(gnewt->currentWindow->buffer);
	free(gnewt->currentWindow->title);

	if (gnewt->currentWindow == windowStack) {
		gnewt->currentWindow = NULL;
		gnewt->currentParent = gnewt->gRootWindow;
	} else {
		gnewt->currentWindow--;
		gnewt->currentParent = gnewt->currentWindow->widget;
	}

	newtRefresh();
}

void newtGetrc(int *row, int *col)
{
	*row = cursorRow;
	*col = cursorCol;
}

void newtGotorc(int newRow, int newCol)
{
	if (gnewt->currentWindow) {
		newRow += gnewt->currentWindow->top;
		newCol += gnewt->currentWindow->left;
	}

	cursorRow = newRow;
	cursorCol = newCol;
}

void newtDrawBox(int left, int top, int width, int height, int shadow)
{
	if (gnewt->currentWindow) {
		top += gnewt->currentWindow->top;
		left += gnewt->currentWindow->left;
	}

}

void newtClearBox(int left, int top, int width, int height)
{
	if (gnewt->currentWindow) {
		top += gnewt->currentWindow->top;
		left += gnewt->currentWindow->left;
	}

}

void newtDelay(int usecs)
{
	fd_set set;
	struct timeval tv;

	FD_ZERO(&set);

	tv.tv_sec = usecs / 1000000;
	tv.tv_usec = usecs % 1000000;

	select(0, &set, &set, &set, &tv);
}

struct eventResult newtDefaultEventHandler(newtComponent c,
					   struct event ev)
{
	struct eventResult er;

	er.result = ER_IGNORED;
	return er;
}

void newtRedrawHelpLine(void)
{
	GtkStyle *style;
	GtkWidget *win;
	if (!currentHelpline) {
		return;
	}
	if (!currentHelpWidget) {
		/* creat a background for the help line */
		win = gtk_fixed_new();
		style = gtk_style_copy(gnewt->gRootStyle);
		if (gnewt->useNewtColor) {
			style->bg[0] = getColorByName(
					gnewt->globalColors->helpLineBg);
			style->fg[0] = getColorByName(
					gnewt->globalColors->helpLineFg);
		}
		gtk_widget_set_style(win, style);
		gtk_widget_set_usize(win, ScreenW, gnewt->FontSizeH + 3);
		gtk_fixed_put(GTK_FIXED(gnewt->gRootWindow), win,
			      0, ScreenH - gnewt->FontSizeH - 3);
		gtk_widget_show(win);

		currentHelpWidget = gtk_statusbar_new();
		gtk_widget_set_usize(currentHelpWidget, ScreenW,
				     gnewt->FontSizeH + 3);
		gtk_fixed_put(GTK_FIXED(win), currentHelpWidget, 1, 0);
		gtk_widget_set_style(
				     (GTK_STATUSBAR(currentHelpWidget)->
				      label), style);
		gtk_widget_show(currentHelpWidget);
	}
	gtk_statusbar_push(GTK_STATUSBAR(currentHelpWidget), 1,
			   *currentHelpline);
}

void newtPushHelpLine(const char *text)
{
	if (!text)
		text = defaultHelpLine;

	if (currentHelpline)
		(*(++currentHelpline)) = strdup(text);
	else {
		currentHelpline = helplineStack;
		*currentHelpline = strdup(text);
	}

	newtRedrawHelpLine();
}

void newtPopHelpLine(void)
{
	if (!currentHelpline)
		return;

	free(*currentHelpline);
	if (currentHelpline == helplineStack) {
		currentHelpline = NULL;
		gtk_widget_destroy(currentHelpWidget);
		currentHelpWidget = NULL;
	} else {
		currentHelpline--;
	}

	newtRedrawHelpLine();

}

void extendedFeatures(const char *text)
{
	GtkWidget *wpixmap;
	GdkPixmap *pixmap;
	GdkBitmap *mask;
	char *cmd[15];
	char *txt = strdup(text);
	int i = 1;
	cmd[0] = txt;
	cmd[1] = "";
	while (*txt) {
		if (*txt == '\v') {
			*txt = '\0';
			cmd[i] = txt + 1;
			i++;
		}
		txt++;
	}
	/* printf ("%s -- extended!!\n", text); */
	if (!strcmp(cmd[1], "root_xpm")) {
		pixmap = gdk_pixmap_create_from_xpm(gnewt->gRootWindow->window,
						    &mask,
						    &gnewt->gRootStyle->
						    bg[GTK_STATE_NORMAL],
						    cmd[2]);
		if (pixmap) {
			wpixmap = gtk_pixmap_new(pixmap, mask);
			gdk_pixmap_unref(pixmap);
			gdk_pixmap_unref(mask);
			gtk_fixed_put(GTK_FIXED(gnewt->gRootWindow),
				      wpixmap, atoi(cmd[3]) * gnewt->FontSizeW,
				      atoi(cmd[4]) * gnewt->FontSizeH);
			gtk_widget_show(wpixmap);
		}
	}
	free(cmd[0]);
}

void newtDrawRootText(int col, int row, const char *text)
{
	GtkWidget *w;
	GtkStyle *style;

	if (col < 0) {
		col = ScreenCols + col;
	}

	if (row < 0) {
		row = ScreenRows + row;
	}

	w = gtk_label_new((char *) text);
	style = gtk_style_copy(gnewt->gRootStyle);
	if (gnewt->useNewtColor) {
		style->fg[0] = getColorByName(gnewt->globalColors->rootTextFg);
	}
	gtk_widget_set_style(w, style);
	gtk_widget_show(w);
	gtk_fixed_put(GTK_FIXED(gnewt->gRootWindow), w,
		      gnewt->FontSizeW * col, gnewt->FontSizeH * row);
}

int newtSetFlags(int oldFlags, int newFlags, enum newtFlagsSense sense)
{
	switch (sense) {
	case NEWT_FLAGS_SET:
		return oldFlags | newFlags;

	case NEWT_FLAGS_RESET:
		return oldFlags & (~newFlags);

	case NEWT_FLAGS_TOGGLE:
		return oldFlags ^ newFlags;

	default:
		return oldFlags;
	}
}

void newtBell(void)
{
}

void newtGetScreenSize(int *cols, int *rows)
{
	if (rows)
		*rows = ScreenRows;
	if (cols)
		*cols = ScreenCols;
}

void newtDefaultPlaceHandler(newtComponent c, int newLeft, int newTop)
{
	c->left = newLeft;
	c->top = newTop;
}

void newtDefaultMappedHandler(newtComponent c, int isMapped)
{
	c->isMapped = isMapped;
}
