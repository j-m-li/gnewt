
#include "newt.h" 	/* standard newt include file */
#include "gnewt.h"	/* gnewt special functions inculde file */ 

int main(void)
{
	gnewtCmd("set_style:0:0:0:"); 	/* set the default look of gnewt :
					 * - don't use newt colors
					 * - don't use 16 colors display
					 * - don't show a global background
					 */
	
	newtInit(); 	/* initialize the toolkit */

	newtCls(); 	/* clear the screen */

	/* Display a message : */
	newtWinMessage("gNewt Hello", "Bye", "Hello world!"); 

	newtFinished();	/* de-initialize the toolkit */

	return 0;
}
