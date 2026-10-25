#include <graphics.h>
#include <mouse.c>
#include <stdio.h>
#include <dos.h>
#include <stdlib.h>
#include <string.h>
#include "icons."

// Defines
// Maximums
#define MWINDOWS	15
#define MAREAS		100
#define MLABELS     30
#define MBUTTONS	50
#define MOBJECTS	30
#define MSPINS		10
#define MSCROLLS	15
#define MUSERS		15
#define MLISTBOXES	15
#define MITEMS		500		// Maximum items in a listbox

// Attribs
#define NORMAL		0
#define MOVE		1
#define CLOSE		2
#define RESIZE		4
#define MAXIMIZE	8
#define MINIMIZE	16
#define ALL			31

// Positions
#define LEFT   		1
#define RIGHT       2
#define TOP         3
#define BOTTOM      4
#define CENTER      5

// Types
#define LABEL		1
#define BUTTON		2
#define ICONBUTTON	3
#define HSCROLL		4
#define VSCROLL		5
#define LISTBOX		6

// Colors
#define NONE		-1


// Globals
char order[MWINDOWS]={0},maxwin=0;


// Structures
struct {
	int name,size,height,width[255];
} font;

struct {
	int areaptr,clip;
	char attrib;
	char *caption,back;
	char objects[MOBJECTS][2],state;
	char userarea[MUSERS];
	void (*usercall[MUSERS])(void);
	void (*redraw)(void);
} win[MWINDOWS];

struct {
	int x1,y1,x2,y2;
	char used;
} area[MAREAS];

struct {
	int areaptr,clip;
	char *caption,hal,val;
	char text,back;
} labelinfo[MLABELS];

struct {
	int areaptr,clip;
	char *caption,color,enabled;
	void (*onclick)(int);
	int *data;
} buttoninfo[MBUTTONS];

struct {
	char color;
	char pattern;
	int width,height;
} desktop;

struct {
	int x1,y1,x2,y2;
} restoreinfo[MWINDOWS];

struct {
	char used;
	int low,high;
	int x1,y1,width,labelptr,*value;
	void (*onclick)(int);
} spininfo[MSPINS];

struct {
	int spinptr;
	char inc;
} spintable[MBUTTONS];

struct {
	int x1,y1,x2;
	int *value,high;
	int clip,areaptr;
	int lbut,rbut,mbut;
	void (*onclick)(int);
} hscrollinfo[MSCROLLS];

struct {
	int ptr;
	char inc;
} hscrolltable[MBUTTONS];

struct {
	int x1,y1,y2;
	int *value,high;
	int clip,areaptr;
	int lbut,rbut,mbut;
	void (*onclick)(int);
} vscrollinfo[MSCROLLS];

struct {
	int ptr;
	char inc;
} vscrolltable[MBUTTONS];

struct {
	int areaptr,clip;
	int items,vmax;
	int selected,topitem;
	char *text[MITEMS];
} listinfo[MLISTBOXES];


// Functions
void controlicon(int,char,int,char);
void maximize(void);
void minimize(void);
void close(void);
void restore(void);
void drawhscroll(int);
void drawvscroll(int);
void clrscr(char);

// Set current font
void setfont(int name,int size) {
	// Variables
	int ch;
	char string[2];

	// Set font
	settextstyle(name,0,size);

	// Store widths+heigth,etc.
	font.name=name;
	font.size=size;
	font.height=textheight("Tg€")+2;

	for(ch=32;ch<256;ch++) {
		sprintf(string,"%c\0",ch);
		font.width[ch]=textwidth(string);
	}
}


// Set color table
void setcolors(void) {
	setrgbpalette(BLUE,0,0,32);
	setrgbpalette(GREEN,0,32,0);
	setrgbpalette(CYAN,0,32,32);
	setrgbpalette(RED,32,0,0);
	setrgbpalette(MAGENTA,32,0,32);
	setrgbpalette(BROWN,32,16,0);
	setrgbpalette(LIGHTBLUE,0,0,64);
	setrgbpalette(LIGHTGREEN,0,64,0);
	setrgbpalette(LIGHTCYAN,0,64,64);
	setrgbpalette(LIGHTRED,64,0,0);
	setrgbpalette(LIGHTMAGENTA,64,0,64);
	setrgbpalette(YELLOW,64,64,0);
}


// Set font to small
void setsmallfont(void) {
	// Set font
	settextstyle(SMALL_FONT,0,4);
}


// Restore font
void restorefont(void) {
	// Set font
	setfont(font.name,font.size);
}



// Initialise graphics mode
void opengraph(void) {
	// Variables
	int gdriver=VGA,gmode=VGAHI,errorcode;

	// Check mouse presence
	if(!minit()) {
		puts("Error - Mouse not found");
		exit(0);
	}

	// Enter graphics mode
	registerbgidriver(EGAVGA_driver);
	registerbgifont(small_font);
	initgraph(&gdriver,&gmode,"");

	errorcode=graphresult();

	if(errorcode!=grOk) {
		puts("Error initialising graphics mode");
		exit(0);
	}

	// Store desktop variables
	desktop.width=getmaxx();
	desktop.height=getmaxy();

	// Set font variables
	setfont(SMALL_FONT,4);

	// Set color values
	setcolors();

	// Clear desktop
	setfillstyle(desktop.pattern,desktop.color);
	bar(0,0,desktop.width,desktop.height);

	// Display mouse pointer
	mshow();
}


// Delete an area element
void delarea(int num) {
	area[num].used=0;
}


// Delete a label element
void dellabel(int num) {
	delarea(labelinfo[num].areaptr);
	delarea(labelinfo[num].clip);

	free(labelinfo[num].caption);

	labelinfo[num].areaptr=0;
	labelinfo[num].clip=0;
	labelinfo[num].back=0;
	labelinfo[num].text=0;
	labelinfo[num].hal=0;
	labelinfo[num].val=0;
}


// Delete a button element
void delbutton(int num) {
	delarea(buttoninfo[num].areaptr);
	delarea(buttoninfo[num].clip);

	free(buttoninfo[num].caption);
	delete buttoninfo[num].data;

	buttoninfo[num].areaptr=0;
	buttoninfo[num].clip=0;
	buttoninfo[num].color=0;
	buttoninfo[num].onclick=NULL;
	buttoninfo[num].enabled=0;

	spintable[num].spinptr=0;
	spintable[num].inc=0;
	vscrolltable[num].ptr=0;
	vscrolltable[num].inc=0;
	hscrolltable[num].ptr=0;
	hscrolltable[num].inc=0;
}


// Delete a vertical scroll bar element
void delvscrollbar(int num) {
	delarea(vscrollinfo[num].areaptr);
	delarea(vscrollinfo[num].clip);

	vscrollinfo[num].areaptr=0;
	vscrollinfo[num].clip=0;
	vscrollinfo[num].x1=0;
	vscrollinfo[num].y1=0;
	vscrollinfo[num].y2=0;
	vscrollinfo[num].onclick=NULL;
}


// Delete a horizontal scroll bar element
void delhscrollbar(int num) {
	delarea(hscrollinfo[num].areaptr);
	delarea(hscrollinfo[num].clip);

	hscrollinfo[num].areaptr=0;
	hscrollinfo[num].clip=0;
	hscrollinfo[num].x1=0;
	hscrollinfo[num].y1=0;
	hscrollinfo[num].x2=0;
	hscrollinfo[num].onclick=NULL;
}


// Delete a character in order string
void delchar(char string[MWINDOWS],int pos) {

	for(int loop=pos;loop<maxwin;loop++)
		string[loop]=string[loop+1];

	string[maxwin]=0;

	maxwin--;
}


// Delete a window element
void delwindow(int num) {
	// Return if not used
	if(!win[num].areaptr)
		return;

	// Delete from order list
	for(int loop=1;loop<maxwin;loop++)
		if(order[loop]==num)
			delchar(order,loop);

	// Delete areas
	delarea(win[num].areaptr);
	delarea(win[num].clip);

	// Free caption memory
	free(win[num].caption);

	// Reset components
	win[num].areaptr=0;
	win[num].state=NORMAL;
	win[num].redraw=NULL;
	win[num].attrib=0;
	win[num].clip=0;

	// Clear object links
	for(loop=1;loop<MOBJECTS;loop++) {
		switch(win[num].objects[loop][0]) {
			case LABEL :
				dellabel(win[num].objects[loop][1]);
				break;
			case BUTTON :
				delbutton(win[num].objects[loop][1]);
				break;
			case VSCROLL :
				delvscrollbar(win[num].objects[loop][1]);
				break;
			case HSCROLL :
				delhscrollbar(win[num].objects[loop][1]);
				break;
		}

		win[num].objects[loop][0]=0;
		win[num].objects[loop][1]=0;
	}
}


// Shutdown program
void shutdown(void) {
	// Variables
	int loop;

	// Close graphics mode
	clrscr(BLACK);
	closegraph();

	// Free window caption allocations
	for(loop=1;loop<MWINDOWS;loop++)
		delwindow(loop);
}


// Get height of window area
int getheight(int winnum) {
	return(area[win[winnum].clip].y2-area[win[winnum].clip].y1);
}


// Get width of window area
int getwidth(int winnum) {
	return(area[win[winnum].clip].x2-area[win[winnum].clip].x1);
}


