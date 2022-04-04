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

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <stdio.h>

#include "newt.h"
#include "newt_pr.h"
#include "gnewt.h"
#include "gnewt_pr.h"

struct textbox {
	char **lines;
	int numLines;
	int linesAlloced;
	int doWrap;
	newtComponent sb;
	int topLine;
	int textWidth;
	GtkWidget *widget;
	GtkWidget *parent;
	GtkWidget **label;
};
extern struct Gnewt *gnewt;

static char *expandTabs(const char *text);
static void textboxDraw(newtComponent co);
static void addLine(newtComponent co, const char *s, int len);
static void doReflow(const char *text, char **resultPtr, int width,
		     int *badness, int *heightPtr);
static struct eventResult textboxEvent(newtComponent c, struct event ev);
static void textboxDestroy(newtComponent co);
static void textboxPlace(newtComponent co, int newLeft, int newTop);
static void textboxMapped(newtComponent co, int isMapped);

static struct componentOps textboxOps = {
	textboxDraw,
	textboxEvent,
	textboxDestroy,
	textboxPlace,
	textboxMapped,
};

static void textboxMapped(newtComponent co, int isMapped)
{
	struct textbox *tb = co->data;

	co->isMapped = isMapped;
	if (tb->sb)
		tb->sb->ops->mapped(tb->sb, isMapped);
}

static void textboxPlace(newtComponent co, int newLeft, int newTop)
{
	struct textbox *tb = co->data;

	co->top = newTop;
	co->left = newLeft;

	if (tb->sb)
		tb->sb->ops->place(tb->sb, co->left + co->width - 1,
				   co->top);
}

void newtTextboxSetHeight(newtComponent co, int height)
{
	co->height = height;
}

int newtTextboxGetNumLines(newtComponent co)
{
	struct textbox *tb = co->data;

	return (tb->numLines);
}

newtComponent newtTextboxReflowed(int left, int top, char *text, int width,
				  int flexDown, int flexUp, int flags)
{
	newtComponent co;
	char *reflowedText;
	int actWidth, actHeight;

	reflowedText = newtReflowText(text, width, flexDown, flexUp,
				      &actWidth, &actHeight);

	co = newtTextbox(left, top, actWidth, actHeight, NEWT_FLAG_WRAP);
	newtTextboxSetText(co, reflowedText);
	free(reflowedText);

	return co;
}

newtComponent newtTextbox(int left, int top, int width, int height,
			  int flags)
{
	newtComponent co;
	struct textbox *tb;

	co = malloc(sizeof(*co));
	tb = malloc(sizeof(*tb));
	co->data = tb;

	co->ops = &textboxOps;

	/* workaround for dialogboxes.c */
	if (width < -1)
		width = 78 + width;
	if (height < -1)
		height = 22 + height;

	if (left <= 0)
		left = 0;
	if (top <= 0)
		top = 0;
	if (height <= 0)
		height = 0;
	if (width <= 0)
		width = 0;

	co->height = height;
	co->top = top;
	co->left = left;
	co->takesFocus = 0;
	co->width = width;

	tb->doWrap = flags & NEWT_FLAG_WRAP;
	tb->numLines = 0;
	tb->linesAlloced = 0;
	tb->lines = NULL;
	tb->topLine = 0;
	tb->textWidth = width;
	tb->label = NULL;

	tb->widget = NULL; /*gtk_fixed_new();*/
	tb->parent = NULL;

	if (flags & NEWT_FLAG_SCROLL) {
		co->width += 2;
		tb->sb =
		    newtVerticalScrollbar(co->left + co->width - 1,
					  co->top, co->height,
					  COLORSET_TEXTBOX,
					  COLORSET_TEXTBOX);
	} else {
		tb->sb = NULL;
	}

	return co;
}

static char *expandTabs(const char *text)
{
	int bufAlloced = strlen(text) + 40;
	char *buf, *dest;
	const char *src;
	int bufUsed = 0;
	int lineChar = 0;
	int i;

	buf = malloc(bufAlloced + 1);
	for (src = text, dest = buf; *src; src++) {
		if ((bufUsed + 10) > bufAlloced) {
			bufAlloced += strlen(text) / 2;
			buf = realloc(buf, bufAlloced + 1);
			dest = buf + bufUsed;
		}
		if (*src == '\t') {
			i = 8 - (lineChar & 7);
			if (!i)
				i = 8;
			memset(dest, ' ', i);
			dest += i, bufUsed += i;
			lineChar += i;
		} else {
			*dest++ = *src;
			bufUsed++;
			lineChar++;
			if (*src == '\n')
				lineChar = 0;
		}
	}

	*dest = '\0';
	return buf;
}

