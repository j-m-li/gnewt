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
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <stdio.h>

#include "newt.h"
#include "newt_pr.h"
#include "gnewt.h"
#include "gnewt_pr.h"

enum type { CHECK, RADIO };

struct checkbox {
	char *text;
	char *seq;
	char *result;
	newtComponent prevButton, lastButton;
	enum type type;
	char value;
	int active, inactive;
	const void *data;
	int flags;
	int hasFocus;
	GtkWidget *widget;
	GtkWidget *menu;
	GtkWidget *parent;
	GtkWidget *back_ground;
};

extern struct Gnewt *gnewt;

static void cbDrawIt(newtComponent c, int active);
static void makeActive(newtComponent co);

static void cbDraw(newtComponent c);
static void cbDestroy(newtComponent co);
struct eventResult cbEvent(newtComponent co, struct event ev);

static struct componentOps cbOps = {
	cbDraw,
	cbEvent,
	cbDestroy,
	newtDefaultPlaceHandler,
	newtDefaultMappedHandler,
};

newtComponent newtRadiobutton(int left, int top, const char *text,
			      int isDefault, newtComponent prevButton)
{
	newtComponent co;
	newtComponent curr;
	struct checkbox *rb, *prevRb;
	char initialValue;
	GtkStyle *style;

	if (isDefault)
		initialValue = '*';
	else
		initialValue = ' ';

	co = newtCheckbox(left, top, text, initialValue, " *", NULL);
	rb = co->data;
	rb->type = RADIO;

	if (prevButton) {
		prevRb = prevButton->data;
		rb->widget =
		    	gtk_radio_button_new_with_label(
				gtk_radio_button_group(
					GTK_RADIO_BUTTON(prevRb->widget)),
				rb->text);

		style = gtk_widget_get_style(prevRb->widget);
		rb->back_ground = gtk_event_box_new();
		gtk_widget_set_style(rb->back_ground, style);
		gtk_widget_set_style(rb->widget, style);
		gtk_widget_set_style(GTK_BIN(rb->widget)->child, style);
	} else {
		rb->widget =
		    	gtk_radio_button_new_with_label(NULL, rb->text);
		style = gtk_style_copy(gnewt->gRootStyle);
		if (gnewt->useNewtColor) {
			style->bg[0] = getColorByName(
					gnewt->globalColors->checkboxBg);
			style->fg[0] = getColorByName(
					gnewt->globalColors->checkboxFg);
			style->bg[1] = getColorByName(
					gnewt->globalColors->checkboxFg);
			style->fg[1] = getColorByName(
					gnewt->globalColors->checkboxFg);
		}
		rb->back_ground = gtk_event_box_new();
		gtk_widget_set_style(rb->back_ground, style);
		gtk_widget_set_style(rb->widget, style);
		gtk_widget_set_style(GTK_BIN(rb->widget)->child, style);

	}
	rb->prevButton = prevButton;

	for (curr = co; curr; curr = rb->prevButton) {
		rb = curr->data;
		rb->lastButton = co;
	}


	return co;
}

newtComponent newtRadioGetCurrent(newtComponent setMember)
{
	struct checkbox *rb = setMember->data;

	setMember = rb->lastButton;
	rb = setMember->data;

	while (rb && rb->value != '*') {
		setMember = rb->prevButton;
		rb = setMember->data;
	}

	return setMember;
}

char newtCheckboxGetValue(newtComponent co)
{
	struct checkbox *cb = co->data;

	return cb->value;
}

newtComponent newtCheckbox(int left, int top, const char *text,
			   char defValue, const char *seq, char *result)
{
	newtComponent co;
	struct checkbox *cb;

	if (!seq)
		seq = " *";

	co = malloc(sizeof(*co));
	cb = malloc(sizeof(struct checkbox));
	co->data = cb;
	cb->flags = 0;
	if (result)
		cb->result = result;
	else
		cb->result = &cb->value;

	cb->text = strdup(text);
	cb->seq = strdup(seq);
	cb->type = CHECK;
	cb->hasFocus = 0;
	cb->inactive = COLORSET_CHECKBOX;
	cb->active = COLORSET_ACTCHECKBOX;
	defValue ? (*cb->result = defValue) : (*cb->result = cb->seq[0]);

	cb->parent = NULL;
	cb->menu = NULL;

	if (strlen(cb->seq) < 3) {
		GtkStyle *style;

		style = gtk_style_copy(gnewt->gRootStyle);
		if (gnewt->useNewtColor) {
			style->bg[0] = getColorByName(
					gnewt->globalColors->checkboxBg);
			style->fg[0] = getColorByName(
					gnewt->globalColors->checkboxFg);
			style->bg[1] = getColorByName(
					gnewt->globalColors->checkboxFg);
			style->fg[1] = getColorByName(
					gnewt->globalColors->checkboxFg);
		}
		cb->back_ground = gtk_event_box_new();
		cb->widget = gtk_check_button_new_with_label(cb->text);
		gtk_widget_set_style(cb->back_ground, style);
		gtk_widget_set_style(cb->widget, style);
		gtk_widget_set_style(GTK_BIN(cb->widget)->child, style);

	} else {
		char tmp[100];
		GtkStyle *style;
		GtkWidget *label_widget;

		cb->widget = gtk_event_box_new();
		sprintf(tmp, "[%c]%s", *cb->result, cb->text);
		label_widget = gtk_label_new(tmp);
		style = gtk_style_copy(gnewt->gRootStyle);
		gtk_widget_set_style(cb->widget, style);
		gtk_widget_set_style(label_widget, style);
		if (gnewt->useNewtColor) {
			style->bg[0] = getColorByName(
					gnewt->globalColors->checkboxBg);
			style->fg[0] = getColorByName(
					gnewt->globalColors->checkboxFg);
		}
		gtk_misc_set_alignment(GTK_MISC(label_widget), 0, 0.5);
		gtk_container_add(GTK_CONTAINER(cb->widget), label_widget);
		gtk_widget_show(label_widget);
	}

	co->ops = &cbOps;

	co->callback = NULL;
	co->height = 1;
	co->width = strlen(text) + 4;
	co->top = top;
	co->left = left;
	co->takesFocus = 1;

	return co;
}

