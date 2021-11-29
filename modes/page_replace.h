#ifndef PAGE_REPLACE_H_INCLUDED
#define PAGE_REPLACE_H_INCLUDED
#include <string>
#include <vector>
#include <map>
#include <iomanip>
#include <iostream>
#include <algorithm>
#include <cstdio>
#include <thread>
#include <chrono>
#include "../env_adapter.h"
namespace PAGE_REP {
const int Pagesize = 16;

struct PCB {
	int pageid;
	int used;
	int time;
	bool flag;
	PCB (int id = -1) : pageid (id), used (0), time(0), flag(true) {}
	friend bool operator == (const PCB & p1, const PCB & p2) {
		return p1.pageid == p2.pageid;
	}
};

vector<PCB> Processes;
vector<int> Space; // pageid
vector<int> SwapSpace; // pageid

string Algo = "FIFO";
int lastPages[4] = {-1, -1, -1, -1};
int clock_now = -1;

void view(int);
PCB & getPCB (int pageid) {
	static PCB nullPCB;
	auto ite = find (Processes.begin(), Processes.end(), pageid);
	if (ite != Processes.end())
		return *ite;
	else
		return nullPCB;
}

int findSpace (int pageid, vector<int> & space) {
	for (int i = 0; i < space.size(); ++i) {
		if (space[i] == -1 || space[i] == pageid)
			return i;
	}
	return -1;
}

void unput (int pageid, vector<int> & space) {
	for (int i = 0; i < space.size(); ++i) {
		if (space[i] == pageid) {
			space[i] = -1;
			break;
		}
	}
}

void put (int pageid, vector<int> & space) {
	PCB & pcb1 = getPCB (pageid);
	int loc = findSpace (pageid, space);
	if (loc == -1) { // 需要置换
		copy(Space.begin(), Space.end(), lastPages);
		int swaped = -1;
		if (Algo == "FIFO") {
			int maxi = -1, maxtime = -999;
			for (int i = 0; i < Space.size(); ++i) {
				if (getPCB(Space[i]).time > maxtime) {
					maxtime = getPCB(Space[i]).time;
					maxi = i;
				}
			}
			swaped = getPCB(Space[maxi]).pageid;
			cout << maxi << " " << maxtime;
		} else if (Algo == "LRU") {
			int maxi = -1, maxused = -999;
			for (int i = 0; i < Space.size(); ++i) {
				if (getPCB(Space[i]).used > maxused) {
					maxused = getPCB(Space[i]).used;
					maxi = i;
				}
			}
			swaped = getPCB(Space[maxi]).pageid;
		} else if (Algo == "Clock") {
			while (1) {
				PCB & now = getPCB(Space[clock_now % 4]);
				if (now.flag) {
					now.flag = false;
					clock_now = ++clock_now % 4;
				} else {
					swaped = now.pageid;
					break;
				}
				view(10);
				this_thread::sleep_for(chrono::milliseconds(500));
			}
		}
		getPCB(pageid).time = 0;
		getPCB(pageid).used = 0;
		getPCB(pageid).flag = true;
		unput(swaped, Space);
		put(swaped, SwapSpace);
		put(pageid, Space);
		char str[100];
		sprintf(str, "将页面%d置换为页面%d", swaped, pageid);
		scr_status(str);
	} else {         // 不需要置换
		space[loc] = pageid;
	}
}

void refreshTimeSlice() {
	for (int i = 0; i < Processes.size(); ++i) {
		Processes[i].used++;
		Processes[i].time++;
	}
};

void create() {
	int pageid;
	for (pageid = 0; ; pageid++)
		if (find (Processes.begin(), Processes.end(), PCB (pageid)) == Processes.end())
			break;
	Processes.push_back (PCB (pageid));
	if (Processes.size() <= 12) {
		char str[100];
		sprintf (str, "已创建页面, 页面id为: %d", pageid);
		scr_status (str);
		refreshTimeSlice();
		put (pageid, Space);
	} else {
		Processes.pop_back();
		scr_status ("当前无法创建页面！");
	}
}

void distroy() {
	int pageid;
	scr_gotoxy (14, 10);
	cout << "                             ";
	scr_gotoxy (14, 11);
	cout << "     请输入页面id号:         ";
	scr_gotoxy (14, 12);
	cout << "                             ";
	scr_gotoxy (35, 11);
	cin >> pageid;
	auto pt = find (Processes.begin(), Processes.end(), PCB (pageid));
	if (pt == Processes.end()) {
		scr_status ("找不到对应页面");
		return;
	}
	unput (pageid, Space);
	unput (pageid, SwapSpace);
	Processes.erase (find (Processes.begin(), Processes.end(), PCB (pageid)));
	char str[100];
	sprintf (str, "已将页面%d销毁", pageid);
	scr_status (str);
}

void algoChange() {
	int i;
	scr_gotoxy (14, 8);
	cout << "                             ";
	scr_gotoxy (14, 9);
	cout << "     当前算法: " << setw (6) << Algo << "        ";
	scr_gotoxy (14, 10);
	cout << "   1.FIFO 2.LRU 3.Clock      ";
	scr_gotoxy (14, 11);
	cout << "     请输入算法序号:         ";
	scr_gotoxy (14, 12);
	cout << "                             ";
	scr_gotoxy (35, 11);
	cin >> i;
	switch (i) {
	case 1: {
		Algo = "FIFO";
		scr_status ("切换到先进先出置换算法");
		break;
	}
	case 2: {
		Algo = "LRU";
		scr_status ("切换到最近最久未使用置换算法");
		break;
	}
	case 3: {
		Algo = "Clock";
		scr_status ("切换到Clock置换算法");
		break;
	}
	default:
		scr_status ("输入有误，未切换算法");
	}
}

void nextTimeslice() {
	int pageid;
	scr_gotoxy (14, 10);
	cout << "                             ";
	scr_gotoxy (14, 11);
	cout << " 请输入要执行的页面id:       ";
	scr_gotoxy (14, 12);
	cout << "                             ";
	scr_gotoxy (37, 11);
	cin >> pageid;
	auto pt = find (Processes.begin(), Processes.end(), PCB (pageid));
	if (pt == Processes.end()) {
		scr_status ("找不到对应页面");
		return;
	}
	unput (pageid, SwapSpace);
	put (pageid, Space);
	refreshTimeSlice();
	getPCB (pageid).used = 0;
	getPCB(pageid).flag = true;
}

void view (int start) {
	auto preview = [&] (auto space) {
		cout << "  ";
		for (int i = 0; i < 4; ++i) {
			if (space[i] != -1)
				cout << setw(2) << space[i];
			else
				cout << "  ";
			cout << "  ";
		}
	};
	scr_gotoxy (25, 4);
	cout << "last: "; preview(lastPages);
	scr_gotoxy (25, 6);
	cout << " now: "; preview(Space);
	scr_gotoxy (1, start - 1);
	scr_line();
	scr_gotoxy (1, start - 1);
	cout << "+-----------------------+---------------------+\n";
	cout << "|         内 存         |     页 面 信 息     |\n"; // start
	cout << "|-----------------------| id 使用 装载 标记位 |\n";
	cout << "|     |     |     |     |                     |\n"; // 2
	cout << "+-----------------------+                     |\n";
	cout << "|        swap 区        |                     |\n"; // 4
	cout << "|-----------------------|                     |\n";
	cout << "|     |     |     |     |---------------------+\n"; // 6
	cout << "|-----------------------|                     \n";
	cout << "|     |     |     |     |                     \n"; // 8
	cout << "+-----------------------+\n";
	{
		int i = 2;
		for (int j = 0; j < 4; ++j) {
			auto p = getPCB(Space[j]);
			scr_gotoxy(26, start + i++);
			cout << setw(3) << p.pageid << setw(5) << p.used << setw(5) << p.time << setw(7) << p.flag << " |";
		}
	}

	auto prt = [&] (vector<int> & space, int I = 0) {
		for (int i = 0; i < space.size(); ++i) {
			scr_gotoxy (1 + ((I + i) % 4) * 6, start + 2 + ((I + i) / 4) * 2);
			char str[100] = "";
			if (space[i] != -1)
				sprintf (str, "  %-2d ", getPCB(space[i]));
			else
				sprintf (str, "   ");
			cout << "|" << setw (5) << str << endl;
		}
	};
	prt (Space);
	prt (SwapSpace, 8);
}

void init() {
	if (Space.empty()) {
		Space.resize (4, -1);
		SwapSpace.resize (8, -1);
	}
}

void showMenu() {
	while (1) {
		char index1;
		init();
		do {
			scr_clear();
			view (10);
			scr_gotoxy (0, 0);
			cout << "+--------------------+" << endl;
			cout << "| 1.创建新页面       |" << endl;
			cout << "| 2.销毁旧页面       |" << endl;
			cout << "| 3.更改置换算法     |" << endl;
			cout << "| 4.进入下一个时间片 |" << endl;
			cout << "| 0.返回上一级       |" << endl;
			cout << "|--------------------|" << endl;
			cout << "| 请输入:            |" << endl;
			cout << "+--------------------+" << endl;
			scr_gotoxy (11, 8);
			index1 = getchar();
			fflush (stdin);
		} while (index1 < '0' || index1 > '4');
		switch (index1) {
		case '1': {
			create();
			break;
		}
		case '2': {
			distroy();
			break;
		}
		case '3': {
			algoChange();
			break;
		}
		case '4': {
			nextTimeslice();
			break;
		}
		case '0':
			return;
		}
	}
}

}


#endif // PAGE_REPLACE_H_INCLUDED
