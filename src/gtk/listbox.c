/* This goofed-up box whacked into shape by Elliot Lee <sopwith@cuc.edu>
   (from the original listbox by Erik Troan <ewt@redhat.com>)
   and contributed to newt for use under the LGPL license.
   Copyright (C) 1996, 1997 Elliot Lee */

/*   Gtk+ version of the newt library
 *
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <stdio.h>

#include "newt.h"
#include "newt_pr.h"
#include "gnewt.h"
#include "gnewt_pr.h"

/* Linked list of items in the listbox */
struct items {
	char *text;
	const void *data;
	unsigned char isSelected;
	struct items *next;
	GtkWidget *widget;
	GtkWidget *parent;
	newtComponent parentObj;
	int num;
};

/* Holds all the relevant information for this listbox */
struct listbox {
	newtComponent sb;	/* Scrollbar on right side of listbox */
	int curWidth;		/* size of text w/o scrollbar or border */
	int curHeight;		/* size of text w/o border */
	int sbAdjust;
	int bdxAdjust, bdyAdjust;
	int numItems, numSelected;
	int userHasSetWidth;
	int currItem, startShowItem;	/* startShowItem is the first item 
					   displayed on the screen */
	int isActive;		/* If we handle key events all the time, 
				   it seems to do things even when they are 
				   supposed to be for another button/whatever */
	struct items *boxItems;
	int grow;
	int flags;		/* flags for this listbox, right now just
				   NEWT_FLAG_RETURNEXIT */
	GtkWidget *widget;
	GtkWidget *parent;
};

extern struct Gnewt *gnewt;

static void listboxDraw(newtComponent co);
static void listboxDestroy(newtComponent co);
static struct eventResult listboxEvent(newtComponent co, struct event ev);
static void newtListboxRealSetCurrent(newtComponent co);
static void listboxPlace(newtComponent co, int newLeft, int newTop);
static inline void updateWidth(newtComponent co, struct listbox *li,
			       int maxField);
static void listboxMapped(newtComponent co, int isMapped);

static struct componentOps listboxOps = {
	listboxDraw,
	listboxEvent,
	listboxDestroy,
	listboxPlace,
	listboxMapped,
};

static void listboxMapped(newtComponent co, int isMapped)
{
	struct listbox *li = co->data;

	co->isMapped = isMapped;
	if (li->sb)
		li->sb->ops->mapped(li->sb, isMapped);
}

static void listboxPlace(newtComponent co, int newLeft, int newTop)
{
	struct listbox *li = co->data;

	co->top = newTop;
	co->left = newLeft;

	if (li->sb)
		li->sb->ops->place(li->sb, co->left + co->width - 1,
				   co->top);
}

newtComponent newtListbox(int left, int top, int height, int flags)
{
	newtComponent co, sb;
	struct listbox *li;

	if (!(co = malloc(sizeof(*co))))
		return NULL;

	if (!(li = malloc(sizeof(struct listbox)))) {
		free(co);
		return NULL;
	}

	/* dialogboxes.c workaround */
	if (left < -1)
		left = 78 + left;
	if (top < -1)
		top = 22 + top;
	if (height < -1)
		height = 22 + height;

	if (left < 0)
		left = 0;
	if (top < 0)
		top = 0;
	if (height < 0)
		height = 0;

	li->boxItems = NULL;
	li->numItems = 0;
	li->currItem = 0;
	li->numSelected = 0;
	li->isActive = 0;
	li->userHasSetWidth = 0;
	li->startShowItem = 0;
	li->sbAdjust = 0;
	li->bdxAdjust = 0;
	li->bdyAdjust = 0;
	li->flags = flags & (NEWT_FLAG_RETURNEXIT | NEWT_FLAG_BORDER |
			     NEWT_FLAG_MULTIPLE);

	if (li->flags & NEWT_FLAG_BORDER) {
		li->bdxAdjust = 2;
		li->bdyAdjust = 1;
	}
	co->height = height;
	li->curHeight = co->height - (2 * li->bdyAdjust);
	if (height) {
		li->grow = 0;
		if (flags & NEWT_FLAG_SCROLL) {
			sb =
			    newtVerticalScrollbar(left,
						  top + li->bdyAdjust,
						  li->curHeight,
						  COLORSET_LISTBOX,
						  COLORSET_ACTLISTBOX);
			li->sbAdjust = 3;
		} else {
			sb = NULL;
		}
	} else {
		li->grow = 1;
		sb = NULL;
	}

	li->sb = sb;
	li->widget = NULL; /*gtk_list_new();*/
	li->parent = NULL;

	co->data = li;
	co->isMapped = 0;
	co->left = left;
	co->top = top;
	co->ops = &listboxOps;
	co->takesFocus = 1;
	co->callback = NULL;

	updateWidth(co, li, 5);

	return co;
}

