#include <stdio.h>
#include <string.h>
#include <map>
#include <unistd.h>
#include <pthread.h>

#include <cassert>
#include <termios.h>
#include "testdef.h"
#include "common/chartable.h"

using namespace std;

void DrawRectangle(int x, int y, int w, int h)
{
	printf("\e[%d;%dH+", y, x);
	for(int i = 1; i < w; i++)
	{
		printf("\e[%d;%dH-", y, x + i);
		printf("\e[%d;%dH-", y + h, x + i);
	}

	for(int i = 1; i < h; i++)
        {
                printf("\e[%d;%dH|", y + i, x);
                printf("\e[%d;%dH|", y + i, x + w);
        }
	

	printf("\e[%d;%dH+", y, x);
	printf("\e[%d;%dH+", y, x + w);
	printf("\e[%d;%dH+", y + h, x);
	printf("\e[%d;%dH+", y + h, x + w);	

	
}

void Move(int x, int y)
{
	printf("\e[%d;%dH", y, x);
}

DECLARE_FUNC(tcdemo)
{
	struct termios told;
	struct termios tnew;

	printf("■");
	tcgetattr(0, &told);
	tnew = told;

	tnew.c_lflag &= ~(ICANON | ECHO | ECHOE | IEXTEN);
	tnew.c_oflag &= ~ONLCR;
	tnew.c_cc[VMIN] = 1;
	tnew.c_cc[VTIME] = 0;
	
	
	tcsetattr(0, TCSANOW, &tnew);

	printf("\e[0;0H\e[2J");
	//printf("\e[%d;%dH", 20, 10);
	
	DrawRectangle(10, 10, 40, 20);

	Move(30, 20);

	fflush(stdout);
	int buf[2] = {0};
	while (buf[0] != 'q')
	{
		read(0, buf, 1);
		switch(buf[0])
		{
			case 'w':
				printf("\e[1A");
				break;
			case 's':
				printf("\e[1B");
				break;
			case 'a':
				printf("\e[1D");
				break;
			case 'd':
				printf("\e[1C");
				break;
			default:
				;
				//printf("%c", buf[0]);
		}
		fflush(stdout);
	}


	for(int i = 0; i < 10 ; i++)
	{
		printf("\e[35m%-3d%c\e[0m\e[4D\e[?25l", (i + 1) * 100 /10, '%');
		fflush(stdout);
		sleep(1);
	}

	printf("\e[?25h\n");
		
	Move(0, 31);

	tcsetattr(0, TCSANOW, &told);

	return 0;
}

DECLARE_FUNC(chartable)
{
	CCharTable tab(4, 5);
	CStyle* style = tab.CreateStyle();

	style->SetHorAlign(CStyle::LIFT);
	style->SetVerAlign(CStyle::VCENTER);

	tab.Write(0, 0, "id");
	tab.Write(0, 1, "name");
	tab.Write(0, 2, "age");
	tab.Write(0, 3, "sexy");
	tab.Write(0, 4, "addr");

	tab.Column(0)->SetStyle(style);
	tab.Column(4)->SetStyle(style);
	tab.Column(4)->SetWidth(10);

	for (int i = 1; i < 4; i++)
	{
		tab.Row(i)->SetHight(3);
		tab.Write(i, 0, I2S(i+1));
		tab.Write(i, 1, "panda");
		tab.Write(i, 2, "10");
		tab.Write(i, 3, "man");
		tab.Write(i, 4, "sheng zheng baoan xi xiang");
	}

	tab.Print();
	//cout << tab.TopLine() << endl;
	//cout << StrNum('*', 10) << endl;
	return 0;
}