// Add element to area structure
int addarea(int x1,int y1,int x2,int y2) {
	// Variables
	int slot;

	// Find empty slot in structure
	for(slot=1;slot<MAREAS;slot++) {
		// Add data if slot is empty
		if(!area[slot].used) {
			area[slot].x1=x1;
			area[slot].y1=y1;
			area[slot].x2=x2;
			area[slot].y2=y2;

			area[slot].used=1;

			return(slot);
		}
	}

	// No slot found, return 0
	return(0);
}


// Find element in window structure
int windowslot(void) {
	// Variables
	int slot;

	// Find empty slot in structure
	for(slot=1;slot<MWINDOWS;slot++) {
		// Return slot number if empty
		if(!win[slot].areaptr)
			return(slot);
	}

	// No slot found, return 0
	return(0);
}


// Find element in label structure
int labelslot(int areaptr,int clip,char text,char back,char hal,char val) {
	// Variables
	int slot;

	// Find empty slot in structure
	for(slot=1;slot<MLABELS;slot++) {
		// Return slot number if empty
		if(!labelinfo[slot].areaptr) {
			labelinfo[slot].areaptr=areaptr;
			labelinfo[slot].clip=clip;
			labelinfo[slot].text=text;
			labelinfo[slot].back=back;
			labelinfo[slot].hal=hal;
			labelinfo[slot].val=val;

			return(slot);
		}
	}

	// No slot found, return 0
	return(0);
}


// Find element in button structure
int buttonslot(int areaptr,int clip,char color,void (*onclick)(int)) {
	// Variables
	int slot;

	// Find empty slot in structure
	for(slot=1;slot<MBUTTONS;slot++) {
		// Return slot number if empty
		if(!buttoninfo[slot].areaptr) {
			buttoninfo[slot].areaptr=areaptr;
			buttoninfo[slot].clip=clip;
			buttoninfo[slot].color=color;
			buttoninfo[slot].onclick=onclick;
			buttoninfo[slot].enabled=1;

			return(slot);
		}
	}

	// No slot found, return 0
	return(0);
}


// Add element in object structure
int objectslot(int winnum,int type,int num) {
	// Variables
	int slot;

	// Find empty slot in structure
	for(slot=1;slot<MOBJECTS;slot++) {
		// Fill element and return number
		if(!win[winnum].objects[slot][0]) {
			win[winnum].objects[slot][0]=type;
			win[winnum].objects[slot][1]=num;

			return(slot);
		}
	}

	// No slot found, return 0
	return(0);
}


// Add element in user structure
int userslot(int winnum,int areaptr,void (*func)(void)) {
	// Variables
	int slot;

	// Find empty slot in structure
	for(slot=1;slot<MUSERS;slot++) {
		// Fill element and return number
		if(!win[winnum].userarea[slot]) {
			win[winnum].userarea[slot]=areaptr;
			win[winnum].usercall[slot]=func;

			return(slot);
		}
	}

	// No slot found, return 0
	return(0);
}


// Add element in user structure
int listslot(int winnum,int areaptr) {
	// Variables
	int slot;

	// Find empty slot in structure
	for(slot=1;slot<MLISTBOXES;slot++) {
		// Fill element and return number
		if(!listinfo[slot].items) {
			listinfo[slot].topitem=0;
			listinfo[slot].selected=0;

			return(slot);
		}
	}

	// No slot found, return 0
	return(0);
}


// Add element in spinner structure
int spinslot() {
	// Variables
	int slot;

	// Find empty slot in structure
	for(slot=1;slot<MSPINS;slot++) {
		// Return number
		if(!spininfo[slot].used)
			return(slot);
	}

	// No slot found, return 0
	return(0);
}


// Add element in spinner structure
int hscrollslot() {
	// Variables
	int slot;

	// Find empty slot in structure
	for(slot=1;slot<MSCROLLS;slot++) {
		// Return number
		if(!hscrollinfo[slot].x2)
			return(slot);
	}

	// No slot found, return 0
	return(0);
}


// Add element in spinner structure
int vscrollslot() {
	// Variables
	int slot;

	// Find empty slot in structure
	for(slot=1;slot<MSCROLLS;slot++) {
		// Return number
		if(!vscrollinfo[slot].y2)
			return(slot);
	}

	// No slot found, return 0
	return(0);
}


// Return maximum of two numbers
int max(int num1,int num2) {
	return((num1>num2)?num1:num2);
}


// Return minimum of two numbers
int min(int num1,int num2) {
	return((num1<num2)?num1:num2);
}


// Umm. A function that Beeps?
void beep(void) {
	// Beep
	sound(1000);
	delay(8);
	nosound();
}


// Insert a character in order string
void stuff(char string[MWINDOWS],int pos,char ch) {
	// Variables
	int start;

	// Find end of list
	start=min(maxwin+1,MWINDOWS-1);

	// Move array elements
	for(int loop=start;loop>pos;loop--)
		string[loop]=string[loop-1];

	string[pos]=ch;

	maxwin++;
}


// Check to see if a point is inside an area
char zone(int x,int y,int x1,int y1,int x2,int y2) {
	return((x>=x1 && x<=x2 && y>=y1 && y<=y2)?1:0);
}

char zone(int x,int y,int areaptr) {
	return(zone(x,y,area[areaptr].x1,area[areaptr].y1,
					area[areaptr].x2,area[areaptr].y2));
}


// Create window
int window(char *caption,int x1,int y1,int x2,int y2) {
	// Variables
	int winnum,areaptr,clip;

	// Check dimensions
	if(x2-x1+1<100)
		x2=x1+100;
	if(y2-y1+1<19)
		y2=y1+19;

	// Add window area structure, return if error
	areaptr=addarea(x1,y1,x2,y2);
	if(!areaptr)
		return(0);

	// Add clip area structure, return if error
	clip=addarea(x1+3,y1+18,x2-3,y2-3);
	if(!clip)
		return(0);

	// Store window data, return if error
	winnum=windowslot();
	if(!winnum)
		return(0);

	win[winnum].areaptr=areaptr;
	win[winnum].clip=clip;
	win[winnum].attrib=MOVE+CLOSE+RESIZE+MAXIMIZE+MINIMIZE;
	win[winnum].redraw=NULL;
	win[winnum].state=NORMAL;
	win[winnum].back=LIGHTGRAY;

	// Allocate space for caption, quit if error
	if(strlen(caption)) {
		win[winnum].caption=(char *) malloc(strlen(caption)+1);
		if(!win[winnum].caption) {
			shutdown();
			puts("Error allocating caption memory in WINDOW");
			exit(0);
		}

		strcpy(win[winnum].caption,caption);
	}

	// Insert window number at element 1 of order
	stuff(order,1,winnum);

	// Return window number
	return(winnum);
}


// Set viewport to area
char setviewport(int x1,int y1,int x2,int y2) {
	// Trim area to screen
	x1=(x1<0)?0:x1;
	x1=(x1>desktop.width)?desktop.width:x1;
	y1=(y1<0)?0:y1;
	y1=(y1>desktop.height)?desktop.height:y1;

	x2=(x2<0)?0:x2;
	x2=(x2>desktop.width)?desktop.width:x2;
	y2=(y2<0)?0:y2;
	y2=(y2>desktop.height)?desktop.height:y2;

	// Set clipping area
	setviewport(x1,y1,x2,y2,1);

	// Return true if successfull
	return((x1==x2 || y1==y2)?0:1);
}


char setviewport(int areaptr) {
	// Variables
	int x1,y1,x2,y2;

	// Get area
	x1=area[areaptr].x1;
	y1=area[areaptr].y1;
	x2=area[areaptr].x2;
	y2=area[areaptr].y2;

	// Set clipping area
	return(setviewport(x1,y1,x2,y2));
}


// Reset viewport
char setviewport(void) {
	return(setviewport(0,0,desktop.width,desktop.height));
}


// Display text
void output(int x,int y,char *string,
			int ax1,int ay1,int ax2,int ay2,	// Draw area
			int bx1,int by1,int bx2,int by2,	// Clip area
			char ho,char vo) {

	// Variables
	int left,top;

	// Return if out of area
	if(ax1+x<bx1 || ay1+y<by1)
		return;

	// Return if off screen
	if(!zone(ax1+x,ay1+y,0,0,desktop.width,desktop.height))
		return;

	// Set drawing font
	setfont(font.name,font.size);

	// Find horizontal offsets
	switch(ho) {
		case LEFT :
			left=x;
			break;
		case CENTER :
			left=x+(ax2-ax1+1-textwidth(string))/2;
			break;
		case RIGHT :
			left=x+ax2-ax1-textwidth(string);
			break;
	}

	// Find vertical offsets
	switch(vo) {
		case TOP :
			top=y;
			break;
		case CENTER :
			top=y+(ay2-ay1-font.height)/2;
			break;
		case BOTTOM :
			top=y+ay2-ay1-font.height;
			break;
	}

	// Set clipping area
	setviewport(bx1,by1,bx2,by2);

	// Display text
	mhide();
	outtextxy(ax1-bx1+left,ay1-by1+top,string);
	mshow();

	// Reset clipping area
	setviewport();
}