static inline void updateWidth(newtComponent co, struct listbox *li,
			       int maxField)
{
	li->curWidth = maxField;
	co->width = li->curWidth + li->sbAdjust + 2 * li->bdxAdjust;
	if (gnewt->currentWindow &&
	    co->width > gnewt->currentWindow->width - co->left - 1) {
		co->width = gnewt->currentWindow->width - co->left - 1;
	}
	if (li->sb)
		li->sb->left = co->left + co->width - 1;
}

void newtListboxSetCurrentByKey(newtComponent co, void *key)
{
	struct listbox *li = co->data;
	struct items *item;
	int i;

	item = li->boxItems, i = 0;
	while (item && item->data != key)
		item = item->next, i++;

	if (item)
		newtListboxSetCurrent(co, i);
}

void newtListboxSetCurrent(newtComponent co, int num)
{
	struct listbox *li = co->data;

	if (num >= li->numItems)
		li->currItem = li->numItems - 1;
	else if (num < 0)
		li->currItem = 0;
	else
		li->currItem = num;

	if (li->currItem < li->startShowItem)
		li->startShowItem = li->currItem;
	else if (li->currItem - li->startShowItem > co->height - 1)
		li->startShowItem = li->currItem - co->height + 1;
	if (li->startShowItem + co->height > li->numItems)
		li->startShowItem = li->numItems - co->height;
	if (li->startShowItem < 0)
		li->startShowItem = 0;

	newtListboxRealSetCurrent(co);
}

static void newtListboxRealSetCurrent(newtComponent co)
{
	struct listbox *li = co->data;
	struct items *item;
	item = li->boxItems;
	if (li->sb)
		newtScrollbarSet(li->sb, li->currItem + 1, li->numItems);

	listboxDraw(co);
	if (co->callback)
		co->callback(co, co->callbackData);
}

void newtListboxSetWidth(newtComponent co, int width)
{
	struct listbox *li = co->data;

	co->width = width;
	li->curWidth = co->width - li->sbAdjust - 2 * li->bdxAdjust;
	li->userHasSetWidth = 1;
	if (li->sb)
		li->sb->left = co->width + co->left - 1;
	listboxDraw(co);
}

void *newtListboxGetCurrent(newtComponent co)
{
	struct listbox *li = co->data;
	int i;
	struct items *item;

	for (i = 0, item = li->boxItems; item != NULL && i < li->currItem;
	     i++, item = item->next);

	if (item)
		return (void *) item->data;
	else
		return NULL;
}

void newtListboxSelectItem(newtComponent co, const void *key,
			   enum newtFlagsSense sense)
{
	struct listbox *li = co->data;
	int i;
	struct items *item;

	item = li->boxItems, i = 0;
	while (item && item->data != key)
		item = item->next, i++;
	if (!item)
		return;

	if (item->isSelected)
		li->numSelected--;

	switch (sense) {
	case NEWT_FLAGS_RESET:
		item->isSelected = 0;
		break;
	case NEWT_FLAGS_SET:
		item->isSelected = 1;
		break;
	case NEWT_FLAGS_TOGGLE:
		item->isSelected = !item->isSelected;
	}
	if (item->isSelected)
		li->numSelected++;
}

void newtListboxClearSelection(newtComponent co)
{
	struct items *item;
	struct listbox *li = co->data;

	for (item = li->boxItems; item != NULL; item = item->next)
		item->isSelected = 0;
	li->numSelected = 0;
	listboxDraw(co);
}

/* Free the returned array after use, but NOT the values in the array */
void **newtListboxGetSelection(newtComponent co, int *numitems)
{
	struct listbox *li;
	int i;
	void **retval;
	struct items *item;

	if (!co || !numitems)
		return NULL;

	li = co->data;
	*numitems = li->numSelected;
	if (!li || !li->numSelected)
		return NULL;

	retval = malloc(li->numSelected * sizeof(void *));
	for (i = 0, item = li->boxItems; item != NULL; item = item->next)
		if (item->isSelected)
			retval[i++] = (void *) item->data;
	return retval;
}

