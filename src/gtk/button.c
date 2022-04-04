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
 *	 O'ksi'D <nickasil@linuxave.net>
 *       Jean-Marc Lienher
 *       Rue de la Cheminee 1
 *       CH-2065 Savagnier
 *       Switzerland 
 */

#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>

#include "newt.h"
#include "newt_pr.h"
#include "gnewt.h"
#include "gnewt_pr.h"

struct button {
	char *text;
	int compact;
	GtkWidget *widget;
	GtkWidget *parent;
	GdkPixmap *pixmap;
	GdkBitmap *mask;
};

/* gtk specific */
extern struct Gnewt *gnewt;
/* end gtk */

static void buttonDrawIt(newtComponent co, int active, int pushed);

static void buttonDraw(newtComponent c);
static void buttonDestroy(newtComponent co);
static struct eventResult buttonEvent(newtComponent c, struct event ev);
static void buttonPlace(newtComponent co, int newLeft, int newTop);

static struct componentOps buttonOps = {
	buttonDraw,
	buttonEvent,
	buttonDestroy,
	buttonPlace,
	newtDefaultMappedHandler,
};

static newtComponent createButton(int left, int row, const char *text,
				  int compact)
{
	newtComponent co;
	struct button *bu;
	co = malloc(sizeof(*co));
	bu = malloc(sizeof(struct button));
	co->data = bu;

	bu->text = strdup(text);
	bu->compact = compact;
	bu->widget = NULL; /*gtk_button_new();*/
	bu->parent = NULL;
	bu->pixmap = gnewt->buttonPixmap;
	bu->mask = gnewt->buttonMask;
	co->ops = &buttonOps;

	if (bu->compact) {
		co->height = 1;
		co->width = strlen(text) + 3;
	} else {
		co->height = 4;
		co->width = strlen(text) + 5;
	}

	co->top = row;
	co->left = left;
	co->takesFocus = 1;
	co->callback = NULL;
	co->isMapped = 0;

	return co;
}

newtComponent newtCompactButton(int left, int row, const char *text)
{
	/* dialogboxes.c workaround */
	if (left == -5)
		left = 35;
	if (left == -6)
		left = 20;
	if (left == -3)
		left = 49;
	if (row < -1)
		row = 22 + row;

	return createButton(left, row, text, 1);
}

newtComponent newtButton(int left, int row, const char *text)
{
	/* dialogboxes.c workaround */
	if (left == -5)
		left = 35;
	if (left == -6)
		left = 20;
	if (left == -3)
		left = 49;
	if (row < -1)
		row = 22 + row;

	return createButton(left, row, text, 0);
}

static void buttonDestroy(newtComponent co)
{
	struct button *bu = co->data;

	free(bu->text);
	free(bu);
	free(co);
}

static void buttonPlace(newtComponent co, int newLeft, int newTop)
{
	co->top = newTop;
	co->left = newLeft;

}

static void buttonDraw(newtComponent co)
{
	buttonDrawIt(co, 0, 0);
}

static void buttonCb(newtComponent co)
{
	gnewt->currentExitComp = co;
	gtk_main_quit();
}

static void buttonFocusCb(newtComponent co)
{
	if (co->callback)
		co->callback(co, co->callbackData);
}

static void buttonDestroyed(newtComponent co)
{
	struct button *bu = co->data;
	bu->widget = NULL;
}