// Display icon
void output(int x,int y,int *data,
			int ax1,int ay1,int ax2,int ay2,	// Draw area
			int bx1,int by1,int bx2,int by2,	// Clip area
			char ho,char vo) {

	// Variables
	int left,top,width,height,xl,yl,count;

	// Return if out of area
	if(ax1+x<bx1 || ay1+y<by1)
		return;

	// Return if off screen
	if(!zone(ax1+x,ay1+y,0,0,desktop.width,desktop.height))
		return;

	// Get size of icon
	width=data[0];
	height=data[1];

	// Return if no data
	if(!data[0] || !data[1])
		return;

	// Find horizontal offsets
	switch(ho) {
		case LEFT :
			left=0;
			break;
		case CENTER :
			left=(ax2-ax1+1-width)/2;
			break;
		case RIGHT :
			left=ax2-ax1+1-width;
			break;
	}

	// Find vertical offsets
	switch(vo) {
		case TOP :
			top=0;
			break;
		case CENTER :
			top=(ay2-ay1+1-height)/2;
			break;
		case BOTTOM :
			top=ay2-ay1+1-height;
			break;
	}

	// Set clipping area
	setviewport(bx1,by1,bx2,by2);

	// Display icon
	mhide();

	count=1;
	for(yl=y+(ay1-by1+top);yl<y+(ay1-by1+top+height);yl++)
		for(xl=x+(ax1-bx1+left);xl<x+(ax1-bx1+left+width);xl++)
			if(data[++count])
				putpixel(xl,yl,data[count]-1);

	mshow();

	// Reset clipping area
	setviewport();
}


// Make a box 'zoom' from one place to another
void zoombox(int ax1,int ay1,int ax2,int ay2,
			 int bx1,int by1,int bx2,int by2) {
	// Variables
	float left,right,top,bottom,x1,y1,x2,y2,steps=50;
	int loop;

	// Find step values
	left=(bx1-ax1)/steps;
	right=(bx2-ax2)/steps;
	top=(by1-ay1)/steps;
	bottom=(by2-ay2)/steps;

	// Set initial box
	x1=ax1;
	y1=ay1;
	x2=ax2;
	y2=ay2;

	// Hide mouse
	mhide();

	// Set write mode and color
	setcolor(RED);
	setwritemode(XOR_PUT);

	// Draw first wave of boxes
	for(loop=0;loop<steps;loop++) {
		rectangle(x1,y1,x2,y2);

		x1+=left;
		y1+=top;
		x2+=right;
		y2+=bottom;
	}

	// Reset initial box
	x1=ax1;
	y1=ay1;
	x2=ax2;
	y2=ay2;

	// Clear first wave of boxes
	for(loop=0;loop<steps;loop++) {
		rectangle(x1,y1,x2,y2);

		x1+=left;
		y1+=top;
		x2+=right;
		y2+=bottom;
	}

	// Show mouse
	mshow();

	// Restore write mode
	setwritemode(COPY_PUT);
}


// Clear screen with a 'shrink' effect
void clrscr(char color) {
	// Variables
	float left,right,top,bottom,x1,y1,x2,y2,steps;
	int loop;

	// Find step values
	steps=(desktop.width/2)/5;
	left=(desktop.width/2)/steps;
	right=-left;
	top=(desktop.height/2)/steps;
	bottom=-top;

	// Set initial box
	x1=0;
	y1=0;
	x2=desktop.width+1;
	y2=desktop.height;

	// Hide mouse
	mhide();

	// Set write mode and color
	setfillstyle(SOLID_FILL,color);

	// Draw first wave of boxes
	for(loop=0;loop<steps;loop++) {
		x1+=left;
		y1+=top;
		x2+=right;
		y2+=bottom;

		bar(x1-left,y1-top,x2-right,y1);
		bar(x1-left,y1-top,x1,y2-bottom);
		bar(x1-left,y2,x2-right,y2-bottom);
		bar(x2,y1-top,x2-right,y2-bottom);
	}

	// Show mouse
	mshow();
}


// Display title box
void drawtitle(int winnum) {
	// Variables
	int x1,y1,x2,num;

	// Get area
	x1=area[win[winnum].areaptr].x1;
	y1=area[win[winnum].areaptr].y1;
	x2=area[win[winnum].areaptr].x2;

	// Set fill for title
	setfillstyle(SOLID_FILL,(winnum==order[1])?BLUE:DARKGRAY);

	// Hide mouse
	mhide();

	// Draw title box
	bar(x1+3,y1+3,x2-3,y1+16);

	// Draw window controls and alter clipping area for title text
	num=0;
	if(win[winnum].attrib&CLOSE)
		controlicon(winnum,CLOSE,++num,0);
	if(win[winnum].attrib&MAXIMIZE)
		controlicon(winnum,MAXIMIZE,++num,0);
	if(win[winnum].attrib&MINIMIZE)
		controlicon(winnum,MINIMIZE,++num,0);

	// Display caption
	x2-=4+(12*num);

	setcolor((winnum==order[1])?WHITE:LIGHTGRAY);

	setsmallfont();
	output(0,0,win[winnum].caption,
		   x1+7,y1+3,x2,y1+14,
		   x1+7,y1+3,x2,y1+14,
		   LEFT,CENTER);
	restorefont();

	// Show mouse
	mshow();
}


// Check to see if window is completly covered by another
char covered(int winnum) {
	// Variables
	int check,
		x1,y1,x2,y2;

	// Get window area
	x1=area[win[winnum].areaptr].x1;
	y1=area[win[winnum].areaptr].y1;
	x2=area[win[winnum].areaptr].x2;
	y2=area[win[winnum].areaptr].y2;

	// Only check windows with higher priority
	check=0;

	while(order[++check]!=winnum) {
		// Return true if completely covered
		if(x1>=area[win[order[check]].areaptr].x1 &&
		   x1<=area[win[order[check]].areaptr].x2 &&
		   x2>=area[win[order[check]].areaptr].x1 &&
		   x2<=area[win[order[check]].areaptr].x2 &&
		   y1>=area[win[order[check]].areaptr].y1 &&
		   y1<=area[win[order[check]].areaptr].y2 &&
		   y2>=area[win[order[check]].areaptr].y1 &&
		   y2<=area[win[order[check]].areaptr].y2)
			return(1);
	}

	// None found, return false
	return(0);
}


// Draw a label
void drawlabel(int num) {
	// Variables
	int ax1,ay1,ax2,ay2,
		bx1,by1,bx2,by2;

	// Get clipping area
	bx1=area[labelinfo[num].clip].x1;
	by1=area[labelinfo[num].clip].y1;
	bx2=area[labelinfo[num].clip].x2;
	by2=area[labelinfo[num].clip].y2;

	// Get drawing area
	ax1=area[labelinfo[num].areaptr].x1+bx1;
	ay1=area[labelinfo[num].areaptr].y1+by1;
	ax2=area[labelinfo[num].areaptr].x2+bx1;
	ay2=area[labelinfo[num].areaptr].y2+by1;

	// Return if not visible
	if(by2<ay1)
		return;

	// Set text color
	setfillstyle(SOLID_FILL,labelinfo[num].back);
	setcolor(labelinfo[num].text);

	// Set viewport
	if(!setviewport(labelinfo[num].clip))
		return;

	// Draw backgound
	if(labelinfo[num].back!=NONE) {
		mhide();
		bar(ax1-bx1,ay1-by1,ax2-bx1,ay2-by1);
		mshow();
	}

	// Reset viewport
	setviewport();

	// Display text
	output(0,0,labelinfo[num].caption,
		   ax1,ay1,ax2,ay2,
		   bx1,by1,bx2,by2,
		   labelinfo[num].hal,labelinfo[num].val);
}


// Draw a button
void drawbutton(int num,char pressed) {
	// Variables
	int ax1,ay1,ax2,ay2,
		bx1,by1,bx2,by2;

	// Get clipping area
	bx1=area[buttoninfo[num].clip].x1;
	by1=area[buttoninfo[num].clip].y1;
	bx2=area[buttoninfo[num].clip].x2;
	by2=area[buttoninfo[num].clip].y2;

	// Get drawing area
	ax1=area[buttoninfo[num].areaptr].x1;
	ay1=area[buttoninfo[num].areaptr].y1;
	ax2=area[buttoninfo[num].areaptr].x2;
	ay2=area[buttoninfo[num].areaptr].y2;

	// Return if not visible
	if(by2<ay1+by1)
		return;

	// Set viewport
	if(!setviewport(buttoninfo[num].clip))
		return;

	// Draw normal button ...
	if(!pressed) {
		// Hide mouse
		mhide();

		// Draw main button
		setfillstyle(SOLID_FILL,(buttoninfo[num].color&0xf0)>>4);
		bar(ax1+1,ay1+1,ax2-2,ay2-2);

		// Draw 3D bits
		setcolor(WHITE);
		line(ax1,ay1,ax2,ay1);
		line(ax1,ay1,ax1,ay2-1);

		setcolor(BLACK);
		line(ax2,ay1,ax2,ay2);
		line(ax2,ay2,ax1,ay2);

		setcolor(DARKGRAY);
		line(ax2-1,ay1+1,ax2-1,ay2-1);
		line(ax1+1,ay2-1,ax2-1,ay2-1);

		// Set text color
		if(buttoninfo[num].enabled)
			setcolor(buttoninfo[num].color&0x0f);

		// Display text
		output(0,0,buttoninfo[num].caption,
			   bx1+ax1+1,by1+ay1+1,bx1+ax2-2,by1+ay2-2,
			   bx1,by1,bx2,by2,
			   CENTER,CENTER);

		// Show mouse
		mshow();
	} else {
		// ... otherwise make the button indented
		// Hide mouse
		mhide();

		// Draw main button
		setfillstyle(SOLID_FILL,(buttoninfo[num].color&0xf0)>>4);
		bar(ax1+2,ay1+2,ax2-1,ay2-1);

		// Draw 3D bits
		setcolor(BLACK);
		rectangle(ax1,ay1,ax2,ay2);

		setcolor(DARKGRAY);
		line(ax1+1,ay1+1,ax2-1,ay1+1);
		line(ax1+1,ay1+1,ax1+1,ay2-1);

		// Set text color
		if(buttoninfo[num].enabled)
			setcolor(buttoninfo[num].color&0x0f);

		// Display text
		output(1,1,buttoninfo[num].caption,
			   bx1+ax1+1,by1+ay1+1,bx1+ax2-2,by1+ay2-2,
			   bx1,by1,bx2,by2,
			   CENTER,CENTER);

		// Show mouse
		mshow();
	}

	// Reset viewport
	setviewport();
}


