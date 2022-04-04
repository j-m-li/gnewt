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

struct label {
	char *text;
	int length;
	GtkWidget *widget;
	GtkWidget *parent;
	GtkWidget *bgWidget;
};
extern struct Gnewt *gnewt;

static void labelDraw(newtComponent co);
static void labelDestroy(newtComponent co);

static struct componentOps labelOps = {
	labelDraw,
	newtDefaultEventHandler,
	labelDestroy,
	newtDefaultPlaceHandler,
	newtDefaultMappedHandler,
};

newtComponent newtLabel(int left, int top, const char *text)
{
	newtComponent co;
	struct label *la;

	/************** gNewt extended features ***************/
	if (left == -300) {
		return gnewtExtended (top, text);
	}

	co = malloc(sizeof(*co));
	la = malloc(sizeof(struct label));
	co->data = la;

	co->ops = &labelOps;

	if (left < 0)
		left = 0;
	if (top < 0)
		top = 0;
	co->height = 1;
	co->width = strlen(text);
	co->top = top;
	co->left = left;
	co->takesFocus = 0;

	la->length = strlen(text);
	la->text = strdup(text);
	la->widget = NULL; 
	la->parent = NULL;
	la->bgWidget = NULL;

	return co;
}

void newtLabelSetText(newtComponent co, const char *text)
{
	int newLength;
	struct label *la = co->data;

	newLength = strlen(text);
	free(la->text);
	la->text = strdup(text);
	la->length = newLength;
	co->width = newLength;
	/*labelDraw(co);*/
}

static void labelDraw(newtComponent co)
{
	struct label *la = co->data;
	GtkStyle *style;

	if (co->top == -1)
		return;
	if (co->isMapped == -1) return;

	if (!la->parent) {
		int len1, len2;
		la->parent = gnewt->currentParent;
		la->bgWidget = gtk_fixed_new();
		len1 = co->width * gnewt->FontSizeW;
		len2 = gdk_string_width (gnewt->gRootStyle->font, la->text);
		if (len2 > len1) len1 = len2;
		gtk_widget_set_usize(la->bgWidget, len1,
			co->height * gnewt->FontSizeH);
		gtk_widget_show(la->bgWidget);
		gtk_fixed_put(GTK_FIXED(la->parent), la->bgWidget,
		      gnewt->FontSizeW * co->left, 
		      gnewt->FontSizeH * co->top);
	}
	if (la->widget) {
		gtk_widget_destroy(la->widget);
	}
	la->widget = gtk_label_new(la->text);
	if (gnewt->useNewtColor) {
		style = gtk_style_copy(gnewt->gRootStyle);
		style->fg[0] = 
			getColorByName(gnewt->globalColors->labelFg);
		gtk_widget_set_style(la->widget, style);
		gtk_widget_set_style(la->bgWidget, la->parent->style);
	}
	gtk_widget_show(la->widget);
	gtk_fixed_put(GTK_FIXED(la->bgWidget), la->widget, 0, 0);
	/*newtRefresh();*/
}

static void labelDestroy(newtComponent co)
{
	struct label *la = co->data;

	free(la->text);
	free(la);
	free(co);
}
