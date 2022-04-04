#ifndef H_GNEWT
#define H_GNEWT

#ifdef __cplusplus
extern "C" {
#endif

/* this is the same structure as newtComponent_struct */
struct gnewtData_struct {
    	/* common data */
    	int height, width;
    	int top, left;
    	int takesFocus;
    	int isMapped;
    	void * ops;
    	void * callback;
    	void * callbackData;
   	void * data;
};

typedef struct gnewtData_struct * gnewtData;

#define gnewtCmd(txt) (gnewtData)newtLabel(-300, (int)':', txt)
#define IS_GNEWT(res) (res->top == 300)

#ifdef __cplusplus
} /* End of extern "C" { */
#endif

#endif /* H_GNEWT */