// Draw a control icon
void controlicon(int winnum,char type,int offset,char pressed) {
	// Variables
	int x1,y1,x2,y2,*data;

	// Get drawing area
	x1=area[win[winnum].areaptr].x2-2-(offset*12);
	y1=area[win[winnum].areaptr].y1+4;
	y2=y1+11;
	x2=x1+11;

	// Return if off screen
	if(x1>desktop.width || y1>desktop.height)
		return;

	// Set data pointer
	switch(type) {
		case CLOSE :
			data=closeicon;
			break;
		case MINIMIZE :
			data=minimizeicon;
			break;
		case MAXIMIZE :
			data=(win[winnum].state==MAXIMIZE)?restoreicon:maximizeicon;
			break;
	}

	// Draw normal button ...
	if(!pressed) {
		// Hide mouse
		mhide();

		// Draw main button
		setfillstyle(SOLID_FILL,LIGHTGRAY);
		bar(x1+1,y1+1,x2-2,y2-2);

		// Draw 3D bits
		setcolor(WHITE);
		line(x1,y1,x2,y1);
		line(x1,y1,x1,y2-1);

		setcolor(BLACK);
		line(x2,y1,x2,y2);
		line(x2,y2,x1,y2);

		setcolor(DARKGRAY);
		line(x2-1,y1+1,x2-1,y2-1);
		line(x1+1,y2-1,x2-1,y2-1);

		// Display icon
		output(0,0,data,
			   x1+1,y1+1,x2-2,y2-2,
			   x1+1,y1+1,x2-2,y2-2,
			   CENTER,CENTER);

		// Show mouse
		mshow();
	} else {
		// ... otherwise make the button indented
		// Hide mouse
		mhide();

		// Draw main button
		setfillstyle(SOLID_FILL,LIGHTGRAY);
		bar(x1+2,y1+2,x2-1,y2-1);

		// Draw 3D bits
		setcolor(BLACK);
		rectangle(x1,y1,x2,y2);

		setcolor(DARKGRAY);
		line(x1+1,y1+1,x2-1,y1+1);
		line(x1+1,y1+1,x1+1,y2-1);

		// Display text
		output(1,1,data,
			   x1+1,y1+1,x2-2,y2-2,
			   x1+1,y1+1,x2-2,y2-2,
			   CENTER,CENTER);

		// Show mouse
		mshow();
	}
}


// Draw an icon button
void drawiconbutton(int num,char pressed) {
	// Variables
	int ax1,ay1,ax2,ay2,
		bx1,by1,bx2,by2;

	// Get clipping area
	bx1=area[buttoninfo[num].clip].x1;
	by1=area[buttoninfo[num].clip].y1;
	bx2=area[buttoninfo[num].clip].x2;
	by2=area[buttoninfo[num].clip].y2;

	// Get drawing area
	ax1=area[buttoninfo[num].areaptr].x1;
	ay1=area[buttoninfo[num].areaptr].y1;
	ax2=area[buttoninfo[num].areaptr].x2;
	ay2=area[buttoninfo[num].areaptr].y2;

	// Return if not visible
	if(by2<ay1+by1)
		return;

	// Set viewport
	if(!setviewport(buttoninfo[num].clip))
		return;

	// Draw normal button ...
	if(!pressed) {
		// Hide mouse
		mhide();

		// Draw main button
		setfillstyle(SOLID_FILL,buttoninfo[num].color);
		bar(ax1+1,ay1+1,ax2-2,ay2-2);

		// Draw 3D bits
		setcolor(WHITE);
		line(ax1,ay1,ax2,ay1);
		line(ax1,ay1,ax1,ay2-1);

		setcolor(BLACK);
		line(ax2,ay1,ax2,ay2);
		line(ax2,ay2,ax1,ay2);

		setcolor(DARKGRAY);
		line(ax2-1,ay1+1,ax2-1,ay2-1);
		line(ax1+1,ay2-1,ax2-1,ay2-1);

		// Display icon
		output(0,0,buttoninfo[num].data,
			   bx1+ax1+1,by1+ay1+1,bx1+ax2-2,by1+ay2-2,
			   bx1,by1,bx2,by2,
			   CENTER,CENTER);

		// Show mouse
		mshow();
	} else {
		// ... otherwise make the button indented
		// Hide mouse
		mhide();

		// Draw main button
		setfillstyle(SOLID_FILL,buttoninfo[num].color);
		bar(ax1+2,ay1+2,ax2-1,ay2-1);

		// Draw 3D bits
		setcolor(BLACK);
		rectangle(ax1,ay1,ax2,ay2);

		setcolor(DARKGRAY);
		line(ax1+1,ay1+1,ax2-1,ay1+1);
		line(ax1+1,ay1+1,ax1+1,ay2-1);

		// Display text
		output(1,1,buttoninfo[num].data,
			   bx1+ax1+1,by1+ay1+1,bx1+ax2-2,by1+ay2-2,
			   bx1,by1,bx2,by2,
			   CENTER,CENTER);

		// Show mouse
		mshow();
	}

	// Reset viewport
	setviewport();
}


// Draw objects in a window
void drawobjectsforwindow(int winnum) {
	// Cycle through all elements
	for(int loop=1;loop<MOBJECTS;loop++)
		// Draw if used
		if(win[winnum].objects[loop][0])
			switch(win[winnum].objects[loop][0]) {
				case LABEL :
					drawlabel(win[winnum].objects[loop][1]);
					break;
				case HSCROLL :
					drawhscroll(win[winnum].objects[loop][1]);
					break;
				case VSCROLL :
					drawvscroll(win[winnum].objects[loop][1]);
					break;
				case BUTTON :
					drawbutton(win[winnum].objects[loop][1],0);
					break;
				case ICONBUTTON :
					drawiconbutton(win[winnum].objects[loop][1],0);
					break;
			}
}


// Display window
void drawwindow(int winnum) {
	// Variables
	int x1,y1,x2,y2;

	// Return if window is undefined
	if(!win[winnum].areaptr)
		return;

	// Return if window is completely covered (Pointless in drawing it)
	if(covered(winnum))
		return;

	// Get window position
	x1=area[win[winnum].areaptr].x1;
	y1=area[win[winnum].areaptr].y1;
	x2=area[win[winnum].areaptr].x2;
	y2=area[win[winnum].areaptr].y2;

	// Make sure viewport is set to full screen
	setviewport();

	// Hide mouse
	mhide();

	// Draw main box
	setfillstyle(SOLID_FILL,win[winnum].back);
	bar(x1+3,y1+18,x2-3,y2-3);

	// Draw bits around main window
	setcolor(LIGHTGRAY);
	rectangle(x1+2,y1+17,x2-2,y2-2);
	line(x1,y1+17,x1,y2);

	// Surrounding title box bits
	setfillstyle(SOLID_FILL,LIGHTGRAY);
	bar(x1,y1,x2,y1+2);
	bar(x1,y1+3,x1+2,y1+16);
	bar(x2-2,y1+3,x2,y1+16);

	// Draw 3D bits
	setcolor(BLACK);
	line(x2,y1,x2,y2);
	line(x2,y2,x1,y2);

	setcolor(DARKGRAY);
	line(x2-1,y1+1,x2-1,y2-1);
	line(x2-1,y2-1,x1+1,y2-1);

	setcolor(WHITE);
	line(x2-2,y1+1,x1+1,y1+1);
	line(x1+1,y1+1,x1+1,y2-2);

	// Draw resize indicator
	if(win[winnum].attrib&RESIZE && win[winnum].state==NORMAL) {
		setcolor(DARKGRAY);
		line(x2-2,y2-5,x2-2,y2-2);
		line(x2-5,y2-2,x2-2,y2-2);
	}

	// Show mouse
	mshow();

	// Draw title
	drawtitle(winnum);

	// Update clipping area
	area[win[winnum].clip].x1=x1+3;
	area[win[winnum].clip].y1=y1+18;
	area[win[winnum].clip].x2=x2-3;
	area[win[winnum].clip].y2=y2-3;

	// Call redraw function
	if(win[winnum].redraw)
		win[winnum].redraw();

	// Draw objects
	drawobjectsforwindow(winnum);
}


// Display all windows on screen
void drawall(void) {
	// Clear background
	mhide();
	setfillstyle(desktop.pattern,desktop.color);
	bar(0,0,desktop.width,desktop.height);
	mshow();

	// Draw windows
	for(int loop=maxwin;loop>0;loop--)
		drawwindow(order[loop]);
}


// Do two areas overlap?
char overlapcheck(int ax1,int ay1,int ax2,int ay2,
				  int bx1,int by1,int bx2,int by2) {
	if(ay1>by2) return(0);
	if(ay2<by1) return(0);
	if(ax2<bx1) return(0);
	if(ax1>bx2) return(0);

	return(1);
}