static void buttonDrawIt(newtComponent co, int active, int pushed)
{
	struct button *bu = co->data;
	GtkStyle *style;

	if (!co->isMapped)
		return;
	if (!bu->widget || !GTK_IS_WIDGET(bu->widget)) {
		GtkWidget *label, *box;
		bu->widget = gtk_button_new();
		box = gtk_hbox_new(FALSE, 0);
		gtk_container_set_border_width(GTK_CONTAINER(box), 0);
		gtk_widget_show(box);
		bu->parent = gnewt->currentParent;
		label = gtk_label_new(bu->text);
		gtk_widget_show(label);
		style = gtk_style_copy(gnewt->gRootStyle);
		gtk_widget_set_style(label, style);
		if (bu->compact) {
			if (gnewt->useNewtColor) {
				style->bg[0] =
			    	     getColorByName(
					gnewt->globalColors->compactButtonBg);
				style->fg[0] =
			    	     getColorByName(
					gnewt->globalColors->compactButtonFg);
			}
			gtk_widget_set_usize(bu->widget,
					     gnewt->FontSizeW * (co->width - 1),
					     gnewt->FontSizeH * co->height);
		} else {
			GtkWidget *pixmapwid;
			if (gnewt->useNewtColor) {
				style->bg[0] =
			    	    getColorByName(
					gnewt->globalColors->buttonBg);
				style->fg[0] =
			    	    getColorByName(
					gnewt->globalColors->buttonFg);
				style->bg[1] =
			    	    getColorByName(
					gnewt->globalColors->actButtonBg);
				style->bg[2] =
			    	    getColorByName(
					gnewt->globalColors->actButtonBg);
				style->fg[1] =
			    	    getColorByName(
					gnewt->globalColors->actButtonFg);
				style->fg[2] =
			    	    getColorByName(
					gnewt->globalColors->actButtonFg);
				style->fg[2] =
			    		getColorByName(
					    gnewt->globalColors->actButtonFg);
			}
			if (bu->pixmap) {
				pixmapwid = gtk_pixmap_new(
						bu->pixmap, 
						bu->mask);
				gtk_widget_show(pixmapwid);
				gtk_box_pack_start(GTK_BOX(box),
					   pixmapwid, FALSE, FALSE, 0);
			}
			gtk_widget_set_usize(bu->widget,
					gnewt->FontSizeW * (co->width),
					gnewt->FontSizeH * co->height / 2.5);
			
		}
		gtk_box_pack_start(GTK_BOX(box), label, TRUE, FALSE, 0);
		gtk_container_add(GTK_CONTAINER(bu->widget), box);
		gtk_container_set_border_width(GTK_CONTAINER(bu->widget),
					       0);
		gtk_widget_set_style(bu->widget, style);
		gtk_widget_set_style(GTK_BIN(bu->widget)->child, style);
		if (bu->compact) {
			gtk_fixed_put(GTK_FIXED(bu->parent), bu->widget,
				      gnewt->FontSizeW * (co->left + 1),
				      gnewt->FontSizeH * co->top);
		} else {
			gtk_fixed_put(GTK_FIXED(bu->parent), bu->widget,
				      gnewt->FontSizeW * (co->left),
				      gnewt->FontSizeH * (co->top + 1));
		}
		gtk_signal_connect_object(GTK_OBJECT(bu->widget),
					  "clicked",
					  GTK_SIGNAL_FUNC(buttonCb),
					  (GtkObject *) co);
		gtk_signal_connect_object(GTK_OBJECT(bu->widget),
					  "focus_in_event",
					  GTK_SIGNAL_FUNC(buttonFocusCb),
					  (GtkObject *) co);
		gtk_signal_connect_object(GTK_OBJECT(bu->widget),
					  "enter_notify_event",
					  GTK_SIGNAL_FUNC(buttonFocusCb),
					  (GtkObject *) co);
/*
		gtk_signal_connect_object(GTK_OBJECT(bu->widget), "destroy",
	                         	GTK_SIGNAL_FUNC(buttonDestroyed),
		                        (GtkObject *) co);
*/
	}
	gtk_widget_show(bu->widget);
	/*newtRefresh();*/
}

static struct eventResult buttonEvent(newtComponent co, struct event ev)
{
	struct eventResult er;
	struct button *bu = co->data;

	if (ev.when == EV_NORMAL) {
		switch (ev.event) {
		case EV_FOCUS:
			er.result = ER_SWALLOWED;
			if (co->callback)
				co->callback(co, co->callbackData);
			break;

		case EV_UNFOCUS:
			er.result = ER_SWALLOWED;
			break;

		case EV_KEYPRESS:
			if (ev.u.key == ' ' || ev.u.key == '\r') {
				er.result = ER_EXITFORM;
			} else
				er.result = ER_IGNORED;
			break;
		case EV_MOUSE:
			if (ev.u.mouse.type == MOUSE_BUTTON_DOWN &&
			    co->top <= ev.u.mouse.y &&
			    co->top + co->height - !bu->compact >
			    ev.u.mouse.y && co->left <= ev.u.mouse.x
			    && co->left + co->width - !bu->compact >
			    ev.u.mouse.x) {
				er.result = ER_EXITFORM;
			}
			break;

		}
	} else
		er.result = ER_IGNORED;

	return er;
}