void newtListboxSetText(newtComponent co, int num, const char *text)
{
	struct listbox *li = co->data;
	int i;
	struct items *item;

	for (i = 0, item = li->boxItems; item != NULL && i < num;
	     i++, item = item->next);

	if (!item)
		return;
	else {
		free(item->text);
		item->text = strdup(text);
	}
	if (li->userHasSetWidth == 0 && strlen(text) > li->curWidth) {
		updateWidth(co, li, strlen(text));
	}

	if (num >= li->startShowItem
	    && num <= li->startShowItem + co->height) listboxDraw(co);
}

void newtListboxSetEntry(newtComponent co, int num, const char *text)
{
	newtListboxSetText(co, num, text);
}

void newtListboxSetData(newtComponent co, int num, void *data)
{
	struct listbox *li = co->data;
	int i;
	struct items *item;

	for (i = 0, item = li->boxItems; item != NULL && i < num;
	     i++, item = item->next);

	item->data = data;
}

static char *formatText (const char *text) {
	char *txt, *txt1;
	int i = 0, tab, done;

	if (text == NULL && strlen(text) < 1) 
		return(strdup(" "));
	txt1 = txt = strdup(text);
	txt++;
	i++;
	done = 0;
	tab = 0;
	while (!done && *txt) {
		if (*txt == ' ' && *(txt - 1) == ' ') {
			tab = i;
		} else if (tab) {
			done = 1;
		}
		txt++;
		i++;
	}
	if (!tab)
		tab = strlen(text);
	free(txt1);
	txt = malloc(strlen(text) + 100);
	memset(txt, 0, strlen(text) + 100);
	strncpy(txt, text, tab);
	i = tab;
	while ((gnewt->FontSizeW * tab) >
	       gdk_string_width(gnewt->gRootStyle->font, txt) && 
	       i < 100 && done) 
	{
		txt[i] = ' ';
		i++;
		txt[i] = (char) 0;
	}
	strcat(txt, (text + tab));
	return txt;
}

int newtListboxAppendEntry(newtComponent co, const char *text,
			   const void *data)
{
	struct listbox *li = co->data;
	struct items *item;
	/*GtkStyle *style;*/
	/*char *txt;*/

	if (li->boxItems) {
		for (item = li->boxItems; item->next != NULL;
		     item = item->next);
		item = item->next = malloc(sizeof(struct items));
	} else {
		item = li->boxItems = malloc(sizeof(struct items));
	}
	item->parent = NULL;
	item->widget = NULL;
/*
	txt = formatText(text);
	item->widget = gtk_list_item_new_with_label(txt);
	free(txt);
	item->parent = NULL;
	style = gtk_style_copy(gnewt->gRootStyle);

	gtk_widget_set_style(GTK_BIN(item->widget)->child, style);
*/
	if (!li->userHasSetWidth && text && (strlen(text) > li->curWidth))
		updateWidth(co, li, strlen(text));

	item->text = strdup(text ? text : "(null)");
	item->data = data;
	item->next = NULL;
	item->isSelected = 0;

	if (li->grow)
		co->height++, li->curHeight++;
	li->numItems++;

	return 0;
}


int newtListboxInsertEntry(newtComponent co, const char *text,
			   const void *data, void *key)
{
	struct listbox *li = co->data;
	struct items *item, *t;
	/*GtkStyle *style;*/
	/*char *txt;*/
	int i = 0, tab, done;

	if (li->boxItems) {
		if (key) {
			item = li->boxItems;
			while (item && item->data != key)
				item = item->next;

			if (!item)
				return 1;

			t = item->next;
			item = item->next = malloc(sizeof(struct items));
			item->next = t;
		} else {
			t = li->boxItems;
			item = li->boxItems = malloc(sizeof(struct items));
			item->next = t;
		}
	} else if (key) {
		return 1;
	} else {
		item = li->boxItems = malloc(sizeof(struct items));
		item->next = NULL;
	}
/*
	txt = formatText(text);
	item->widget = gtk_list_item_new_with_label(txt);
	free(txt);
	item->parent = NULL;
	style = gtk_style_copy(gnewt->gRootStyle);

	gtk_widget_set_style(GTK_BIN(item->widget)->child, style);
*/
	if (!li->userHasSetWidth && text && (strlen(text) > li->curWidth))
		updateWidth(co, li, strlen(text));

	item->text = strdup(text ? text : "(null)");
	item->data = data;
	item->isSelected = 0;

	if (li->sb)
		li->sb->left = co->left + co->width - 1;
	li->numItems++;

	return 0;
}

