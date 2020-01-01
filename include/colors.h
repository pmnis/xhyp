#ifndef __COLORS_H__
#define __COLORS_H__

#define COLOR_BLACK	"[30m"
#define COLOR_RED	"[31m"
#define COLOR_GREEN	"[32m"
#define COLOR_YELLOW	"[33m"
#define COLOR_BLUE	"[34m"
#define COLOR_MAGENTA	"[35m"
#define COLOR_CYAN	"[36m"
#define COLOR_WHITE	"[37m"

#define COLOR   "[3"

extern int color_on;

static inline void reset_color(void)
{
	if (color_on)
		printk(COLOR_WHITE);
}

#endif