char overlap(int x11,int y11,int x12,int y12,
			 int x21,int y21,int x22,int y22) {
	if(overlapcheck(x11,y11,x12,y12,x21,y21,x22,y22))
		return(1);
	if(overlapcheck(x21,y21,x22,y22,x11,y11,x12,y12))
		return(1);
	return(0);
}

char overlap(int area1,int area2) {
	return(overlap(area[area1].x1,area[area1].y1,
				   area[area1].x2,area[area1].y2,
				   area[area2].x1,area[area2].y1,
				   area[area2].x2,area[area2].y2));
}


// Does window overlap any others?
char anyoverlap(int winnum) {
	for(int loop=1;loop<MWINDOWS;loop++)
		if(loop!=winnum)
			if(overlap(win[winnum].areaptr,win[loop].areaptr))
				return(1);

	return(0);
}


// Redraw funtion for new window activation
void drawforactivate(int oldwin) {
	// Make old window inactive
	drawtitle(oldwin);

	// If new window is on its own, just redraw title ...
	if(!anyoverlap(order[1]))
		drawtitle(order[1]);
	else
		// ... otherwise redraw the whole window
		drawwindow(order[1]);
}


// Reset mouse limits
void limitmouse(void) {
	limitmouse(0,0,desktop.width,desktop.height);
}


// Erase remaining sections of window for move (pass new coords)
void erase(int x1,int y1,int x2,int y2) {
	// Variables
	int ox1,oy1,ox2,oy2;
	char corner[4];

	// Store <from> coords
	ox1=area[win[order[1]].areaptr].x1;
	oy1=area[win[order[1]].areaptr].y1;
	ox2=area[win[order[1]].areaptr].x2;
	oy2=area[win[order[1]].areaptr].y2;

	// Set fill mode
	setfillstyle(desktop.pattern,desktop.color);

	// Check which new corners are in old window area
	corner[0]=zone(x1,y1,ox1,oy1,ox2,oy2);
	corner[1]=zone(x2,y1,ox1,oy1,ox2,oy2);
	corner[2]=zone(x2,y2,ox1,oy1,ox2,oy2);
	corner[3]=zone(x1,y2,ox1,oy1,ox2,oy2);

	// If no overlap, erase all of old area
	if(!(corner[0]+corner[1]+corner[2]+corner[3])) {
		mhide();
		bar(ox1,oy1,ox2,oy2);
		mshow();
		return;
	}

	// If slide up/down
	if(x1==ox1) {
		mhide();
		if(y1<oy1)
			bar(ox1,y2,ox2,oy2);
		else
			bar(ox1,oy1,ox2,y1);
		mshow();
		return;
	}

	// If slide left/right
	if(y1==oy1) {
		mhide();
		if(x1<ox1)
			bar(x2,oy1,ox2,oy2);
		else
			bar(ox1,oy1,x1,oy2);
		mshow();
		return;
	}

	// Erase necessary sections of old window
	if(corner[0]) {
		mhide();
		bar(ox1,oy1,ox2,y1);
		bar(ox1,y1,x1,oy2);
		mshow();
		return;
	}
	if(corner[1]) {
		mhide();
		bar(ox1,oy1,ox2,y1);
		bar(x2,y1,ox2,oy2);
		mshow();
		return;
	}
	if(corner[2]) {
		mhide();
		bar(x2,oy1,ox2,oy2);
		bar(ox1,y2,x2,oy2);
		mshow();
		return;
	}
	if(corner[3]) {
		mhide();
		bar(ox1,oy1,x1,oy2);
		bar(x1,y2,ox2,oy2);
		mshow();
		return;
	}
}


// Redraw linked windows (pass old area)
void redraw(int x1,int y1,int x2,int y2) {
	// Variables
	char list[MWINDOWS]={0},update[MWINDOWS]={0};
	int end=0,pointer=0,check,loop;

	// Redraw if there is only one window
	if(maxwin==1) {
		drawwindow(order[1]);
		return;
	}

	// Check window against all others
	do {
		for(check=2;check<=maxwin;check++)
			if(overlap(x1,y1,x2,y2,
					   area[win[order[check]].areaptr].x1,
					   area[win[order[check]].areaptr].y1,
					   area[win[order[check]].areaptr].x2,
					   area[win[order[check]].areaptr].y2))
				if(!update[order[check]]) {
					list[++end]=order[check];
					update[order[check]]=1;
				}

		// Get position of next window
		pointer++;

		x1=area[win[list[pointer]].areaptr].x1;
		y1=area[win[list[pointer]].areaptr].y1;
		x2=area[win[list[pointer]].areaptr].x2;
		y2=area[win[list[pointer]].areaptr].y2;

	} while(pointer<=end+1);

	// Always redraw active window
	update[order[1]]=1;

	// Redraw windows
	for(loop=maxwin;loop>0;loop--)
		if(update[order[loop]])
			drawwindow(order[loop]);
}


// Move window
void movewindow(void) {
	// Variables
	int xoff,yoff,x,y,ox,oy,width,height,ix,iy;

	// Return if not movable
	if(!(win[order[1]].attrib&MOVE) || win[order[1]].state==MAXIMIZE)
		return;

	// Set mouse information
	setmousexy(m.mx,m.my);

	// Get pointer offsets from top-left of window
	xoff=m.mx-area[win[order[1]].areaptr].x1;
	yoff=m.my-area[win[order[1]].areaptr].y1;

	// Limit mouse movement
	limitmouse(xoff,yoff,desktop.width,desktop.height);

	// Reset viewport
	setviewport();

	// Set drawing mode
	setcolor(RED);
	setwritemode(XOR_PUT);

	// Set initial values
	width=area[win[order[1]].areaptr].x2-area[win[order[1]].areaptr].x1;
	height=area[win[order[1]].areaptr].y2-area[win[order[1]].areaptr].y1;
	x=ix=area[win[order[1]].areaptr].x1;
	y=iy=area[win[order[1]].areaptr].y1;

	// Move window loop
	do {
		// Store old outline position
		ox=x;
		oy=y;

		// Draw outline
		mhide();
		rectangle(x,y,x+width,y+height);
		mshow();

		// Wait until mouse if moved
		do {
			mcheck();

			x=m.mx-xoff;
			y=m.my-yoff;
		} while(x==ox && y==oy && m.lmb);

		// Erase old outline
		mhide();
		rectangle(ox,oy,ox+width,oy+height);
		mshow();
	} while(m.lmb);

	// Restore defaults
	setwritemode(COPY_PUT);
	limitmouse();

	// Return if no movement
	if(x==area[win[order[1]].areaptr].x1 &&
	   y==area[win[order[1]].areaptr].y1)
		return;

	// Erase old window
	erase(x,y,x+width,y+height);

	// If no overlap, do simple redraw ...
	if(!anyoverlap(order[1])) {
		// Store new position
		area[win[order[1]].areaptr].x1=x;
		area[win[order[1]].areaptr].y1=y;
		area[win[order[1]].areaptr].x2=x+width;
		area[win[order[1]].areaptr].y2=y+height;

		// Draw window
		drawwindow(order[1]);
	} else {
		// ... otherwise redraw linked windows
		// Store new position
		area[win[order[1]].areaptr].x1=x;
		area[win[order[1]].areaptr].y1=y;
		area[win[order[1]].areaptr].x2=x+width;
		area[win[order[1]].areaptr].y2=y+height;

		// Draw linked windows
		redraw(ix,iy,ix+width,iy+height);
	}
}


// Resize window
void resizewindow(void) {
	// Variables
	int x1,y1,x2,y2,ox2,oy2,ix2,iy2,xoff,yoff;

	// Return if not resizable
	if(!(win[order[1]].attrib&RESIZE) || win[order[1]].state==MAXIMIZE)
		return;

	// Set initial values
	x1=area[win[order[1]].areaptr].x1;
	y1=area[win[order[1]].areaptr].y1;
	x2=ix2=area[win[order[1]].areaptr].x2;
	y2=iy2=area[win[order[1]].areaptr].y2;

	// Set mouse information
	setmousexy(m.mx,m.my);

	// Find offsets from bottom-right corner
	xoff=x2-m.mx;
	yoff=y2-m.my;

	// Limit mouse movement
	limitmouse(x1+100-xoff,y1+19-yoff,desktop.width-xoff,desktop.height-yoff);

	// Reset viewport
	setviewport();

	// Set drawing mode
	setcolor(RED);
	setwritemode(XOR_PUT);

	// Move window loop
	do {
		// Store old outline position
		ox2=x2;
		oy2=y2;

		// Draw outline
		mhide();
		rectangle(x1,y1,x2,y2);
		mshow();

		// Wait until mouse if moved
		do {
			mcheck();

			x2=m.mx+xoff;
			y2=m.my+yoff;
		} while(x2==ox2 && y2==oy2 && m.lmb);

		// Erase old outline
		mhide();
		rectangle(x1,y1,ox2,oy2);
		mshow();

	} while(m.lmb);

	// Restore defaults
	setwritemode(COPY_PUT);
	limitmouse();

	// Return if no movement
	if(x2==ix2 && y2==iy2)
		return;

	// Store new size
	area[win[order[1]].areaptr].x2=x2;
	area[win[order[1]].areaptr].y2=y2;

	// Redraw if new window is bigger ...
	if(x2>=ix2 && y2>=iy2)
		drawwindow(order[1]);
	else {
		// ... otherwise, Set fill style
		setfillstyle(desktop.pattern,desktop.color);

		// Hide mouse
		mhide();

		// Erase sides
		bar(min(ix2,x2),y1,max(ix2,x2),max(iy2,y2));
		bar(x1,min(iy2,y2),max(ix2,x2),max(iy2,y2));

		// Show mouse
		mshow();

		// Redraw necessary windows
		redraw(x1,y1,ix2,iy2);
	}
}