int newtListboxDeleteEntry(newtComponent co, void *key)
{
	struct listbox *li = co->data;
	int widest = 0, t;
	struct items *item, *item2 = NULL;
	int num;

	if (li->boxItems == NULL || li->numItems <= 0)
		return 0;

	num = 0;

	item2 = NULL, item = li->boxItems;
	while (item && item->data != key) {
		item2 = item;
		item = item->next;
		num++;
	}

	if (!item)
		return -1;

	if (item2)
		item2->next = item->next;
	else
		li->boxItems = item->next;

	free(item->text);
	free(item);
	li->numItems--;

	if (!li->userHasSetWidth) {
		widest = 0;
		for (item = li->boxItems; item != NULL; item = item->next)
			if ((t = strlen(item->text)) > widest)
				widest = t;
	}

	if (li->currItem >= num)
		li->currItem--;

	if (!li->userHasSetWidth) {
		updateWidth(co, li, widest);
	}

	listboxDraw(co);

	return 0;
}

void newtListboxClear(newtComponent co)
{
	struct listbox *li;
	struct items *anitem, *nextitem;
	if (co == NULL || (li = co->data) == NULL)
		return;
	for (anitem = li->boxItems; anitem != NULL; anitem = nextitem) {
		nextitem = anitem->next;
		free(anitem->text);
		free(anitem);
	}
	li->numItems = li->numSelected = li->currItem = li->startShowItem =
	    0;
	li->boxItems = NULL;
	if (!li->userHasSetWidth)
		updateWidth(co, li, 5);
}

/* If you don't want to get back the text, pass in NULL for the ptr-ptr. Same
   goes for the data. */
void newtListboxGetEntry(newtComponent co, int num, char **text,
			 void **data)
{
	struct listbox *li = co->data;
	int i;
	struct items *item;

	if (!li->boxItems || num >= li->numItems) {
		if (text)
			*text = NULL;
		if (data)
			*data = NULL;
		return;
	}

	i = 0;
	item = li->boxItems;
	while (item && i < num) {
		i++, item = item->next;
	}

	if (item) {
		if (text)
			*text = item->text;
		if (data)
			*data = (void *) item->data;
	}
}

static void itemCallbackSet(struct items *item)
{
	newtComponent co = item->parentObj;
	struct listbox *li = co->data;
	li->currItem = item->num;
	newtListboxSelectItem(co, item->data, NEWT_FLAGS_SET);
	if (co->callback)
		co->callback(co, co->callbackData);
}

static void itemCallbackButton(struct items *item, GdkEventButton * event)
{
	newtComponent co = item->parentObj;
	struct listbox *li = co->data;
	if (li->flags & NEWT_FLAG_RETURNEXIT
	    && event->type == GDK_2BUTTON_PRESS) {
		gnewt->currentExitComp = co;
		gtk_main_quit();
	}
}

static void itemCallbackReset(struct items *item)
{
	newtComponent co = item->parentObj;
	struct listbox *li = co->data;
	li->currItem = item->num;
	newtListboxSelectItem(co, item->data, NEWT_FLAGS_RESET);
	if (co->callback)
		co->callback(co, co->callbackData);
}

static gint itemCallbackKey(struct items *item, GdkEventKey * event)
{
	newtComponent co = item->parentObj;
	struct listbox *li = co->data;
	if (event->keyval == 65293 && li->flags & NEWT_FLAG_RETURNEXIT) {
		gnewt->currentExitComp = co;
		gtk_main_quit();
	}
	return TRUE;
}

