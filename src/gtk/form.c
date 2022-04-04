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

#include <stdarg.h>
#include <stdlib.h>
#include <sys/select.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>

#include "newt.h"
#include "newt_pr.h"
#include "gnewt.h"
#include "gnewt_pr.h"

/****************************************************************************
    These forms handle vertical scrolling of components with a height of 1 
   
    Horizontal scrolling won't work, and scrolling large widgets will fail
    miserably. It shouldn't be too hard to fix either of those if anyone
    cares to. I only use scrolling for listboxes and text boxes though so
    I didn't bother.
*****************************************************************************/

extern struct Gnewt *gnewt;
static GtkStyle *formStyle;

struct element {
	int top, left;		/* Actual, not virtual. These are translated */
	newtComponent co;	/* into actual through vertOffset */
};

struct fdInfo {
	int fd;
	int flags;
};

struct form {
	int numCompsAlloced;
	struct element *elements;
	int numComps;
	int currComp;
	int fixedHeight;
	int flags;
	int vertOffset;
	newtComponent vertBar, exitComp;
	const char *help;
	int numRows;
	int *hotKeys;
	int numHotKeys;
	int background;
	int beenSet;
	int numFds;
	struct fdInfo *fds;
	int maxFd;
	GtkWidget *widget;
	GtkWidget *parent;
};


static void gotoComponent(struct form *form, int newComp);
static struct eventResult formEvent(newtComponent co, struct event ev);
static struct eventResult sendEvent(newtComponent comp, struct event ev);
static void formPlace(newtComponent co, int left, int top);

/* this isn't static as grid.c tests against it to find forms */
struct componentOps formOps = {
	newtDrawForm,
	formEvent,
	newtFormDestroy,
	formPlace,
	newtDefaultMappedHandler,
};

static inline int componentFits(newtComponent co, int compNum)
{
	struct form *form = co->data;
	struct element *el = form->elements + compNum;

	if ((co->top + form->vertOffset) > el->top)
		return 0;
	if ((co->top + form->vertOffset + co->height) <
	    (el->top + el->co->height)) return 0;

	return 1;
}

newtComponent newtForm(newtComponent vertBar, const char *help, int flags)
{
	newtComponent co;
	struct form *form;

	co = malloc(sizeof(*co));
	form = malloc(sizeof(*form));
	co->data = form;
	co->width = 0;
	co->height = 0;
	co->top = -1;
	co->left = -1;
	co->isMapped = 0;

	co->takesFocus = 0;	/* we may have 0 components */
	co->ops = &formOps;
	form->exitComp = co;

	form->help = help;
	form->flags = flags;
	form->numCompsAlloced = 5;
	form->numComps = 0;
	form->currComp = -1;
	form->vertOffset = 0;
	form->fixedHeight = 0;
	form->numRows = 0;
	form->numFds = 0;
	form->maxFd = 0;
	form->fds = NULL;
	form->beenSet = 0;
	form->elements =
	    malloc(sizeof(*(form->elements)) * form->numCompsAlloced);
	form->widget = NULL;
	form->parent = NULL;

	form->background = COLORSET_WINDOW;
	form->hotKeys = malloc(sizeof(int));
	form->numHotKeys = 0;
	if (!(form->flags & NEWT_FLAG_NOF12)) {
		newtFormAddHotKey(co, NEWT_KEY_F12);
	}

	if (vertBar)
		form->vertBar = vertBar;
	else
		form->vertBar = NULL;

	return co;
}

newtComponent newtFormGetCurrent(newtComponent co)
{
	struct form *form = co->data;

	return form->elements[form->currComp].co;
}

void newtFormSetCurrent(newtComponent co, newtComponent subco)
{
	struct form *form = co->data;
	int i, new;

	for (i = 0; i < form->numComps; i++) {
		if (form->elements[i].co == subco)
			break;
	}

	if (form->elements[i].co != subco)
		return;
	new = i;

	if (co->isMapped && !componentFits(co, new)) {
		gotoComponent(form, -1);
		form->vertOffset = form->elements[new].top - co->top - 1;
		if (form->vertOffset > (form->numRows - co->height))
			form->vertOffset = form->numRows - co->height;
	}

	gotoComponent(form, new);
}

void newtFormSetHeight(newtComponent co, int height)
{
	struct form *form = co->data;

	form->fixedHeight = 1;
	co->height = height;
}

void newtFormSetWidth(newtComponent co, int width)
{
	co->width = width;
}

