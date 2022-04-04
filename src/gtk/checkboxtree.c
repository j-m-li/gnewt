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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "newt.h"
#include "newt_pr.h"
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include "gnewt.h"
#include "gnewt_pr.h"

struct items {
	char *text;
	void *data;
	unsigned char selected;
	struct items *next;
	struct items *prev;
	struct items *branch;
	int flags;
	int depth;
	GtkWidget *widget;
	GtkWidget *parent;
	newtComponent parentObj;
	int num;
};

extern struct Gnewt *gnewt;

struct CheckboxTree {
	newtComponent sb;
	int curWidth;		/* size of text w/o scrollbar or border */
	int curHeight;		/* size of text w/o border */
	struct items *itemlist;
	struct items **flatList, **currItem, **firstItem;
	int flatCount;
	int flags;
	int pad;
	char *seq;
	char *result;
	GtkWidget *widget;
	GtkWidget *parent;
};

static void ctDraw(newtComponent c);
static void ctDestroy(newtComponent co);
static void ctPlace(newtComponent co, int newLeft, int newTop);
struct eventResult ctEvent(newtComponent co, struct event ev);
static void ctMapped(newtComponent co, int isMapped);
static struct items *findItem(struct items *items, const void *data);
static void buildFlatList(newtComponent co);
static void doBuildFlatList(struct CheckboxTree *ct, struct items *item);
enum countWhat { COUNT_EXPOSED = 0, COUNT_SELECTED = 1 };
static int countItems(struct items *item, enum countWhat justExposed);

static struct componentOps ctOps = {
	ctDraw,
	ctEvent,
	ctDestroy,
	ctPlace,
	ctMapped,
};

static int countItems(struct items *item, enum countWhat what)
{
	int count = 0;

	while (item) {
		if ((!item->branch && item->selected >= what)
		    || (what == COUNT_EXPOSED)) count++;
		if (item->branch
		    || (what == COUNT_EXPOSED
			&& item->selected)) count +=
      countItems(item->branch, what);
		item = item->next;
	}

	return count;
}

static void doBuildFlatList(struct CheckboxTree *ct, struct items *item)
{
	while (item) {
		ct->flatList[ct->flatCount++] = item;
		if (item->branch /*&& item->selected */ )
			doBuildFlatList(ct, item->branch);
		item = item->next;
	}

}

static void buildFlatList(newtComponent co)
{
	struct CheckboxTree *ct = co->data;

	if (ct->flatList)
		free(ct->flatList);
	ct->flatCount = countItems(ct->itemlist, COUNT_EXPOSED);

	ct->flatList = malloc(sizeof(*ct->flatList) * (ct->flatCount + 1));
	ct->flatCount = 0;
	doBuildFlatList(ct, ct->itemlist);;
	ct->flatList[ct->flatCount] = NULL;
}

int newtCheckboxTreeAddItem(newtComponent co,
			    const char *text, const void *data,
			    int flags, int index, ...)
{
	va_list argList;
	int numIndexes;
	int *indexes;
	int i;

	va_start(argList, index);
	numIndexes = 0;
	i = index;
	while (i != NEWT_ARG_LAST) {
		numIndexes++;
		i = va_arg(argList, int);
	}

	va_end(argList);

	indexes = alloca(sizeof(*indexes) * (numIndexes + 1));
	va_start(argList, index);
	numIndexes = 0;
	i = index;
	va_start(argList, index);
	while (i != NEWT_ARG_LAST) {
		indexes[numIndexes++] = i;
		i = va_arg(argList, int);
	}
	va_end(argList);

	indexes[numIndexes++] = NEWT_ARG_LAST;

	return newtCheckboxTreeAddArray(co, text, data, flags, indexes);
}

static int doFindItemPath(struct items *items, void *data, int *path,
			  int *len)
{
	int where = 0;

	while (items) {
		if (items->data == data) {
			if (path)
				path[items->depth] = where;
			if (len)
				*len = items->depth + 1;
			return 1;
		}

		if (items->branch
		    && doFindItemPath(items->branch, data, path, len)) {
			if (path)
				path[items->depth] = where;
			return 1;
		}

		items = items->next;
		where++;
	}

	return 0;
}

int *newtCheckboxTreeFindItem(newtComponent co, void *data)
{
	int len;
	int *path;
	struct CheckboxTree *ct = co->data;

	if (!doFindItemPath(ct->itemlist, data, NULL, &len))
		return NULL;

	path = malloc(sizeof(*path) * (len + 1));
	doFindItemPath(ct->itemlist, data, path, NULL);
	path[len] = NEWT_ARG_LAST;

	return path;
}