static void doReflow(const char *text, char **resultPtr, int width,
		     int *badness, int *heightPtr)
{
	char *result = NULL, *str;
	const char *chptr, *end;
	int howbad = 0;
	int height = 0;
	int wid;
	int len = 0;
	/* width -= 2; */

	if (resultPtr) {
		/* XXX I think this will work */
		result = malloc(strlen(text) + (strlen(text) / width) + 2);
		*result = '\0';
	}
	str = strdup(text);
	while (*text) {
		end = strchr(text, '\n');
		if (!end)
			end = text + strlen(text);

		while (*text && text <= end) {
			len = end - text;		
			strncpy(str, text, len);
			*(str + len) = '\0';
			wid = gdk_string_width(gnewt->gRootStyle->font, str);
			if (wid <= (width * gnewt->FontSizeW)) 
			{
				if (result) {
					strncat(result, text, end - text);
					strcat(result, "\n");
					height++;
				}

				if (end - text < (width / 2))
					howbad +=
					    ((width / 2) -
					     (end - text)) / 2;

				text = end;
				if (*text)
					text++;
			} else {
				len--;
				*(str + len) = '\0';
				while (gdk_string_width(
					gnewt->gRootStyle->font, str) >
					(width * gnewt->FontSizeW)) 
				{
					len--;
					*(str + len) = '\0';
				}
				chptr = text + len - 1;

				while (chptr > text && !isspace(*chptr))
					chptr--;
				while (chptr > text && isspace(*chptr))
					chptr--;
				chptr++;

				if (chptr - text == 1 && !isspace(*chptr))
					chptr = text + len - 1;
					

				if (chptr > text)
					howbad +=
					    width - (chptr - text) + 1;
				if (result) {
					strncat(result, text,
						chptr - text);
					strcat(result, "\n");
					height++;
				}

				if (isspace(*chptr))
					text = chptr + 1;
				else
					text = chptr;
				while (isspace(*text))
					text++;
			}
		}
	}
	free(str);
	if (badness)
		*badness = howbad;
	if (resultPtr)
		*resultPtr = result;
	if (heightPtr)
		*heightPtr = height;
}

char *newtReflowText(char *text, int width, int flexDown, int flexUp,
		     int *actualWidth, int *actualHeight)
{
	int min, max;
	int i;
	char *result;
	int minbad, minbadwidth, howbad;
	char *expandedText;

	expandedText = expandTabs(text);

	if (flexDown || flexUp) {
		min = width - flexDown;
		max = width + flexUp;

		minbad = -1;
		minbadwidth = width;

		for (i = min; i <= max; i++) {
			doReflow(expandedText, NULL, i, &howbad, NULL);

			if (minbad == -1 || howbad < minbad) {
				minbad = howbad;
				minbadwidth = i;
			}
		}

		width = minbadwidth;
	}
	doReflow(expandedText, &result, width, NULL, actualHeight);
	free(expandedText);
	if (actualWidth)
		*actualWidth = width;
	return result;
}

void newtTextboxSetText(newtComponent co, const char *text)
{
	const char *start, *end;
	struct textbox *tb = co->data;
	char *reflowed, *expanded;
	int badness, height, i;

	/* clear original Textbox if exists */
	if (tb->label) {
		for (i = 0; i < tb->linesAlloced; i++) {
			if ((tb->label[i]) && (GTK_IS_LABEL(tb->label[i]))) {
				gtk_widget_destroy(tb->label[i]);
			}
		}
		free(tb->label);
		tb->label = NULL;
	}
	if (tb->lines) {
		for (i = 0; i < tb->linesAlloced; i++) {
			if (tb->lines[i]) {
				free(tb->lines[i]);
				tb->lines[i] = NULL;
			}
		}
		free(tb->lines);
		tb->lines = NULL;
		tb->linesAlloced = tb->numLines = 0;
	}

	expanded = expandTabs(text);

	if (tb->doWrap) {
		doReflow(expanded, &reflowed, tb->textWidth, &badness,
			 &height);
		free(expanded);
		expanded = reflowed;
	}

	for (start = expanded; *start; start++)
		if (*start == '\n')
			tb->linesAlloced++;

	/* This ++ leaves room for an ending line w/o a \n */
	tb->linesAlloced++;
	tb->lines = malloc(sizeof(char *) * tb->linesAlloced);
	tb->label = malloc(sizeof(GtkWidget *) * tb->linesAlloced);
	for (i = 0; i < tb->linesAlloced; i++) {
		tb->label[i] = NULL;
		tb->lines[i] = NULL;
	}
	start = expanded;
	while ((end = strchr(start, '\n'))) {
		addLine(co, start, end - start);
		start = end + 1;
	}

	if (*start)
		addLine(co, start, strlen(start));

	free(expanded);
}

/* This assumes the buffer is allocated properly! */
static void addLine(newtComponent co, const char *s, int len)
{
	struct textbox *tb = co->data;

	tb->lines[tb->numLines] = malloc(len + 1);
	memset(tb->lines[tb->numLines], ' ', len);
	memcpy(tb->lines[tb->numLines], s, len);
	tb->lines[tb->numLines++][len] = '\0';
}