void newtFormAddComponent(newtComponent co, newtComponent newco)
{
	struct form *form = co->data;

	co->takesFocus = 1;

	if (form->numCompsAlloced == form->numComps) {
		form->numCompsAlloced += 5;
		form->elements = realloc(form->elements,
					 sizeof(*(form->elements)) *
					 form->numCompsAlloced);
	}

	/* we grab real values for these a bit later */
	form->elements[form->numComps].left = -2;
	form->elements[form->numComps].top = -2;
	form->elements[form->numComps].co = newco;

	if (newco->takesFocus && form->currComp == -1)
		form->currComp = form->numComps;

	form->numComps++;
}

void newtFormAddComponents(newtComponent co, ...)
{
	va_list ap;
	newtComponent subco;

	va_start(ap, co);

	while ((subco = va_arg(ap, newtComponent)))
		newtFormAddComponent(co, subco);

	va_end(ap);
}

static void formPlace(newtComponent co, int left, int top)
{
	struct form *form = co->data;
	int vertDelta, horizDelta;
	struct element *el;
	int i;

	newtFormSetSize(co);

	vertDelta = top - co->top;
	horizDelta = left - co->left;
	co->top = top;
	co->left = left;

	for (i = 0, el = form->elements; i < form->numComps; i++, el++) {
		el->co->top += vertDelta;
		el->top += vertDelta;
		el->co->left += horizDelta;
		el->left += horizDelta;
	}
}

void newtDrawForm(newtComponent co)
{
	struct form *form = co->data;
	struct element *el;
	GtkWidget *scrolled_win;
	GtkStyle *style;
	int i, top, left;

	newtFormSetSize(co);
	gnewt->formLevel++;
	top = co->top;
	left = co->left;

	if (!form->parent) {
		form->parent = gnewt->currentParent;
		gtk_widget_show(gnewt->currentParent);
		if (gnewt->formLevel == 1) {
			form->widget = gnewt->currentParent;
		} else {
			form->widget = gtk_fixed_new();
			scrolled_win = gtk_scrolled_window_new(NULL, NULL);
			style = gtk_style_copy(gnewt->gRootStyle);
			if (gnewt->useNewtColor) {
				style->bg[0] =
			    		getColorByName(
						gnewt->globalColors->windowBg);
				style->fg[0] =
			    		getColorByName(
						gnewt->globalColors->titleFg);
			}
			gtk_widget_set_style(scrolled_win, style);

			gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW
						       (scrolled_win),
						       GTK_POLICY_NEVER,
						       GTK_POLICY_AUTOMATIC);
			gtk_widget_show(scrolled_win);
			gtk_widget_set_usize(scrolled_win,
					     gnewt->FontSizeW * co->width,
					     gnewt->FontSizeH * co->height);
			gtk_fixed_put(GTK_FIXED(form->parent),
				      scrolled_win, gnewt->FontSizeW * co->left,
				      gnewt->FontSizeH * co->top);

			style = gtk_style_copy(gnewt->gRootStyle);
			if (gnewt->useNewtColor) {
				style->bg[0] =
			    		getColorByName(gnewt->colorTable
					   [(form->background - 2) * 2 + 1]);
				style->fg[0] =
			    		getColorByName(
						gnewt->globalColors->rootFg);
			}
			gtk_widget_set_style(form->widget, style);
			gtk_scrolled_window_add_with_viewport(
			    	GTK_SCROLLED_WINDOW(scrolled_win),
			     	form->widget);
			GTK_SCROLLED_WINDOW_CLASS(
				GTK_OBJECT(scrolled_win)->klass)->
			    		scrollbar_spacing = -2;
			GTK_CONTAINER(scrolled_win)->border_width = -2;
		}
		gtk_widget_show(form->widget);
	}
	for (i = 0, el = form->elements; i < form->numComps; i++, el++) {
		/* the scrollbar *always* fits somewhere */
		if (el->co == form->vertBar) {
			gnewt->currentParent = form->widget;
			el->co->ops->mapped(el->co, 1);
			el->co->ops->draw(el->co);
		} else {
			if (gnewt->formLevel > 1) {
				el->co->top = el->top - co->top;
				el->co->left = el->left - co->left;
			}
			gnewt->currentParent = form->widget;
			el->co->ops->mapped(el->co, 1);
			el->co->ops->draw(el->co);
		}
	}
	gnewt->formLevel--;
	gnewt->currentParent = form->parent;
	/*newtRefresh();*/
}

