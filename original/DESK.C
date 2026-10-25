#include "desk.h"

void guiloop(void);

int win1,win2,win3,win4,win5,lab1,lab2,but1,spin1,spinval;

void userfunc1(void) {
	area[labelinfo[lab1].areaptr].x2=getwidth(win1);
	area[labelinfo[lab1].areaptr].y2=getheight(win1);
}

void beep1(int num) {
	// Beep
	beep();

	// Change color of button
	int back;

	back=(buttoninfo[num].color&0xf0)>>4;
	back=(back+1)%15;

	buttoninfo[num].color=(back<<4)+YELLOW;
	drawbutton(num,0);
}

void beep2(int) {
	// Beep
	beep();

	// Change color of label
	labelinfo[lab2].back=spinval;
	drawlabel(lab2);
}


void main()
{
	// Initialise desktop
	desktop.color=CYAN;
	desktop.pattern=SOLID_FILL;

	opengraph();

	// Create windows
	win1=window("I'm gonna' sneeze.",20,50,150,150);
	win[win1].back=RED;
	lab1=label(win1,
			   "This text is centered",
			   0,0,0,0,
			   YELLOW,NONE,
			   CENTER,CENTER);
	win[win1].redraw=userfunc1;

	win2=window("Beep Me.",150,300,248,349);
	win[win2].attrib=MOVE+CLOSE;
	but1=button(win2,
				"Click Here",
				9,8,82,23,
				YELLOW,MAGENTA,
				beep1);


	win3=window("You can't touch me.",300,10,450,10);
	win[win3].attrib=CLOSE;


	win4=window("Be useful-Die young.",200,380,336,476);
	win[win4].attrib=0xff-CLOSE;

	// Spinner
	label(win4,"Color Spinner",
		  10,5,120,65,
		  BLACK,NONE,
		  CENTER,TOP);
	spinval=5;
	lab2=label(win4,"",70,25,100,55,BLACK,spinval,CENTER,CENTER);
	spin1=spinner(win4,
				  30,35,2,
				  &spinval,
				  0,15,
				  beep2);

	// Scroll bars
	int hscrollpos=5;
	win5=window("Anyone out there?",350,200,600,300);
	hscrollbox(win5,
			   10,10,100,
			   &hscrollpos,10,
			   NULL);
	int vscrollpos=2;
	vscrollbox(win5,
			   110,10,75,
			   &vscrollpos,5,
			   NULL);

	// User-defined area
	label(win5,"User area",10,60,90,75,LIGHTGREEN,BLUE,CENTER,CENTER);
	userarea(win5,10,60,90,75,beep);

	// Display windows
	drawall();

	while(1) guiloop();
}


// Main loop
void guiloop(void) {
	// Update mouse
	mcheck();

	// Return if no mouse buttons are pressed
	if(!m.rmb && !m.lmb)
		return;

	// Activate a new window if necessary
	if(m.lmb)
		checkwindows();

	// If title box is clicked on, check controls or move window
	if(m.lmb)
		if(ontitlebox())
			checkcontrols();

	// If resize indicator is clicked on, resize window
	if(m.lmb)
		if(onresizer())
			resizewindow();

	// Check to see if any areas are clicked on
	if(m.lmb)
		checkareas();

	// Check to see if any user-defined areas are clicked on
	if(m.lmb)
		checkuserareas();

	// Quit if right button pressed
	if(m.rmb) {
		shutdown();
		exit(0);
	}

	// Wait until mouse is released
	while(m.lmb || m.rmb) mcheck();

	// Return
	return;
}