/*   Gtk+ version of the newt library
 *
 *   Copyright (C) 2000  O'ksi'D
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
};
extern struct Gnewt *gnewt;

static int gnewtRootXpm (char *name, int x, int y, int w, int h) {
        GtkWidget *wpixmap;
	GdkPixmap *pixmap = NULL;
	GdkBitmap *mask;

        pixmap = gdk_pixmap_create_from_xpm(
	       		gnewt->gRootWindow->window,
                        &mask,
                        &gnewt->gRootStyle->bg[GTK_STATE_NORMAL],
                        name);
	if (pixmap) {
		wpixmap = gtk_pixmap_new(pixmap, mask);
		gdk_pixmap_unref(pixmap);
		gdk_pixmap_unref(mask);
		gtk_fixed_put(GTK_FIXED(gnewt->gRootWindow),
				wpixmap, x * gnewt->FontSizeW,
				y * gnewt->FontSizeH);
		gtk_widget_show(wpixmap);
	}
	return 0;
}

static int gnewtSetButtonXpm (char *name) {
/*	if (gnewt->buttonPixmap) {
		gdk_pixmap_unref(gnewt->buttonPixmap);
		gdk_pixmap_unref(gnewt->buttonMask);
	}
*/
	gnewt->buttonPixmap = gdk_pixmap_create_from_xpm(
	       			gnewt->currentWindow->widget->window,
                        	&gnewt->buttonMask,
                        	&gnewt->currentWindow->widget->style->
					bg[GTK_STATE_NORMAL],
				name);
}

static int gnewtWinXpm (char *name, int x, int y, int w, int h) {
	GdkPixmap *pixmap = NULL;
	GdkBitmap *mask;

        pixmap = gdk_pixmap_create_from_xpm(
	       		gnewt->currentWindow->widget->window,
                        &mask,
                        &gnewt->currentWindow->widget->style->
				bg[GTK_STATE_NORMAL],
                        name);
	if (pixmap) {
		gdk_gc_set_clip_mask (
				gnewt->currentWindow->widget->style->black_gc,
				mask);
		gdk_gc_set_clip_origin (
				gnewt->currentWindow->widget->style->black_gc,
				x * gnewt->FontSizeW,
				y * gnewt->FontSizeH);
		
		gdk_draw_pixmap (gnewt->currentWindow->pixmap,
				gnewt->currentWindow->widget->style->black_gc,
				pixmap,
				0, 0, 
				x * gnewt->FontSizeW,
				y * gnewt->FontSizeH,
				-1, -1);
		gdk_gc_set_clip_mask (
				gnewt->currentWindow->widget->style->black_gc,
				NULL);
		gdk_gc_set_clip_origin (
				gnewt->currentWindow->widget->style->black_gc,
				0, 0);
				
		gdk_pixmap_unref(pixmap);
		gdk_pixmap_unref(mask);
	}
	return 0;
}

/*************************************************************************/
newtComponent gnewtExtended (int top, const char *text)
{
	char *cmd[15];
        char *txt = strdup(text);
        int i = 1;

	if (!gnewt->gObject) {
		gnewt->gObject = malloc(sizeof(*gnewt->gObject));
		gnewt->gObject->data = NULL;
		gnewt->gObject->ops = NULL;
		gnewt->gObject->top = 0; /* must be set to 300 if 
					  *  everything is ok */
		gnewt->gObject->left = 0;
		gnewt->gObject->takesFocus = 0;
	}
	gnewt->gObject->top = 0;  
	gnewt->gObject->height = gnewt->FontSizeH;
	gnewt->gObject->width = gnewt->FontSizeW;

        cmd[0] = txt;
        cmd[1] = "";
        while (*txt) {
		if (*txt == (char) top) {
                       *txt = '\0';
                       cmd[i] = txt + 1;
                       i++;
		}
                txt++;
        }
	if (!strcmp(cmd[0], "set_style")) {
		gnewt->useNewtColor = atoi(cmd[1]);
		gnewt->Color4bits = atoi(cmd[2]);
		gnewt->showBackground = atoi(cmd[3]);
		gnewt->gObject->height = gnewt->FontSizeH;
		gnewt->gObject->width = gnewt->FontSizeW;
		gnewt->gObject->data = "set colors\n";
		gnewt->gObject->top = 300;
	}
	if (!strcmp(cmd[0], "root_xpm")) {
		if (gnewt->showBackground) {
			gnewtRootXpm(cmd[1], atoi(cmd[2]), atoi(cmd[3]),
					atoi(cmd[4]), atoi(cmd[5]));
		}
		gnewt->gObject->data = "Root pixmap draw\n";
		gnewt->gObject->top = 300;
	}
	if (!strcmp(cmd[0], "win_xpm")) {
		gnewtWinXpm(cmd[1], atoi(cmd[2]), atoi(cmd[3]),
				atoi(cmd[4]), atoi(cmd[5]));
		gnewt->gObject->data = "Window pixmap draw\n";
		gnewt->gObject->top = 300;
	}
	if (!strcmp(cmd[0], "set_button_xpm")) {
		gnewtSetButtonXpm(cmd[1]);
		gnewt->gObject->data = "Set default button pixmap\n";
		gnewt->gObject->top = 300;
	}
	free (cmd[0]);

	return gnewt->gObject;
}