static struct eventResult formEvent(newtComponent co, struct event ev)
{
	struct form *form = co->data;
	newtComponent subco = form->elements[form->currComp].co;
	int new, wrap = 0;
	struct eventResult er;
	int dir = 0, page = 0;
	int i, num;

	er.result = ER_IGNORED;
	if (!form->numComps)
		return er;

	subco = form->elements[form->currComp].co;

	switch (ev.when) {
	case EV_EARLY:
		if (ev.event == EV_KEYPRESS) {
			if (ev.u.key == NEWT_KEY_TAB) {
				er.result = ER_SWALLOWED;
				dir = 1;
				wrap = 1;
			} else if (ev.u.key == NEWT_KEY_UNTAB) {
				er.result = ER_SWALLOWED;
				dir = -1;
				wrap = 1;
			}
		}

		if (form->numComps) {
			i = form->currComp;
			num = 0;
			while (er.result == ER_IGNORED
			       && num != form->numComps) {
				er =
				    form->elements[i].co->ops->event(form->
								     elements
								     [i].
								     co,
								     ev);
				num++;
				i++;
				if (i == form->numComps)
					i = 0;
			}
		}
		break;

	case EV_NORMAL:
		er = subco->ops->event(subco, ev);
		switch (er.result) {
		case ER_NEXTCOMP:
			er.result = ER_SWALLOWED;
			dir = 1;
			break;

		case ER_EXITFORM:
			form->exitComp = subco;
			break;

		default:
			break;
		}
		break;

	case EV_LATE:
		er = subco->ops->event(subco, ev);

		if (er.result == ER_IGNORED) {
			switch (ev.u.key) {
			case NEWT_KEY_UP:
			case NEWT_KEY_LEFT:
			case NEWT_KEY_BKSPC:
				er.result = ER_SWALLOWED;
				dir = -1;
				break;

			case NEWT_KEY_DOWN:
			case NEWT_KEY_RIGHT:
				er.result = ER_SWALLOWED;
				dir = 1;
				break;

			case NEWT_KEY_PGUP:
				er.result = ER_SWALLOWED;
				dir = -1;
				page = 1;
				break;

			case NEWT_KEY_PGDN:
				er.result = ER_SWALLOWED;
				dir = 1;
				page = 1;
				break;
			}
		}
	}

	if (dir) {
		new = form->currComp;

		if (page) {
			new += dir * co->height;
			if (new < 0)
				new = 0;
			else if (new >= form->numComps)
				new = (form->numComps - 1);

			while (!form->elements[new].co->takesFocus)
				new = new - dir;
		} else {
			do {
				new += dir;
				if (wrap) {
					if (new < 0)
						new = form->numComps - 1;
					else if (new >= form->numComps)
						new = 0;
				} else if (new < 0
					   || new >=
					   form->numComps) return er;
			} while (!form->elements[new].co->takesFocus);
		}

		/* make sure this component is visible */
		if (!componentFits(co, new)) {
			gotoComponent(form, -1);

			if (dir < 0) {
				/* make the new component the first one */
				form->vertOffset =
				    form->elements[new].top - co->top;
			} else {
				/* make the new component the last one */
				form->vertOffset =
				    (form->elements[new].top +
				     form->elements[new].co->height) -
				    (co->top + co->height);
			}

			if (form->vertOffset < 0)
				form->vertOffset = 0;
			if (form->vertOffset >
			    (form->numRows -
			     co->height)) form->vertOffset =
				    form->numRows - co->height;

			newtDrawForm(co);
		}

		gotoComponent(form, new);
		er.result = ER_SWALLOWED;
	}

	return er;
}

/* this also destroys all of the components on the form */
void newtFormDestroy(newtComponent co)
{
	newtComponent subco;
	struct form *form = co->data;
	int i;

	/* first, destroy all of the components */
	for (i = 0; i < form->numComps; i++) {
		subco = form->elements[i].co;
		if (subco->ops->destroy) {
			subco->ops->destroy(subco);
		} else {
			if (subco->data)
				free(subco->data);
			free(subco);
		}
	}
	free(form->elements);
	free(form);
	free(co);
}

newtComponent newtRunForm(newtComponent co)
{
	struct newtExitStruct es;

	newtFormRun(co, &es);
	if (es.reason == NEWT_EXIT_HOTKEY) {
		if (es.u.key == NEWT_KEY_F12) {
			es.reason = NEWT_EXIT_COMPONENT;
			es.u.co = co;
		} else {
			return NULL;
		}
	}

	return es.u.co;
}