static void textboxDraw(newtComponent c)
{
	int i;
	struct textbox *tb = c->data;
	GtkStyle *style;
	GtkWidget *scrolled_win;

	if (!tb->widget || !GTK_IS_WIDGET(tb->widget)) {
		tb->widget = gtk_fixed_new();
		tb->parent = gnewt->currentParent;
		if (c->width && c->height) {
			gtk_widget_show(tb->widget);
		}
		if (!tb->sb) {
			gtk_widget_set_usize(tb->widget,
					     gnewt->FontSizeW * c->width,
					     gnewt->FontSizeH * c->height);
			GTK_CONTAINER(tb->widget)->border_width = 0;
			if (GTK_IS_FIXED(tb->parent)) {
				gtk_fixed_put(GTK_FIXED(tb->parent),
					      tb->widget,
					      gnewt->FontSizeW * c->left,
					      gnewt->FontSizeH * c->top);
			} else {
				gtk_container_add(GTK_CONTAINER
						  (tb->parent),
						  tb->widget);
			}
		} else {
			scrolled_win = gtk_scrolled_window_new(NULL, NULL);
			gtk_widget_set_usize(scrolled_win,
					     gnewt->FontSizeW * (c->width),
					     gnewt->FontSizeH * c->height);
			gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW
						       (scrolled_win),
						       GTK_POLICY_NEVER,
						       GTK_POLICY_AUTOMATIC);
			gtk_widget_set_usize(tb->widget,
					     gnewt->FontSizeW * c->width,
					     gnewt->FontSizeH * tb->numLines);
			GTK_CONTAINER(tb->widget)->border_width = 0;

			gtk_fixed_put(GTK_FIXED(tb->parent), scrolled_win,
				      gnewt->FontSizeW * c->left,
				      gnewt->FontSizeH * c->top);

			gtk_scrolled_window_add_with_viewport
			    (GTK_SCROLLED_WINDOW(scrolled_win),
			     tb->widget);

			gtk_container_set_focus_vadjustment(GTK_CONTAINER
					  (tb->widget),
					  gtk_scrolled_window_get_vadjustment
					  (GTK_SCROLLED_WINDOW (scrolled_win)));
			if (c->width && c->height) {
				gtk_widget_show(scrolled_win);
			}
		}
		gdk_window_set_background(tb->widget->window,
					  &tb->parent->style->
					  bg[GTK_STATE_NORMAL]);
	}

	style = gtk_style_copy(tb->widget->style);
	if (gnewt->useNewtColor) {
		style->fg[0] = getColorByName(gnewt->globalColors->textboxFg);
	}
	gtk_widget_push_style(style);

	for (i = 0; (i + tb->topLine) < tb->numLines; i++) {
		if (tb->label[i] && GTK_IS_LABEL(tb->label[i])) {
			gtk_widget_destroy(tb->label[i]);
		}
		tb->label[i] = gtk_label_new(tb->lines[i]);
		gtk_widget_show(tb->label[i]);
		gtk_fixed_put(GTK_FIXED(tb->widget), tb->label[i], 2,
			      gnewt->FontSizeH * i);
	}
	gtk_widget_pop_style();
	/*newtRefresh();*/
}

static struct eventResult textboxEvent(newtComponent co, struct event ev)
{
	struct textbox *tb = co->data;
	struct eventResult er;

	er.result = ER_IGNORED;

	if (ev.when == EV_EARLY && ev.event == EV_KEYPRESS && tb->sb) {
		switch (ev.u.key) {
		case NEWT_KEY_UP:
			if (tb->topLine)
				tb->topLine--;
			textboxDraw(co);
			er.result = ER_SWALLOWED;
			break;

		case NEWT_KEY_DOWN:
			if (tb->topLine < (tb->numLines - co->height))
				tb->topLine++;
			textboxDraw(co);
			er.result = ER_SWALLOWED;
			break;

		case NEWT_KEY_PGDN:
			tb->topLine += co->height;
			if (tb->topLine > (tb->numLines - co->height)) {
				tb->topLine = tb->numLines - co->height;
				if (tb->topLine < 0)
					tb->topLine = 0;
			}
			textboxDraw(co);
			er.result = ER_SWALLOWED;
			break;

		case NEWT_KEY_PGUP:
			tb->topLine -= co->height;
			if (tb->topLine < 0)
				tb->topLine = 0;
			textboxDraw(co);
			er.result = ER_SWALLOWED;
			break;
		}
	}

	return er;
}

static void textboxDestroy(newtComponent co)
{
	int i;
	struct textbox *tb = co->data;

	for (i = 0; i < tb->numLines; i++)
		free(tb->lines[i]);

	free(tb->lines);
	free(tb);
	free(co);
}