// Add a label
int label(int winnum,char *caption,
		  int x1,int y1,int x2,int y2,
		  char text,char back,
		  char hal,char val) {

	// Variables
	int num,areaptr,objectptr;

	// Add label to area structure, return if error
	areaptr=addarea(x1,y1,x2,y2);
	if(!areaptr)
		return(0);

	// Check clip area
	if(!area[win[winnum].clip].x2)
		return(0);

	// Store label data, return if error
	num=labelslot(areaptr,win[winnum].clip,text,back,hal,val);
	if(!num)
		return(0);

	// Allocate space for caption, quit if error
	if(strlen(caption)) {
		labelinfo[num].caption=(char *) malloc(strlen(caption)+1);
		if(!labelinfo[num].caption) {
			shutdown();
			puts("Error allocating caption memory in LABEL");
			exit(0);
		}

		strcpy(labelinfo[num].caption,caption);
	}

	// Add label to object array, return if error
	objectptr=objectslot(winnum,LABEL,num);
	if(!objectptr)
		return(0);

	// Return label number
	return(num);
}


// Add a button
int button(int winnum,char *caption,
		   int x1,int y1,int x2,int y2,
		   char text,char back,
		   void (*onclick)(int)) {

	// Variables
	int num,areaptr,objectptr;

	// Add button to area structure, return if error
	areaptr=addarea(x1,y1,x2,y2);
	if(!areaptr)
		return(0);

	// Check clip area
	if(!area[win[winnum].clip].x2)
		return(0);

	// Store button data, return if error
	num=buttonslot(areaptr,win[winnum].clip,(back<<4)+text,onclick);
	if(!num)
		return(0);

	// Allocate space for caption, quit if error
	if(strlen(caption)) {
		buttoninfo[num].caption=(char *) malloc(strlen(caption)+1);
		if(!buttoninfo[num].caption) {
			shutdown();
			puts("Error allocating caption memory in BUTTON");
			exit(0);
		}

		strcpy(buttoninfo[num].caption,caption);
	}

	// Add button to object array, return if error
	objectptr=objectslot(winnum,BUTTON,num);
	if(!objectptr)
		return(0);

	// Return button number
	return(num);
}


// Add an icon button
int button(int winnum,int *data,
		   int x1,int y1,int x2,int y2,
		   char back,
		   void (*onclick)(int)) {

	// Variables
	int num,areaptr,objectptr,loop;

	// Add button to area structure, return if error
	areaptr=addarea(x1,y1,x2,y2);
	if(!areaptr)
		return(0);

	// Check clip area
	if(!area[win[winnum].clip].x2)
		return(0);

	// Store button data, return if error
	num=buttonslot(areaptr,win[winnum].clip,back,onclick);
	if(!num)
		return(0);

	// Allocate space for data, quit if error
	buttoninfo[num].data=new int[(data[0]*data[1])+2+1];
	if(!buttoninfo[num].data) {
		shutdown();
		puts("Error allocating icon memory in BUTTON");
		exit(0);
	}

	// Store icon data
	for(loop=0;loop<(data[0]*data[1])+2;loop++)
		buttoninfo[num].data[loop]=data[loop];

	// Add button to object array, return if error
	objectptr=objectslot(winnum,ICONBUTTON,num);
	if(!objectptr)
		return(0);

	// Return button number
	return(num);
}


// Check for control clicks or move window
void checkcontrols(void) {
	// Variables
	int count,x1,y1,type,pressed;

	// Count controls
	count=0;
	if(win[order[1]].attrib&CLOSE)
		count++;
	if(win[order[1]].attrib&MAXIMIZE)
		count++;
	if(win[order[1]].attrib&MINIMIZE)
		count++;

	// Get control button area
	x1=area[win[order[1]].areaptr].x2-2-(count*12);
	y1=area[win[order[1]].areaptr].y1+4;

	// If not on a control, move window
	if(!zone(m.mx,m.my,x1,y1,area[win[order[1]].areaptr].x2-2,y1+11)) {
		movewindow();
		return;
	}

	// Find which area has been clicked on
	x1=area[win[order[1]].areaptr].x2-38;

	if(zone(m.mx,m.my,x1,y1,x1+11,y1+11)) {
		count=3;
		type=MINIMIZE;
	} else
		if(zone(m.mx,m.my,x1+12,y1,x1+23,y1+11)) {
			count=2;
			x1+=12;
			if(!(win[order[1]].attrib&CLOSE))
				type=MINIMIZE;
			else
				type=MAXIMIZE;
		} else {
			count=1;
			x1+=24;
			if(win[order[1]].attrib&MINIMIZE)
				type=MINIMIZE;
			if(win[order[1]].attrib&MAXIMIZE)
				type=MAXIMIZE;
			if(win[order[1]].attrib&CLOSE)
				type=CLOSE;
		}

	// Indent button whilst pressed
	pressed=0;

	do {
		// Update mouse
		mcheck();

		// If on button ...
		if(zone(m.mx,m.my,x1,y1,x1+11,y1+11)) {
			// ... press if not pressed
			if(!pressed) {
				// Draw indent button
				controlicon(order[1],type,count,1);

				// Make pressed
				pressed=1;
			}
		} else
			// If not on button, release it
			if(pressed) {
				// Draw normal button
				controlicon(order[1],type,count,0);

				// Make released
				pressed=0;
			}
	} while(m.lmb);

	// Return if not pressed
	if(!pressed)
		return;

	// Release button
	controlicon(order[1],type,count,0);

	// Call whatever function is necessary
	switch(type) {
		case MINIMIZE :
			minimize();
			break;
		case MAXIMIZE :
			if(win[order[1]].state==MAXIMIZE)
				restore();
			else
				maximize();
			break;
		case CLOSE :
			close();
	}
}


// Close a window
void close(void) {
	// Variables
	int x1,y1,x2,y2;

	// Store area
	x1=area[win[order[1]].areaptr].x1;
	y1=area[win[order[1]].areaptr].y1;
	x2=area[win[order[1]].areaptr].x2;
	y2=area[win[order[1]].areaptr].y2;

	// Delete window element
	delwindow(order[1]);

	// Delete window from screen
	setfillstyle(desktop.pattern,desktop.color);
	mhide();
	bar(x1,y1,x2,y2);
	mshow();

	// Redraw touching windows
	redraw(x1,y1,x2,y2);
}


// Minimize a window
void minimize(void) {
	// Variables
	int x1,y1,x2,y2;

	// Store area
	x1=area[win[order[1]].areaptr].x1;
	y1=area[win[order[1]].areaptr].y1;
	x2=area[win[order[1]].areaptr].x2;
	y2=area[win[order[1]].areaptr].y2;

	// Store old size
	restoreinfo[order[1]].x1=x1;
	restoreinfo[order[1]].y1=y1;
	restoreinfo[order[1]].x2=x2;
	restoreinfo[order[1]].y2=y2;

	// Set state is minimized
	win[order[1]].state=MINIMIZE;

	// Delete window from screen
	setfillstyle(desktop.pattern,desktop.color);
	mhide();
	bar(x1,y1,x2,y2);
	mshow();

	// Delete from order list
	delchar(order,1);

	// Redraw touching windows
	redraw(x1,y1,x2,y2);
}


// Maximize a window
void maximize(void) {
	// Set state to maximized
	win[order[1]].state=MAXIMIZE;

	// Store old size
	restoreinfo[order[1]].x1=area[win[order[1]].areaptr].x1;
	restoreinfo[order[1]].y1=area[win[order[1]].areaptr].y1;
	restoreinfo[order[1]].x2=area[win[order[1]].areaptr].x2;
	restoreinfo[order[1]].y2=area[win[order[1]].areaptr].y2;

	// 'Zoom' box
	zoombox(restoreinfo[order[1]].x1,restoreinfo[order[1]].y1,
			restoreinfo[order[1]].x2,restoreinfo[order[1]].y2,
			0,0,desktop.width,desktop.height);

	// Store new size
	area[win[order[1]].areaptr].x1=0;
	area[win[order[1]].areaptr].y1=0;
	area[win[order[1]].areaptr].x2=desktop.width;
	area[win[order[1]].areaptr].y2=desktop.height;

	// Redraw window
	drawwindow(order[1]);
}


// Restore a window
void restore(void) {
	// Variables
	int x1,y1,x2,y2;

	// Retrieve old area
	x1=restoreinfo[order[1]].x1;
	y1=restoreinfo[order[1]].y1;
	x2=restoreinfo[order[1]].x2;
	y2=restoreinfo[order[1]].y2;

	// 'Zoom' box
	zoombox(0,0,desktop.height,desktop.width,
			x1,y1,x2,y2);

	// Store as new area
	area[win[order[1]].areaptr].x1=x1;
	area[win[order[1]].areaptr].y1=y1;
	area[win[order[1]].areaptr].x2=x2;
	area[win[order[1]].areaptr].y2=y2;

	// Set state to normal
	win[order[1]].state=NORMAL;

	// Redraw windows
	drawall();
}


// Return true if mouse in on the titlebox
char ontitlebox(void) {
	return(zone(m.mx,m.my,
			area[win[order[1]].areaptr].x1+3,
			area[win[order[1]].areaptr].y1+3,
			area[win[order[1]].areaptr].x2-3,
			area[win[order[1]].areaptr].y1+16));
}