void newtFormAddHotKey(newtComponent co, int key)
{
	struct form *form = co->data;

	form->numHotKeys++;
	form->hotKeys =
	    realloc(form->hotKeys, sizeof(int) * form->numHotKeys);
	form->hotKeys[form->numHotKeys - 1] = key;
}

void newtFormSetSize(newtComponent co)
{
	struct form *form = co->data;
	int delta, i;
	struct element *el;

	if (form->beenSet)
		return;

	form->beenSet = 1;

	if (!form->numComps)
		return;

	co->width = 0;
	if (!form->fixedHeight)
		co->height = 0;

	co->top = form->elements[0].co->top;
	co->left = form->elements[0].co->left;
	for (i = 0, el = form->elements; i < form->numComps; i++, el++) {
		if (el->co->ops == &formOps)
			newtFormSetSize(el->co);

		el->left = el->co->left;
		el->top = el->co->top;

		if (co->left > el->co->left) {
			delta = co->left - el->co->left;
			co->left -= delta;
			co->width += delta;
		}

		if (co->top > el->co->top) {
			delta = co->top - el->co->top;
			co->top -= delta;
			if (!form->fixedHeight)
				co->height += delta;
		}

		if ((co->left + co->width) <
		    (el->co->left + el->co->width)) co->width =
			    (el->co->left + el->co->width) - co->left;

		if (!form->fixedHeight) {
			if ((co->top + co->height) <
			    (el->co->top + el->co->height)) co->height =
				    (el->co->top + el->co->height) -
				    co->top;
		}

		if ((el->co->top + el->co->height - co->top) >
		    form->numRows) {
			form->numRows =
			    el->co->top + el->co->height - co->top;
		}
	}
}

void newtFormRun(newtComponent co, struct newtExitStruct *es)
{
	struct form *form = co->data;
	int i;
	int done = 0;

	gnewt->currentExitComp = malloc(sizeof(*gnewt->currentExitComp));

	newtFormSetSize(co);
	/* draw all of the components */
	newtDrawForm(co);
	if (form->currComp == -1) {
		gotoComponent(form, 0);
	} else
		gotoComponent(form, form->currComp);

	while (!done) {
		gtk_main();

		for (i = 0; i < form->numHotKeys; i++) {
			if (form->hotKeys[i] == gnewt->pressedKey) {
				es->reason = NEWT_EXIT_HOTKEY;
				es->u.key = gnewt->pressedKey;
				done = 1;
				break;
			}
		}
		if (!done && (gnewt->pressedKey == 0)) {
			int i = 0;
			es->reason = NEWT_EXIT_COMPONENT;
			es->u.co = gnewt->currentExitComp;
			done = 1;
			while (form->elements[i].co != gnewt->currentExitComp &&
			       i < form->numComps)
				i++;
			form->currComp = i;
		}
		gnewt->pressedKey = 0;
		newtRefresh();
	}
}

static struct eventResult sendEvent(newtComponent co, struct event ev)
{
	struct eventResult er;

	ev.when = EV_EARLY;
	er = co->ops->event(co, ev);

	if (er.result == ER_IGNORED) {
		ev.when = EV_NORMAL;
		er = co->ops->event(co, ev);
	}

	if (er.result == ER_IGNORED) {
		ev.when = EV_LATE;
		er = co->ops->event(co, ev);
	}

	return er;
}

static void gotoComponent(struct form *form, int newComp)
{
	struct event ev;

	if (0)
		sendEvent(form->elements[form->currComp].co, ev);
	form->currComp = newComp;
}

void newtComponentAddCallback(newtComponent co, newtCallback f, void *data)
{
	co->callback = f;
	co->callbackData = data;
}

void newtComponentTakesFocus(newtComponent co, int val)
{
	co->takesFocus = val;
}

void newtFormSetBackground(newtComponent co, int color)
{
	struct form *form = co->data;

	form->background = color;
}

void newtFormWatchFd(newtComponent co, int fd, int fdFlags)
{
	struct form *form = co->data;

	form->fds =
	    realloc(form->fds, (form->numFds + 1) * sizeof(*form->fds));
	form->fds[form->numFds].fd = fd;
	form->fds[form->numFds++].flags = fdFlags;
	if (form->maxFd < fd)
		form->maxFd = fd;
}