int newtCheckboxTreeAddArray(newtComponent co,
			     const char *text, const void *data,
			     int flags, int *indexes)
{
	struct items *curList, *newNode, *item;
	struct items **listPtr = NULL;
	int i, index, numIndexes;
	struct CheckboxTree *ct = co->data;

	numIndexes = 0;
	while (indexes[numIndexes] != NEWT_ARG_LAST)
		numIndexes++;

	if (!ct->itemlist) {
		if (numIndexes > 1)
			return -1;
		ct->itemlist = malloc(sizeof(*ct->itemlist));
		item = ct->itemlist;
		item->prev = NULL;
		item->next = NULL;
	} else {
		curList = ct->itemlist;
		listPtr = &ct->itemlist;

		i = 0;
		index = indexes[i];
		while (i < numIndexes) {
			item = curList;

			if (index == NEWT_ARG_APPEND) {
				item = NULL;
			} else {
				while (index && item)
					item = item->next, index--;
			}

			i++;
			if (i < numIndexes) {
				curList = item->branch;
				listPtr = &item->branch;
				if (!curList && (i + 1 != numIndexes))
					return -1;

				index = indexes[i];
			}
		}

		if (!curList) {	/* create a new branch */
			item = malloc(sizeof(*curList->prev));
			item->next = item->prev = NULL;
			*listPtr = item;
		} else if (!item) {	/* append to end */
			item = curList;
			while (item->next)
				item = item->next;
			item->next = malloc(sizeof(*curList->prev));
			item->next->prev = item;
			item = item->next;
			item->next = NULL;

		} else {
			newNode = malloc(sizeof(*newNode));
			newNode->prev = item->prev;
			newNode->next = item;

			if (item->prev)
				item->prev->next = newNode;
			item->prev = newNode;
			item = newNode;
			if (!item->prev)
				*listPtr = item;
		}
	}

	item->text = strdup(text);
	item->data = data;
	if (flags & NEWT_FLAG_SELECTED) {
		item->selected = 1;
	} else {
		item->selected = 0;
	}
	item->flags = flags;
	item->branch = NULL;
	item->depth = numIndexes - 1;

	i = 4 + (3 * item->depth);

	if ((strlen(text) + i + ct->pad) > co->width) {
		co->width = strlen(text) + i + ct->pad;
	}
	item->parent = NULL;
	item->widget = NULL;
	return 0;
}

static struct items *findItem(struct items *items, const void *data)
{
	struct items *i;

	while (items) {
		if (items->data == data)
			return items;
		if (items->branch) {
			i = findItem(items->branch, data);
			if (i)
				return i;
		}

		items = items->next;
	}

	return NULL;
}

static void listSelected(struct items *items, int *num, void **list,
			 int seqindex)
{
	while (items) {
		if (
		    (seqindex ? items->selected ==
		     seqindex : items->selected) && !items->branch) {
			list[(*num)++] = items->data;
		}
		if (items->branch)
			listSelected(items->branch, num, list, seqindex);
		items = items->next;
	}
}

void **newtCheckboxTreeGetSelection(newtComponent co, int *numitems)
{
	return newtCheckboxTreeGetMultiSelection(co, numitems, 0);
}

void **newtCheckboxTreeGetMultiSelection(newtComponent co, int *numitems,
					 char seqnum)
{
	struct CheckboxTree *ct;
	void **retval;
	int seqindex = 0;

	if (!co || !numitems)
		return NULL;

	ct = co->data;

	if (seqnum) {
		while (ct->seq[seqindex] && (ct->seq[seqindex] != seqnum)) {
			seqindex++;
		}
	} else {
		seqindex = 0;
	}

	*numitems = countItems(ct->itemlist,
			       (seqindex ? seqindex : COUNT_SELECTED));

	if (!*numitems)
		return NULL;

	retval = malloc(*numitems * sizeof(void *));
	*numitems = 0;
	listSelected(ct->itemlist, numitems, retval, seqindex);

	return retval;
}

newtComponent newtCheckboxTree(int left, int top, int height, int flags)
{
	return newtCheckboxTreeMulti(left, top, height, NULL, flags);
}

newtComponent newtCheckboxTreeMulti(int left, int top, int height,
				    char *seq, int flags)
{
	newtComponent co;
	struct CheckboxTree *ct;

	co = malloc(sizeof(*co));
	ct = malloc(sizeof(struct CheckboxTree));
	co->data = ct;
	co->ops = &ctOps;
	co->takesFocus = 1;
	co->height = height;
	co->width = 0;
	co->isMapped = 0;
	ct->itemlist = NULL;
	ct->firstItem = NULL;
	ct->currItem = NULL;
	ct->flatList = NULL;
	ct->parent = NULL;
	ct->widget = NULL; /*gtk_tree_new();*/

	if (seq)
		ct->seq = strdup(seq);
	else
		ct->seq = strdup(" *");

	if (flags & NEWT_FLAG_SCROLL) {
		ct->sb = newtVerticalScrollbar(left, top, height,
					       COLORSET_LISTBOX,
					       COLORSET_ACTLISTBOX);
		ct->pad = 2;
	} else {
		ct->sb = NULL;
		ct->pad = 0;
	}

	return co;
}

