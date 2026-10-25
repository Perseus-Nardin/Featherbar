#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<X11/Xlib.h>
#include<err.h>
#include<X11/Xft/Xft.h>
#include<unistd.h>
#include<signal.h>
#include<stdbool.h>

// Your settings right here
int width;
int height = 30;
int posx = 0;
int posy = 0;
const char *bg_color = "#000000";
const char *fg_color = "#FFFFFF";
const char *font_name = "JetBrainsMono Nerd Font:size=14";
const char *bar_content = "featherbar_script";
const int   update_rate = 1; // Updates every one second
const int   length = 4096; // The length of the bar by characters (one character = one byte)

static volatile sig_atomic_t running = 1;

Display* dpy;
Window root, win;
XSetWindowAttributes attr;
XColor background;
Colormap cmap;
int scr;

static void handle_signal(int sig){
    // (void)sig;
    if (sig == SIGINT || sig == SIGTERM)
	running = 0;}

char *exec_cmd(const char *cmd){
    char *output = calloc(length, 1);
    
    if (!output)
	err(1, "Failed to allocate output");

    FILE *fp = popen(cmd, "r");

    if (!fp) {
        free(output);
        err(1, "Failed to read command");}

    fread(output, 1, 4095, fp);
    pclose(fp);

    return output;}

int main(){
    struct sigaction sa = {0};
    sa.sa_handler = handle_signal;
    sigemptyset(&sa.sa_mask);

    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGUSR1, &sa, NULL);

    dpy = XOpenDisplay(NULL);

    if (!dpy)
        err(1, "Connection to X failed\n");

    scr = DefaultScreen(dpy);
    root = DefaultRootWindow(dpy);

    cmap = DefaultColormap(dpy, scr);

    XParseColor(dpy, cmap, bg_color, &background);
    XAllocColor(dpy, cmap, &background);
    attr.background_pixel = background.pixel;

    attr.override_redirect = True;

    width = DisplayWidth(dpy, scr);

    win = XCreateWindow(dpy, root, posx, posy, width, height, 0, CopyFromParent, InputOutput, CopyFromParent, CWOverrideRedirect| CWBackPixel, &attr);

    Visual *visual = DefaultVisual(dpy, scr);
    XftFont *font = XftFontOpenName(dpy, scr, font_name);
    if (!font)
	err(1, "Failed to load font");
    XftDraw *draw = XftDrawCreate(dpy, win, visual, cmap);
    if (!draw)
	err(1, "Failed to create draw");

    XftColor foreground_color;
    XftColorAllocName(dpy, visual, cmap, fg_color, &foreground_color);

    XMapWindow(dpy, win);

    int baseline = (height - font->height) / 2 + font->ascent;

    void update(){
	char *text = exec_cmd(bar_content);
    
	XClearWindow(dpy, win);
	size_t len = strlen(text);
       	if (len && text[len - 1] == '\n')
	    --len;
	if (len)
	    XftDrawStringUtf8(draw, &foreground_color, font, 10, baseline, (const FcChar8 *)text, strlen(text)-1);
	free(text);}

    while (True){
	if (running)
	    update();

	XFlush(dpy);
	sleep(update_rate);
    }
    
    /* Release Xft resources */
    XftDrawDestroy(draw);
    XftColorFree(dpy, visual, cmap, &foreground_color);
    XftFontClose(dpy, font);

    /* Release X11 resources */
    XDestroyWindow(dpy, win);
    XFreeColors(dpy, cmap, &background.pixel, 1, 0);
    XCloseDisplay(dpy);

    return 0;}