void newtCheckboxSetFlags(newtComponent co, int flags,
			  enum newtFlagsSense sense)
{
	struct checkbox *cb = co->data;

	cb->flags = newtSetFlags(cb->flags, flags, sense);

	if (!(cb->flags & NEWT_FLAG_DISABLED))
		co->takesFocus = 1;
	else
		co->takesFocus = 0;
}

static void cbDraw(newtComponent c)
{
	cbDrawIt(c, 0);
}

static void checkboxCallbackRelease(newtComponent co)
{
	struct checkbox *cb = co->data;

	if (cb->type == RADIO) {
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(cb->widget),
					     TRUE);
	} else if (cb->type == CHECK) {
		if (GTK_TOGGLE_BUTTON(cb->widget)->active) {
			cb->value = cb->seq[1];
		} else {
			cb->value = cb->seq[0];
		}
	}
	if (co->callback)
		co->callback(co, co->callbackData);
}

static void buttonCallbackPressed(GtkWidget * item, newtComponent co)
{
	struct checkbox *cb = co->data;
	const char *cur;
	GtkLabel *label;
	label =
	    (GtkLabel *) GTK_BIN(gtk_menu_get_active(
				GTK_MENU(cb->menu)))->child;
	gtk_label_set_text((GtkLabel *) GTK_BIN(cb->widget)->child,
			   label->label);

	cur = strchr(cb->seq, label->label[1]);
	if (!*cur)
		*cb->result = *cb->seq;
	else
		*cb->result = *cur;

	if (co->callback)
		co->callback(co, co->callbackData);
}

static void checkboxCallbackPressed(newtComponent co)
{
	struct checkbox *cb = co->data;
	const char *cur;

	if (cb->type == RADIO) {
		if (cb->value != cb->seq[1])
			makeActive(co);
	} else if (cb->type == CHECK) {
		cur = strchr(cb->seq, *cb->result);
		if (!cur)
			*cb->result = *cb->seq;
		else {
			cur++;
			if (!*cur)
				*cb->result = *cb->seq;
			else
				*cb->result = *cur;
		}
	}
	if (co->callback)
		co->callback(co, co->callbackData);
}

static void menu_button_press(newtComponent co, GdkEventButton * event)
{
	struct checkbox *cb = co->data;
	gtk_menu_popup(GTK_MENU(cb->menu), NULL, NULL,
		       NULL, NULL, 0, event->time);
}

static void menu_button_leave(newtComponent co, GdkEventButton * event)
{
	struct checkbox *cb = co->data;

	gtk_widget_set_state(cb->widget, 0);
}

static void menu_button_enter(newtComponent co, GdkEventButton * event)
{
	struct checkbox *cb = co->data;

	gtk_widget_set_state(cb->widget, 2);
}