static void ctMapped(newtComponent co, int isMapped)
{
	struct CheckboxTree *ct = co->data;

	co->isMapped = isMapped;
	if (ct->sb)
		ct->sb->ops->mapped(ct->sb, isMapped);
}

static void ctPlace(newtComponent co, int newLeft, int newTop)
{
	struct CheckboxTree *ct = co->data;

	co->top = newTop;
	co->left = newLeft;

	if (ct->sb)
		ct->sb->ops->place(ct->sb, co->left + co->width - 1,
				   co->top);
}

int ctSetItem(newtComponent co, struct items *item,
	      enum newtFlagsSense sense)
{
	struct CheckboxTree *ct = co->data;
	struct items *currItem;
	struct items *firstItem;

	if (!item)
		return 1;
	switch (sense) {
	case NEWT_FLAGS_RESET:
		item->selected = 0;
		break;
	case NEWT_FLAGS_SET:
		item->selected = 1;
		break;
	case NEWT_FLAGS_TOGGLE:
		if (item->branch) {
			item->selected = !item->selected;
		} else {
			item->selected++;
			if (item->selected == strlen(ct->seq))
				item->selected = 0;
		}
		break;
	}

	if (item->branch) {
		currItem = *ct->currItem;
		firstItem = *ct->firstItem;

		buildFlatList(co);

		ct->currItem = ct->flatList;
		while (*ct->currItem != currItem)
			ct->currItem++;

		ct->firstItem = ct->flatList;
		while (*ct->firstItem != firstItem)
			ct->firstItem++;
	}

	return 0;
}


static gint cb_itemsignal(GtkWidget * widget, GdkEventKey * event)
{
	gchar *name;
	GtkLabel *label;
	char tmp[100];
	struct items *item;
	struct CheckboxTree *ct;

	item = gtk_object_get_data(GTK_OBJECT(widget), "item");
	ct = item->parentObj->data;
	if (event && event->type == GDK_KEY_PRESS) {
		if (event->keyval != 32)
			return 0;
	}
	if (item->selected == 255) {
		item->selected = 0;
		gtk_tree_unselect_child(GTK_TREE(ct->widget),
					item->widget);
	} else {
		gtk_tree_select_child(GTK_TREE(ct->widget), item->widget);
	}
	if (item->branch)
		return 0;
	ctSetItem(item->parentObj, item, NEWT_FLAGS_TOGGLE);
	label = GTK_LABEL(GTK_BIN(widget)->child);
	sprintf(tmp, "[%c] %s", ct->seq[item->selected], item->text);
	gtk_label_get(label, &name);
	gtk_label_set_text(label, tmp);

	return 1;
}

