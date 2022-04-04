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

#include "newt.h"
#include "newt_pr.h"
#include "gnewt.h"
#include "gnewt_pr.h"

struct scale {
	long long fullValue;
	int charsSet;
	GtkWidget *widget;
	GtkWidget *parent;
};
extern struct Gnewt *gnewt;

static void scaleDraw(newtComponent co);

static struct componentOps scaleOps = {
	scaleDraw,
	newtDefaultEventHandler,
	NULL,
	newtDefaultPlaceHandler,
	newtDefaultMappedHandler,
};

newtComponent newtScale(int left, int top, int width, long long fullValue)
{
	newtComponent co;
	struct scale *sc;

	co = malloc(sizeof(*co));
	sc = malloc(sizeof(struct scale));
	co->data = sc;

	co->ops = &scaleOps;

	co->height = 1;
	co->width = width;
	co->top = top;
	co->left = left;
	co->takesFocus = 0;

	sc->fullValue = fullValue;
	sc->charsSet = 0;
	sc->parent = NULL;
	sc->widget = gtk_progress_bar_new();

	return co;
}

void newtScaleSet(newtComponent co, unsigned long long amount)
{
	struct scale *sc = co->data;
	int newCharsSet;

	newCharsSet = (amount * co->width) / sc->fullValue;

	if (newCharsSet != sc->charsSet) {
		sc->charsSet = newCharsSet;
		scaleDraw(co);
	}
}

static void scaleDraw(newtComponent co)
{
	struct scale *sc = co->data;
	GtkStyle *style;

	if (co->top == -1)
		return;
	if (!sc->parent && GTK_IS_FIXED(gnewt->currentParent)) {
		sc->parent = gnewt->currentParent;
		style = gtk_style_copy(gnewt->gRootStyle);
		if (gnewt->useNewtColor) {
			style->bg[0] = getColorByName(
					gnewt->globalColors->emptyScale);
			style->bg[2] = getColorByName(
					gnewt->globalColors->fullScale);
		}
		gtk_widget_set_style(&GTK_PROGRESS(sc->widget)->widget,
				     style);

		gtk_widget_show(sc->widget);
		gtk_widget_set_usize(sc->widget,
				     gnewt->FontSizeW * co->width,
				     gnewt->FontSizeH * co->height);
		gtk_fixed_put(GTK_FIXED(sc->parent), sc->widget,
			      gnewt->FontSizeW * co->left, 
			      gnewt->FontSizeH * co->top);
	}
	gtk_progress_set_percentage(GTK_PROGRESS(sc->widget),
				    (double) sc->charsSet / co->width);
	/*newtRefresh();*/
}
