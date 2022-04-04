#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>

#include "newt.h"
#include "newt_pr.h"

struct scrollbar {
    int curr;
    int cs, csThumb;
} ;

static void sbDraw(newtComponent co);
static void sbDestroy(newtComponent co);
static void sbDrawThumb(newtComponent co, int isOn);

static struct componentOps sbOps = {
    sbDraw,
    newtDefaultEventHandler,
    sbDestroy,
    newtDefaultPlaceHandler,
    newtDefaultMappedHandler,
} ;

void newtScrollbarSet(newtComponent co, int where, int total) {
    struct scrollbar * sb = co->data;
    int new;

    new = (where * (co->height - 1)) / (total ? total : 1);
    if (new != sb->curr) {
	sbDrawThumb(co, 0);
	sb->curr = new;
	sbDrawThumb(co, 1);
    }
}

newtComponent newtVerticalScrollbar(int left, int top, int height,
				    int normalColorset, int thumbColorset) {
    newtComponent co;
    struct scrollbar * sb;

    co = malloc(sizeof(*co));
    sb = malloc(sizeof(*sb));
    co->data = sb;

    sb->curr = 0;
    sb->cs = normalColorset;
    sb->csThumb = thumbColorset;

    co->ops = &sbOps;
    co->isMapped = 0;
    co->left = left;
    co->top = top;
    co->height = height;
    co->width = 1;
    co->takesFocus = 0;  
    
    return co;
}

static void sbDraw(newtComponent co) {
    int i;

    if (!co->isMapped) return;

    for (i = 0; i < co->height; i++) {
	newtGotorc(i + co->top, co->left);
    }

    sbDrawThumb(co, 1);
}

static void sbDrawThumb(newtComponent co, int isOn) {
    struct scrollbar * sb = co->data;

    if (!co->isMapped) return;

    newtGotorc(sb->curr + co->top, co->left);

}

static void sbDestroy(newtComponent co) {
    struct scrollbar * sb = co->data;
 
    free(sb);
    free(co);
}