static void listboxDraw(newtComponent co)
{
	struct listbox *li = co->data;
	struct items *item;
	GtkWidget *scrolled_win;
	GtkStyle *style;

	int i, j;

	if (!co->isMapped)
		return;

	if (!li->widget || !GTK_IS_WIDGET(li->widget)) {
		li->widget = gtk_list_new();
		li->parent = gnewt->currentParent;
		gtk_widget_show(li->widget);
		scrolled_win = gtk_scrolled_window_new(NULL, NULL);
		GTK_CONTAINER(li->widget)->border_width = -2;
		GTK_CONTAINER(scrolled_win)->border_width = -1;
		if (li->sb || co->width > 20) {
			gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW
						       (scrolled_win),
						       GTK_POLICY_NEVER,
						       GTK_POLICY_AUTOMATIC);
		} else {
			gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW
						       (scrolled_win),
						       GTK_POLICY_NEVER,
						       GTK_POLICY_NEVER);
		}
		gtk_widget_show(scrolled_win);
		gtk_widget_set_usize(scrolled_win,
				     gnewt->FontSizeW * co->width,
				     gnewt->FontSizeH * co->height);
		gtk_fixed_put(GTK_FIXED(li->parent), scrolled_win,
			      gnewt->FontSizeW * co->left, 
			      gnewt->FontSizeH * co->top);

		if (li->flags & NEWT_FLAG_MULTIPLE) {
			gtk_list_set_selection_mode(GTK_LIST(li->widget),
						    GTK_SELECTION_MULTIPLE);
		} else {
			gtk_list_set_selection_mode(GTK_LIST(li->widget),
						    GTK_SELECTION_SINGLE);
		}
		gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW
						      (scrolled_win),
						      li->widget);

		gtk_container_set_focus_vadjustment(GTK_CONTAINER
					(li->widget),
					gtk_scrolled_window_get_vadjustment
					(GTK_SCROLLED_WINDOW (scrolled_win)));
	}

	style = gtk_style_copy(gnewt->gRootStyle);
	item = li->boxItems;
	j = 0;
	for (i = 0; item != NULL; i++, item = item->next) {
		if (!item->text) {
			continue;
		}
		if (!item->widget || !GTK_IS_WIDGET(item->widget)) {
			char *txt;
			txt = formatText(item->text);
			item->widget = gtk_list_item_new_with_label(txt);
			free(txt);
			gtk_widget_set_style(item->widget, style); 
			gtk_widget_set_style(GTK_BIN(item->widget)->child, 
					style);

			gnewt->currentParent = li->widget;
			item->parent = gnewt->currentParent;
			gtk_widget_show(item->widget);
			item->parentObj = co;
			item->num = i;
			if (!j)
				j = 1;
			gtk_container_add(GTK_CONTAINER(li->widget),
					  item->widget);
			if (item->isSelected) {
				j = 2;
				gtk_list_select_child(GTK_LIST(li->widget),
						      item->widget);
			} else {
				gtk_list_unselect_child(GTK_LIST
							(li->widget),
							item->widget);
			}
			gtk_signal_connect_object(GTK_OBJECT(item->widget),
						  "select",
						  GTK_SIGNAL_FUNC
						  (itemCallbackSet),
						  (GtkObject *) item);

			gtk_signal_connect_object(GTK_OBJECT(item->widget),
						  "deselect",
						  GTK_SIGNAL_FUNC
						  (itemCallbackReset),
						  (GtkObject *) item);

			gtk_signal_connect_object(GTK_OBJECT(item->widget),
						  "key_press_event",
						  GTK_SIGNAL_FUNC
						  (itemCallbackKey),
						  (GtkObject *) item);

			gtk_signal_connect_object(GTK_OBJECT(item->widget),
						  "button_press_event",
						  GTK_SIGNAL_FUNC
						  (itemCallbackButton),
						  (GtkObject *) item);

		}
	}
	if (j == 1 && !(li->flags & NEWT_FLAG_MULTIPLE)) {
		item = li->boxItems;
		for (i = 0; item != NULL; i++, item = item->next) {
			if (i == li->currItem) {
				gtk_list_select_child(GTK_LIST(li->widget),
						      item->widget);
			}
		}
	}
	gnewt->currentParent = li->parent;
	/*newtRefresh();*/
}

static struct eventResult listboxEvent(newtComponent co, struct event ev)
{
	struct eventResult er;
	struct listbox *li = co->data;
	struct items *item;
	int i;

	er.result = ER_IGNORED;

	if (ev.when == EV_EARLY || ev.when == EV_LATE) {
		return er;
	}

