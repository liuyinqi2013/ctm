#include <stdio.h>
#include <string.h>
#include <map>
#include <unistd.h>
#include <pthread.h>
#include "define.h"

using namespace std;

DECLARE_FUNC(chartable)
{
	CCharTable tab(4, 5);
	CStyle* style = tab.CreateStyle();

	style->SetHorAlign(CStyle::RIGHT);
	style->SetVerAlign(CStyle::VCENTER);
	style->SetColor(CStyle::RED);

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
	return 0;
}

DECLARE_FUNC(testqueue)
{
	CSafetyQueue<int> queue;
	queue.PushBack(1);
	queue.PushBack(2);

	if (CSafetyQueue<int>::ERR_TIME_OUT == queue.PushBack(3, 500))
	{
		printf("PushBack time out\n");
	}

	if (CSafetyQueue<int>::ERR_TIME_OUT == queue.PushFront(4, 500))
	{
		printf("PushFront time out\n");
	}

	int a = 0;

	while (1)
	{
		int ret = queue.GetPopBack(a, 500);
		if (CSafetyQueue<int>::ERR_TIME_OUT == ret)
		{
			printf("GetPopBack time out\n");
		}
		else if (ret == -1)
		{
			printf("error !\n");
			break;
		}
		else
		{
			printf("a = %d\n", a);
		}
		
	}
	return 0;
}

DECLARE_FUNC_EX(timetool)
{
	cout << Timestamp() << endl;
	cout << MilliTimestamp() << endl;

	cout << DateTime() << endl;
	cout << DateTime(TDATE_FMT_1) << endl;
	cout << DateTime(TDATE_FMT_2) << endl;
	cout << DateTime(TDATE_FMT_3) << endl;
	cout << DateTime(TDATE_FMT_4) << endl;
	cout << DateTime(TDATE_FMT_5) << endl;

	cout << DateTime(TDATE_FMT_1, TTIME_FMT_0) << endl;
	cout << DateTime(TDATE_FMT_1, TTIME_FMT_1) << endl;
	cout << DateTime(TDATE_FMT_1, TTIME_FMT_2) << endl;
	cout << DateTime(TDATE_FMT_1, TTIME_FMT_3) << endl;

	cout << Date(TDATE_FMT_0) << endl;
	cout << Date(TDATE_FMT_1) << endl;
	cout << Date(TDATE_FMT_2) << endl;
	cout << Date(TDATE_FMT_3) << endl;
	cout << Date(TDATE_FMT_4) << endl;
	cout << Date(TDATE_FMT_5) << endl;

	cout << Time(TTIME_FMT_0) << endl;
	cout << Time(TTIME_FMT_1) << endl;
	cout << Time(TTIME_FMT_2) << endl;
	cout << Time(TTIME_FMT_3) << endl;

	cout << DayOfWeek() << endl;
	cout << WeekOfYear() << endl;
	cout << Timezone() << endl;
	cout << TodayBeginTime() << endl;
	cout << TodayEndTime() << endl;
	
	return 0;
}