static void ctDraw(newtComponent co)
{
	struct CheckboxTree *ct = co->data;
	struct items **item;
	GtkWidget *scrolled_win;
	GtkWidget *currentNode[20];
	int i;
	gchar *txt[2];
	gchar buf1[60];
	gchar buf2[60];
	txt[0] = buf1;
	txt[1] = buf2;

	if (!co->isMapped)
		return;
	if (!ct->widget || !GTK_IS_WIDGET(ct->widget)) {
		ct->widget = gtk_tree_new();
		ct->parent = gnewt->currentParent;
		gtk_widget_show(ct->widget);
		scrolled_win = gtk_scrolled_window_new(NULL, NULL);
		gtk_widget_show(scrolled_win);
		gtk_widget_set_usize(scrolled_win,
				     gnewt->FontSizeW * co->width,
				     gnewt->FontSizeH * co->height);
		gtk_fixed_put(GTK_FIXED(ct->parent), scrolled_win,
			      gnewt->FontSizeW * co->left, 
			      gnewt->FontSizeH * co->top);

		if (ct->sb || co->width > 20) {
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
		gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW
						      (scrolled_win),
						      ct->widget);

		gtk_container_set_focus_vadjustment(GTK_CONTAINER
					(ct->widget),
					gtk_scrolled_window_get_vadjustment
					(GTK_SCROLLED_WINDOW
					(scrolled_win)));

		gtk_tree_set_selection_mode(GTK_TREE(ct->widget),
					    GTK_SELECTION_MULTIPLE);
	}

	if (!ct->firstItem) {
		buildFlatList(co);
		ct->firstItem = ct->currItem = ct->flatList;
	}

	item = ct->firstItem;

	i = 0;
	currentNode[0] = ct->widget;
	while (*item) {
		char tmp[100];
		sprintf(tmp, "[%c] %s", ct->seq[0], (*item)->text);

		if ((*item)->widget && GTK_IS_WIDGET((*item)->widget)) {
			gtk_widget_destroy((*item)->widget);
		}
		(*item)->num = i;
		(*item)->parentObj = co;
		if ((*item)->branch) {
			(*item)->widget = 
				gtk_tree_item_new_with_label((*item)->text);

			gtk_tree_append(GTK_TREE
					(currentNode[(*item)->depth]),
					(*item)->widget);

			currentNode[(*item)->depth + 1] = gtk_tree_new();

			gtk_tree_item_set_subtree(GTK_TREE_ITEM
						  ((*item)->widget),
						  currentNode[(*item)->
							      depth + 1]);

		} else {
			(*item)->widget = gtk_tree_item_new_with_label(tmp);

			gtk_tree_append(GTK_TREE
					(currentNode[(*item)->depth]),
					(*item)->widget);
		}
		if (!(*item)->branch) {
			gtk_signal_connect(GTK_OBJECT((*item)->widget),
					   "button_press_event",
					   GTK_SIGNAL_FUNC(cb_itemsignal),
					   NULL);
			gtk_signal_connect(GTK_OBJECT((*item)->widget),
					   "key_press_event",
					   GTK_SIGNAL_FUNC(cb_itemsignal),
					   NULL);
			gtk_object_set_data(GTK_OBJECT((*item)->widget),
					    "item", (*item));
		}
		gtk_widget_show((*item)->widget);
		item++;
		i++;
	}
	item = ct->firstItem;
	while (*item) {
		if ((*item)->selected) {
			(*item)->selected = 255;
			cb_itemsignal((*item)->widget, NULL);
		}
		item++;
	}
	gnewt->currentParent = ct->parent;
	/*newtRefresh();*/
}

static void ctDestroy(newtComponent co)
{
	struct CheckboxTree *ct = co->data;
	struct items *item, *nextitem;

	nextitem = item = ct->itemlist;
	while (item != NULL) {
		nextitem = item->next;
		free(item->text);
		free(item);
		item = nextitem;
	}

	free(ct->seq);
	free(ct);
	free(co);
}

struct eventResult ctEvent(newtComponent co, struct event ev)
{
	struct CheckboxTree *ct = co->data;
	struct eventResult er;
	struct items **listEnd, **lastItem;

	er.result = ER_IGNORED;

	if (ev.when == EV_EARLY || ev.when == EV_LATE) {
		return er;
	}

	switch (ev.event) {
	case EV_KEYPRESS:
		switch (ev.u.key) {
		case ' ':
		case NEWT_KEY_ENTER:
			if (*ct->currItem) {
				ctSetItem(co, *ct->currItem,
					  NEWT_FLAGS_TOGGLE);
				ctDraw(co);
				er.result = ER_SWALLOWED;
			}
			break;
		case NEWT_KEY_DOWN:
			if ((ct->currItem - ct->flatList + 1) <
			    ct->flatCount) {
				ct->currItem++;

				er.result = ER_SWALLOWED;

				if (ct->currItem - ct->firstItem >=
				    co->height) ct->firstItem++;

				ctDraw(co);
			}
			break;
		case NEWT_KEY_UP:
			if (ct->currItem != ct->flatList) {
				ct->currItem--;
				er.result = ER_SWALLOWED;

				if (ct->currItem < ct->firstItem)
					ct->firstItem = ct->currItem;

				ctDraw(co);
			}
			break;
		case NEWT_KEY_PGUP:
			if (ct->firstItem - co->height < ct->flatList) {
				ct->firstItem = ct->currItem =
				    ct->flatList;
			} else {
				ct->currItem -= co->height;
				ct->firstItem -= co->height;
			}

			ctDraw(co);
			er.result = ER_SWALLOWED;
			break;
		case NEWT_KEY_PGDN:
			listEnd = ct->flatList + ct->flatCount - 1;
			lastItem = ct->firstItem + co->height - 1;

			if (lastItem + co->height > listEnd) {
				ct->firstItem = listEnd - co->height + 1;
				ct->currItem = listEnd;
			} else {
				ct->currItem += co->height;
				ct->firstItem += co->height;
			}

			ctDraw(co);
			er.result = ER_SWALLOWED;
			break;
		}
	case EV_FOCUS:
		ctDraw(co);
		er.result = ER_SWALLOWED;
		break;

	case EV_UNFOCUS:
		ctDraw(co);
		er.result = ER_SWALLOWED;
		break;
	default:
		break;
	}

	return er;
}
