extern "C" void delay( unsigned __milliseconds );


struct {
   char lmb;
   char rmb;
   char lmr;
   char rmr;
   int mx;
   int my;
} m;

minit()
{
	char retval;
	asm {
		mov ax,0x0000;
		int 0x33
		mov retval,ah
	}
	delay(100);
	return(retval);
}


void mshow()
{
	// Interrupt 33, service 1, switch on pointer

	asm {
		mov ax,0x0001
		int 0x33
	}
}


void mhide()
{
	// Interrupt 33, service 2, switch off pointer

	asm {
		mov ax,0x0002
		int 0x33
	}
}


void mcheck()
{
	int mx,my;
	char lmb,lmr,rmb,rmr;

	asm {
		mov ax,0x0003
		int 0x33

		mov mx,cx		// coords
		mov my,dx

		mov dl,bl		// lmb
		and dl,1
		mov lmb,dl

		mov dl,bl       // rmb
		and dl,2
		mov rmb,dl

		mov ax,0x0006
		mov bx,0x0000
		int 0x33
		mov lmr,bl		// lmr

		mov ax,0x0006
		mov bx,0x0001
		int 0x33
		mov rmr,bl		// rmr
	}

	m.mx=mx;
	m.my=my;
	m.lmb=lmb;
	m.rmb=rmb;
	m.lmr=lmr;
	m.rmr=rmr;
}


getmouseclick() {
	do {
		mcheck();

		if(m.lmr)
			return(1);
		if(m.rmr)
			return(2);
	} while(1);
}


void limitmouse(int x1,int y1,int x2,int y2) {
	x1=(x1<0)?0:x1;
	y1=(y1<0)?0:y1;

	// Set limits
	asm {
		mov ax,7
		mov cx,x1
		mov dx,x2
		int 0x33

		mov ax,8
		mov cx,y1
		mov dx,y2
		int 0x33
	}

	mcheck();
}


void setmousexy(int x,int y) {
	x=(x<0)?0:x;
	y=(y<0)?0:y;

	asm {
		mov ax,4
		mov cx,x
		mov dx,y

		int 0x33
	}

	m.mx=x;
	m.my=y;
}