static void cbDrawIt(newtComponent c, int active)
{
	struct checkbox *cb = c->data;
	GtkWidget *item;
	char tmp[100];
	if (c->top == -1)
		return;

	if (!cb->parent) {
		cb->parent = gnewt->currentParent;

		if (strlen(cb->seq) < 3) {
			gtk_widget_set_usize(cb->back_ground,
					     gnewt->FontSizeW * c->width,
					     gnewt->FontSizeH * c->height);
			gtk_fixed_put(GTK_FIXED(cb->parent),
				      cb->back_ground, 
				      gnewt->FontSizeW * c->left,
				      gnewt->FontSizeH * c->top);
			gtk_widget_show(cb->back_ground);
			gtk_container_add(GTK_CONTAINER(cb->back_ground),
					  cb->widget);
			gtk_widget_show(cb->widget);

			gtk_signal_connect_object(GTK_OBJECT(cb->widget),
						  "pressed",
						  GTK_SIGNAL_FUNC
						  (checkboxCallbackPressed),
						  (GtkObject *) c);
			gtk_signal_connect_object(GTK_OBJECT(cb->widget),
						  "released",
						  GTK_SIGNAL_FUNC
						  (checkboxCallbackRelease),
						  (GtkObject *) c);
			if (*cb->result != cb->seq[0]) {
				gtk_toggle_button_set_active
				    (GTK_TOGGLE_BUTTON(cb->widget), TRUE);
			}
		} else {
			int i = 0;
			gtk_widget_set_usize(cb->widget,
					     gnewt->FontSizeW * c->width,
					     gnewt->FontSizeH * c->height);
			gtk_fixed_put(GTK_FIXED(cb->parent), cb->widget,
				      gnewt->FontSizeW * c->left,
				      gnewt->FontSizeH * c->top);
			gtk_widget_show(cb->widget);
			cb->menu = gtk_menu_new();
			while (cb->seq[i]) {
				sprintf(tmp, "[%c]%s", cb->seq[i],
					cb->text);
				item = gtk_menu_item_new_with_label(tmp);
				gtk_widget_show(item);
				gtk_menu_append(GTK_MENU(cb->menu), item);
				if (cb->value == cb->seq[i]) {
					gtk_menu_set_active(GTK_MENU
							    (cb->menu), i);
				}
				gtk_signal_connect(GTK_OBJECT(item),
						   "activate",
						   GTK_SIGNAL_FUNC
						   (buttonCallbackPressed),
						   (GtkObject *) c);
				i++;
			}
			gtk_signal_connect_object(GTK_OBJECT(cb->widget),
						  "button_press_event",
						  GTK_SIGNAL_FUNC
						  (menu_button_press),
						  (GtkObject *) c);
			gtk_signal_connect_object(GTK_OBJECT(cb->widget),
						  "enter_notify_event",
						  GTK_SIGNAL_FUNC
						  (menu_button_enter),
						  (GtkObject *) c);
			gtk_signal_connect_object(GTK_OBJECT(cb->widget),
						  "leave_notify_event",
						  GTK_SIGNAL_FUNC
						  (menu_button_leave),
						  (GtkObject *) c);
		}
	}
	if (cb->type == CHECK && strlen(cb->seq) < 3) {
		if (*cb->result != cb->seq[0]) {
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON
						     (cb->widget), TRUE);
		} else {
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON
						     (cb->widget), FALSE);
		}
	} else if (cb->type != CHECK) {
		if (cb->value != cb->seq[1]) {
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON
						     (cb->widget), FALSE);
		}
	}
	/*newtRefresh();*/
}

static void cbDestroy(newtComponent co)
{
	struct checkbox *cb = co->data;

	free(cb->text);
	free(cb->seq);
	free(cb);
	free(co);
}

struct eventResult cbEvent(newtComponent co, struct event ev)
{
	struct checkbox *cb = co->data;
	struct eventResult er;
	const char *cur;

	if (ev.when == EV_NORMAL) {
		switch (ev.event) {
		case EV_FOCUS:
			er.result = ER_SWALLOWED;
			break;

		case EV_UNFOCUS:
			cbDrawIt(co, 0);
			er.result = ER_SWALLOWED;
			break;

		case EV_KEYPRESS:
			if (ev.u.key == ' ') {
				if (cb->type == RADIO) {
					makeActive(co);
				} else if (cb->type == CHECK) {
					cur = strchr(cb->seq, *cb->result);
					if (!cur)
						*cb->result = *cb->seq;
					else {
						cur++;
						if (!*cur)
							*cb->result =
							    *cb->seq;
						else
							*cb->result = *cur;
					}
					cbDrawIt(co, 1);
					er.result = ER_SWALLOWED;
				} else {
					er.result = ER_IGNORED;
				}
			} else if (ev.u.key == NEWT_KEY_ENTER) {
				er.result = ER_IGNORED;
			} else {
				er.result = ER_IGNORED;
			}
			break;
		case EV_MOUSE:
			if (ev.u.mouse.type == MOUSE_BUTTON_DOWN) {
				if (cb->type == RADIO) {
					makeActive(co);
				} else if (cb->type == CHECK) {
					cur = strchr(cb->seq, *cb->result);
					if (!cur)
						*cb->result = *cb->seq;
					else {
						cur++;
						if (!*cur)
							*cb->result =
							    *cb->seq;
						else
							*cb->result = *cur;
					}
					cbDraw(co);
					er.result = ER_SWALLOWED;

					if (co->callback)
						co->callback(co,
							     co->
							     callbackData);
				}
			}
		}
	} else
		er.result = ER_IGNORED;

	if (er.result == ER_SWALLOWED && co->callback)
		co->callback(co, co->callbackData);

	return er;
}

static void makeActive(newtComponent co)
{
	struct checkbox *cb = co->data;
	struct checkbox *rb;
	newtComponent curr;

	/* find the one that's turned off */
	curr = cb->lastButton;
	while (curr) {
		rb = curr->data;
		if (curr != co) {
			rb->value = rb->seq[0];
			cbDrawIt(curr, 0);
		}
		curr = rb->prevButton;
	}
	cb->value = cb->seq[1];
	cbDrawIt(co, 1);
}