// Return true if mouse in on the resize selector
char onresizer(void) {
	return(zone(m.mx,m.my,
				area[win[order[1]].areaptr].x2-5,
				area[win[order[1]].areaptr].y2-5,
				area[win[order[1]].areaptr].x2,
				area[win[order[1]].areaptr].y2));
}


// Check to see if mouse is on another window
void checkwindows(void) {
	// Variables
	int loop,oldwin,temp;

	// If not on current window...
	if(!zone(m.mx,m.my,win[order[1]].areaptr))

		// Check other windows according to priority
		for(loop=2;loop<MWINDOWS;loop++)
			if(zone(m.mx,m.my,win[order[loop]].areaptr)) {
				// Store old active window
				oldwin=order[1];

				// Make new window active
				temp=order[loop];
				delchar(order,loop);
				stuff(order,1,temp);

				// Redraw necessary windows
				drawforactivate(oldwin);

				// Quit loop
				return;
			}
}


// Function to alter value for spinner
void spinchange(int num) {
	// Variables
	int elem,old;
	char string[6];

	// Find spinner element
	elem=spintable[num].spinptr;

	// Change value of variable
	old=(*spininfo[elem].value);
	(*spininfo[elem].value)+=spintable[num].inc;

	// Check range
	(*spininfo[elem].value)=max((*spininfo[elem].value),spininfo[elem].low);
	(*spininfo[elem].value)=min((*spininfo[elem].value),spininfo[elem].high);

	// Return if no change
	if((*spininfo[elem].value)==old)
		return;

	// Copy value to label caption
	sprintf(string,"%d",(*spininfo[elem].value));

	free(labelinfo[spininfo[elem].labelptr].caption);
	labelinfo[spininfo[elem].labelptr].caption=(char *) malloc(6);
	strcpy(labelinfo[spininfo[elem].labelptr].caption,string);

	// Update label on screen
	drawlabel(spininfo[elem].labelptr);

	// Call onclick function
	if(spininfo[elem].onclick)
		spininfo[elem].onclick(elem);
}


// Spinner
int spinner(int winnum,int x1,int y1,int width,int *value,int low,int high,
			void (*onclick)(int)) {
	// Variables
	int up,down,box,halfy,left,num,objectptr;
	char string[6];

	// Return if no more room in spinner structure
	num=spinslot();
	if(!num)
		return(0);

	// Store data
	spininfo[num].x1=x1;
	spininfo[num].y1=y1;
	spininfo[num].width=width;

	spininfo[num].used=1;
	spininfo[num].high=high;
	spininfo[num].low=low;
	spininfo[num].value=value;
	spininfo[num].onclick=onclick;

	// Work out where spinner is
	halfy=spininfo[num].y1+(font.height/2);
	left=spininfo[num].x1+(textwidth("8")*spininfo[num].width)+2;

	// Create buttons
	up=button(winnum,
			  upicon,
			  left+2,halfy-7,left+11,halfy,
			  LIGHTGRAY,
			  spinchange);
	if(!up)
		return(0);

	down=button(winnum,
			  downicon,
			  left+2,halfy+1,left+11,halfy+8,
			  LIGHTGRAY,
			  spinchange);
	if(!down)
		return(0);

	// Create label
	sprintf(string,"%d",(*value));
	box=label(winnum,
			  string,
			  x1,y1,left,y1+font.height,
			  BLACK,WHITE,
			  RIGHT,CENTER);
	if(!box)
		return(0);

	// Store label pointer
	spininfo[num].labelptr=box;

	// Store function information
	spintable[up].spinptr=num;
	spintable[up].inc=1;
	spintable[down].spinptr=num;
	spintable[down].inc=-1;

	// Return element
	return(num);
}


// Alter value for horizontal scroll bar
void hscrollchange(int num) {
	// Variables
	int elem,old;

	// Find scroll bar element
	elem=hscrolltable[num].ptr;

	// Change value of variable
	old=(*hscrollinfo[elem].value);
	(*hscrollinfo[elem].value)+=hscrolltable[num].inc;

	// Check range
	(*hscrollinfo[elem].value)=max((*hscrollinfo[elem].value),0);
	(*hscrollinfo[elem].value)=min((*hscrollinfo[elem].value),hscrollinfo[elem].high);

	// Return if no change
	if((*hscrollinfo[elem].value)==old)
		return;

	// Update scroll bar on screen
	drawhscroll(elem);

	// Call onclick function
	if(hscrollinfo[elem].onclick)
		hscrollinfo[elem].onclick(elem);
}


// Draw a horizontal scroll bar
void drawhscroll(int num) {
	// Variables
	int x1,y1,x2,y2,x;

	// Get size
	x1=area[hscrollinfo[num].areaptr].x1;
	y1=area[hscrollinfo[num].areaptr].y1;
	x2=area[hscrollinfo[num].areaptr].x2;
	y2=y1+9;

	// Set clipping area to parent window
	setviewport(hscrollinfo[num].clip);

	// Hide mouse
	mhide();

	// Draw scroll bar
	setfillstyle(SOLID_FILL,DARKGRAY);
	bar(x1+10,y1,x2-10,y2);

	// Show mouse
	mshow();

	// Update progress indicator
	x=(x2-x1-29)*((*hscrollinfo[num].value)/(float)(hscrollinfo[num].high));

	area[buttoninfo[hscrollinfo[num].mbut].areaptr].x1=x1+10+x;
	area[buttoninfo[hscrollinfo[num].mbut].areaptr].x2=x1+x+19;

	// Re-draw button
	drawbutton(hscrollinfo[num].mbut,0);

	// Restore viewport
	setviewport();
}


// Create a horizontal scroll box
int hscrollbox(int winnum,int x1,int y1,int x2,
			   int *value,int high,
			   void (*onclick)(int)) {
	// Variables
	int areaptr,num,objectptr,lbut,rbut,mbut;

	// Add button to area structure, return if error
	areaptr=addarea(x1,y1,x2,y1+9);
	if(!areaptr)
		return(0);

	// Check clip area
	if(!area[win[winnum].clip].x2)
		return(0);

	// Get element slot
	num=hscrollslot();
	if(!num)
		return(0);

	// Store data in element
	hscrollinfo[num].areaptr=areaptr;
	hscrollinfo[num].clip=win[winnum].clip;
	hscrollinfo[num].x1=x1;
	hscrollinfo[num].y1=y1;
	hscrollinfo[num].x2=x2;
	hscrollinfo[num].value=value;
	hscrollinfo[num].high=high;
	hscrollinfo[num].onclick=onclick;

	// Add scroll bar to object array, return if error
	objectptr=objectslot(winnum,HSCROLL,num);
	if(!objectptr)
		return(0);

	// Add buttons
	lbut=button(winnum,
				lefticon,
				x1,y1,x1+9,y1+9,
				LIGHTGRAY,
				hscrollchange);
	if(!lbut)
		return(0);

	rbut=button(winnum,
				righticon,
				x2-9,y1,x2,y1+9,
				LIGHTGRAY,
				hscrollchange);
	if(!rbut)
		return(0);

	mbut=button(winnum,
				"",
				x1+10,y1,x1+19,y1+9,
				LIGHTGRAY,LIGHTGRAY,
				NULL);
	if(!rbut)
		return(0);
	buttoninfo[mbut].enabled=0;

	// Store buttons
	hscrollinfo[num].lbut=lbut;
	hscrollinfo[num].rbut=rbut;
	hscrollinfo[num].mbut=mbut;

	// Create link between buttons and value changing function
	hscrolltable[lbut].ptr=num;
	hscrolltable[lbut].inc=-1;
	hscrolltable[rbut].ptr=num;
	hscrolltable[rbut].inc=1;

	// Return button number
	return(num);
}


// Alter value for vertical scroll bar
void vscrollchange(int num) {
	// Variables
	int elem,old;

	// Find scroll bar element
	elem=vscrolltable[num].ptr;

	// Change value of variable
	old=(*vscrollinfo[elem].value);
	(*vscrollinfo[elem].value)+=vscrolltable[num].inc;

	// Check range
	(*vscrollinfo[elem].value)=max((*vscrollinfo[elem].value),0);
	(*vscrollinfo[elem].value)=min((*vscrollinfo[elem].value),vscrollinfo[elem].high);

	// Return if no change
	if((*vscrollinfo[elem].value)==old)
		return;

	// Update scroll bar on screen
	drawvscroll(elem);

	// Call onclick function
	if(vscrollinfo[elem].onclick)
		vscrollinfo[elem].onclick(elem);
}


// Draw a horizontal scroll bar
void drawvscroll(int num) {
	// Variables
	int x1,y1,x2,y2,y;

	// Get size
	x1=area[vscrollinfo[num].areaptr].x1;
	y1=area[vscrollinfo[num].areaptr].y1;
	x2=x1+9;
	y2=area[vscrollinfo[num].areaptr].y2;

	// Set clipping area to parent window
	setviewport(vscrollinfo[num].clip);

	// Hide mouse
	mhide();

	// Draw scroll bar
	setfillstyle(SOLID_FILL,DARKGRAY);
	bar(x1,y1+10,x2,y2-10);

	// Show mouse
	mshow();

	// Update progress indicator
	y=(y2-y1-29)*((*vscrollinfo[num].value)/(float)(vscrollinfo[num].high));

	area[buttoninfo[vscrollinfo[num].mbut].areaptr].y1=y1+10+y;
	area[buttoninfo[vscrollinfo[num].mbut].areaptr].y2=y1+y+19;

	// Re-draw button
	drawbutton(vscrollinfo[num].mbut,0);

	// Restore viewport
	setviewport();
}