	switch (ev.event) {
	case EV_KEYPRESS:
		if (!li->isActive)
			break;

		switch (ev.u.key) {
		case ' ':
			if (!(li->flags & NEWT_FLAG_MULTIPLE))
				break;
			for (i = 0, item = li->boxItems; item != NULL &&
			     i < li->currItem; i++, item = item->next);
			newtListboxSelectItem(co, item->data,
					      NEWT_FLAGS_TOGGLE);
			er.result = ER_SWALLOWED;
			/* We don't break here, because it is cool to be able to
			   hold space to select a bunch of items in a list at 
			   once */

		case NEWT_KEY_DOWN:
			if (li->numItems <= 0)
				break;
			if (li->currItem < li->numItems - 1) {
				li->currItem++;
				if (li->currItem >
				    (li->startShowItem + li->curHeight -
				     1)) {
					li->startShowItem =
					    li->currItem - li->curHeight +
					    1;
					if (li->startShowItem +
					    li->curHeight > li->numItems)
						li->startShowItem =
						    li->numItems -
						    li->curHeight;
				}
				if (li->sb)
					newtScrollbarSet(li->sb,
							 li->currItem + 1,
							 li->numItems);
				listboxDraw(co);
			}
			if (co->callback)
				co->callback(co, co->callbackData);
			er.result = ER_SWALLOWED;
			break;

		case NEWT_KEY_ENTER:
			if (li->numItems <= 0)
				break;
			if (li->flags & NEWT_FLAG_RETURNEXIT)
				er.result = ER_EXITFORM;
			break;

		case NEWT_KEY_UP:
			if (li->numItems <= 0)
				break;
			if (li->currItem > 0) {
				li->currItem--;
				if (li->currItem < li->startShowItem)
					li->startShowItem = li->currItem;
				if (li->sb)
					newtScrollbarSet(li->sb,
							 li->currItem + 1,
							 li->numItems);
				listboxDraw(co);
			}
			if (co->callback)
				co->callback(co, co->callbackData);
			er.result = ER_SWALLOWED;
			break;

		case NEWT_KEY_PGUP:
			if (li->numItems <= 0)
				break;
			li->startShowItem -= li->curHeight - 1;
			if (li->startShowItem < 0)
				li->startShowItem = 0;
			li->currItem -= li->curHeight - 1;
			if (li->currItem < 0)
				li->currItem = 0;
			newtListboxRealSetCurrent(co);
			er.result = ER_SWALLOWED;
			break;

		case NEWT_KEY_PGDN:
			if (li->numItems <= 0)
				break;
			li->startShowItem += li->curHeight;
			if (li->startShowItem >
			    (li->numItems - li->curHeight)) {
				li->startShowItem =
				    li->numItems - li->curHeight;
			}
			li->currItem += li->curHeight;
			if (li->currItem > li->numItems) {
				li->currItem = li->numItems - 1;
			}
			newtListboxRealSetCurrent(co);
			er.result = ER_SWALLOWED;
			break;

		case NEWT_KEY_HOME:
			if (li->numItems <= 0)
				break;
			newtListboxSetCurrent(co, 0);
			er.result = ER_SWALLOWED;
			break;

		case NEWT_KEY_END:
			if (li->numItems <= 0)
				break;
			li->startShowItem = li->numItems - li->curHeight;
			if (li->startShowItem < 0)
				li->startShowItem = 0;
			li->currItem = li->numItems - 1;
			newtListboxRealSetCurrent(co);
			er.result = ER_SWALLOWED;
			break;
		default:
			/* keeps gcc quiet */
			}
			break;

		case EV_FOCUS:
			li->isActive = 1;
			listboxDraw(co);
			er.result = ER_SWALLOWED;
			break;

		case EV_UNFOCUS:
			li->isActive = 0;
			listboxDraw(co);
			er.result = ER_SWALLOWED;
			break;
		case EV_MOUSE:
			er.result = ER_SWALLOWED;
			break;
		}

		return er;
	}

	static void listboxDestroy(newtComponent co) {
		    struct listbox *li = co->data;
		struct items *item, *nextitem;
		 nextitem = item = li->boxItems;

		while (item != NULL) {
			nextitem = item->next;
			if (gnewt->currentWindow
			    && gnewt->currentWindow->widget) {
				gtk_signal_disconnect_by_func(GTK_OBJECT
							      (item->
							       widget),
							      GTK_SIGNAL_FUNC
							      (itemCallbackReset),
							      (GtkObject *)
							      item);
			}
			free(item->text);
			item->next = NULL;
			item->widget = NULL;
			free(item);
			item = nextitem;
		}
		free(li);
		free(co);
	}
