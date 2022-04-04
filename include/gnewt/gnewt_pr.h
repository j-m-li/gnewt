#ifndef H_GNEWTPR
#define H_GNEWTPR

#ifdef __cplusplus
extern "C" {
#endif

struct Window {
    int height, width, top, left;
    short * buffer;
    char * title;
    GtkWidget* widget;
    GtkWidget* bg_widget;
    GdkPixmap* pixmap;
    GtkWidget* top_widget;
};

struct Gnewt {
	int version;
	struct Window *currentWindow;
	int pressedKey;
	GtkStyle *gRootStyle;
	GtkWidget* currentParent;
	int FontSizeW; 
	int FontSizeH;
	int formLevel;
	newtComponent currentExitComp;
	GtkWidget *gRootWindow;
	int useNewtColor;
	int Color4bits;
	gchar **colorTable;
	struct newtColors *globalColors;
	newtComponent gObject;
	GdkPixmap *buttonPixmap;
	GdkBitmap *buttonMask;
	int showBackground;
};

GdkColor getColorByName(gchar * name);
newtComponent gnewtExtended (int top, const char *text);

#ifdef __cplusplus
} /* End of extern "C" { */
#endif

#endif /* H_GNEWTPR */