// Create a horizontal scroll box
int vscrollbox(int winnum,int x1,int y1,int y2,
			   int *value,int high,
			   void (*onclick)(int)) {
	// Variables
	int areaptr,num,objectptr,lbut,rbut,mbut;

	// Add button to area structure, return if error
	areaptr=addarea(x1,y1,x1+9,y2);
	if(!areaptr)
		return(0);

	// Check clip area
	if(!area[win[winnum].clip].x2)
		return(0);

	// Get element slot
	num=vscrollslot();
	if(!num)
		return(0);

	// Store data in element
	vscrollinfo[num].areaptr=areaptr;
	vscrollinfo[num].clip=win[winnum].clip;
	vscrollinfo[num].x1=x1;
	vscrollinfo[num].y1=y1;
	vscrollinfo[num].y2=y2;
	vscrollinfo[num].value=value;
	vscrollinfo[num].high=high;
	vscrollinfo[num].onclick=onclick;

	// Add scroll bar to object array, return if error
	objectptr=objectslot(winnum,VSCROLL,num);
	if(!objectptr)
		return(0);

	// Add buttons
	lbut=button(winnum,
				downicon,
				x1,y2-9,x1+9,y2,
				LIGHTGRAY,
				vscrollchange);
	if(!lbut)
		return(0);

	rbut=button(winnum,
				upicon,
				x1,y1,x1+9,y1+9,
				LIGHTGRAY,
				vscrollchange);
	if(!rbut)
		return(0);

	mbut=button(winnum,
				"",
				x1,y1+10,x1+9,y1+19,
				LIGHTGRAY,LIGHTGRAY,
				NULL);
	if(!rbut)
		return(0);
	buttoninfo[mbut].enabled=0;

	// Store buttons
	vscrollinfo[num].lbut=lbut;
	vscrollinfo[num].rbut=rbut;
	vscrollinfo[num].mbut=mbut;

	// Create link between buttons and value changing function
	vscrolltable[lbut].ptr=num;
	vscrolltable[lbut].inc=1;
	vscrolltable[rbut].ptr=num;
	vscrolltable[rbut].inc=-1;

	// Return button number
	return(num);
}


// Move slider indicator position
void scrollclick(int type,int num,int areaptr,int clipptr) {
	// Variables
	int old,cpos,width,x1,y1;

	// Get clipping area
	x1=area[clipptr].x1;
	y1=area[clipptr].y1;

	// Find new position
	if(type==HSCROLL) {
		old=(*hscrollinfo[num].value);

		cpos=(m.mx-(x1+area[areaptr].x1+10));
		width=(area[areaptr].x2-area[areaptr].x1-19);
		(*hscrollinfo[num].value)=(cpos*(1+hscrollinfo[num].high))/width;

		// Update slider if moved
		if(old!=(*hscrollinfo[num].value))
			drawhscroll(num);
	} else {
		old=(*vscrollinfo[num].value);

		cpos=(m.my-(y1+area[areaptr].y1+10));
		width=(area[areaptr].y2-area[areaptr].y1-19);
		(*vscrollinfo[num].value)=(cpos*(1+vscrollinfo[num].high))/width;

		// Update slider if moved
		if(old!=(*vscrollinfo[num].value))
			drawvscroll(num);
	}


	// Call onclick function, passing the slider number
	if((type==HSCROLL)?hscrollinfo[num].onclick:hscrollinfo[num].onclick)
		(type==HSCROLL)?hscrollinfo[num].onclick(num):hscrollinfo[num].onclick(num);
}


// Make a button pressed and call onclick function
void buttonclick(int type,int num,int areaptr,int clipptr) {
	// Variables
	int pressed,x1,y1,x2,y2,mx,my;

	// Return if not enabled
	if(!buttoninfo[num].enabled)
		return;

	// Get clipping area
	x1=area[clipptr].x1;
	y1=area[clipptr].y1;
	x2=area[clipptr].x2;
	y2=area[clipptr].y2;

	// Limit checking area to parent clip area
	mx=min(x2,x1+area[areaptr].x2);
	my=min(y2,y1+area[areaptr].y2);

	// Indent button whilst pressed
	pressed=0;

	do {
		// Update mouse
		mcheck();

		// If on button ...
		if(zone(m.mx,m.my,
				x1+area[areaptr].x1,y1+area[areaptr].y1,
				mx,my)) {
			// ... press if not pressed
			if(!pressed) {
				// Draw indent button
				if(type==BUTTON)
					drawbutton(num,1);
				else
					drawiconbutton(num,1);

				// Make pressed
				pressed=1;
			}
		} else
			// If not on button, release it
			if(pressed) {
				// Draw normal button
				if(type==BUTTON)
					drawbutton(num,0);
				else
					drawiconbutton(num,0);

				// Make released
				pressed=0;
			}
	} while(m.lmb);

	// Return if not pressed
	if(!pressed)
		return;

	// Release button
	if(type==BUTTON)
		drawbutton(num,0);
	else
		drawiconbutton(num,0);

	// Call onclick function, passing the button number
	if(buttoninfo[num].onclick)
		buttoninfo[num].onclick(num);
}


// Check to see if any areas are pressed
void checkareas(void) {
	// Variables
	int check,num,areaptr,clipptr,x1,y1,x2,y2,mx,my,type,inarea;

	// Check to see if mouse is on an area
	num=0;
	for(check=1;check<MAREAS;check++) {
		// Get area type and number
		type=win[order[1]].objects[check][0];
		num=win[order[1]].objects[check][1];

		// Get object area pointer and clip area pointer
		switch(type) {
			case BUTTON :
				areaptr=buttoninfo[num].areaptr;
				clipptr=buttoninfo[num].clip;
				break;
			case ICONBUTTON :
				areaptr=buttoninfo[num].areaptr;
				clipptr=buttoninfo[num].clip;
				break;
			case HSCROLL :
				areaptr=hscrollinfo[num].areaptr;
				clipptr=hscrollinfo[num].clip;
				break;
			case VSCROLL :
				areaptr=vscrollinfo[num].areaptr;
				clipptr=vscrollinfo[num].clip;
				break;
		}

		// Get clipping co-ordinates
		x1=area[clipptr].x1;
		y1=area[clipptr].y1;
		x2=area[clipptr].x2;
		y2=area[clipptr].y2;

		// Limit checking area to parent clip area
		mx=min(x2,x1+area[areaptr].x2);
		my=min(y2,y1+area[areaptr].y2);

		// Check object area
		inarea=zone(m.mx,m.my,
			x1+area[areaptr].x1+((type==HSCROLL)?10:0),
			y1+area[areaptr].y1+((type==VSCROLL)?10:0),
			mx-((type==HSCROLL)?10:0),
			my-((type==VSCROLL)?10:0));

		// If mouse is in the area, stop looping...
		if(inarea) {
			// ...and call appropriate function
			switch(type) {
				case BUTTON :
					buttonclick(type,num,areaptr,clipptr);
					break;
				case ICONBUTTON :
					buttonclick(type,num,areaptr,clipptr);
					break;
				case HSCROLL :
					scrollclick(type,num,areaptr,clipptr);
					break;
				case VSCROLL :
					scrollclick(type,num,areaptr,clipptr);
					break;
			}

			// Return
			return;
		}
	}
}


// Check to see if any user-defined areas are pressed
void checkuserareas(void) {
	// Variables
	int check,areaptr,clipptr,x1,y1,x2,y2,mx,my,inarea;

	// Check to see if mouse is on an area
	for(check=1;check<MUSERS;check++) {
		// Get area number
		areaptr=win[order[1]].userarea[check];
		clipptr=win[order[1]].clip;

		// Check if element is used
		if(areaptr) {
			// Get clipping co-ordinates
			x1=area[clipptr].x1;
			y1=area[clipptr].y1;
			x2=area[clipptr].x2;
			y2=area[clipptr].y2;

			// Limit checking area to parent clip area
			mx=min(x2,x1+area[areaptr].x2);
			my=min(y2,y1+area[areaptr].y2);

			// Check object area
			inarea=zone(m.mx,m.my,
				x1+area[areaptr].x1,
				y1+area[areaptr].y1,
				mx,my);

			// If mouse is in the area, stop looping...
			if(inarea) {
				// ...and call appropriate function
				if(win[order[1]].usercall[check])
					win[order[1]].usercall[check]();

				// Return
				return;
			}
		}
	}
}


// Add a user-function and area to the check list
int userarea(int winnum,int x1,int y1,int x2,int y2,void (*func)(void)) {
	// Variables
	int areaptr,slot;

	// Add area to structure
	areaptr=addarea(x1,y1,x2,y2);
	if(!areaptr)
		return(0);

	// Add data to a user slot
	slot=userslot(winnum,areaptr,func);

	// Return slot number
	return(slot);
}


// Add listbox data to structure
int listbox(int winnum,int x1,int y1,int x2,int y2,char *string) {
	// Variables
	int areaptr,slot;

	// Add area to structure
	areaptr=addarea(x1,y1,x2,y2);
	if(!areaptr)
		return(0);

	// Add data to a listbox slot
	slot=listslot(winnum,areaptr);
	if(!slot)
		return(0);

	// Split up 'string' into text items

	// Return slot number
	return(slot);
}
