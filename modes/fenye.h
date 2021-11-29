#ifndef FENYE_H_INCLUDED
#define FENYE_H_INCLUDED
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
namespace FENYE {
const int Pagesize = 16;

struct PCB {
	int pid;
	int allsize;
	vector<int> pageTable;
	PCB (int id = -1, int size1 = 0) : pid (id), allsize(size1) {}
	friend bool operator == (const PCB & p1, const PCB & p2) {
		return p1.pid == p2.pid;
	}
};

vector<PCB> Processes;
vector<pair<int, int>> Space; // pair<pid, pageid>
using SpacePair = vector<pair<int, int>>::iterator;

//string Algo = "FF";

PCB & getPCB(int pid) {
	static PCB nullPCB;
	auto ite = find(Processes.begin(), Processes.end(), pid);
	if (ite != Processes.end())
		return *ite;
	else
		return nullPCB;
}

int getLoc(SpacePair ite) {
	int i = 0;
	while (--ite >= Space.begin()) {
		++i;
	}
	return i;
}

vector<SpacePair> findSpace() {
	vector<SpacePair> res;
	for (auto ite = Space.begin(); ite != Space.end(); ++ite) {
		if (ite->first == -1)
			res.push_back(ite);
	}
	return res;
}

bool put(int pid) {
	PCB & pcb1 = getPCB(pid);
	unsigned int pageneed = pcb1.allsize / Pagesize + bool(pcb1.allsize % Pagesize);
	auto ites = findSpace();
	if (ites.size() < pageneed)
		return false;
	for (unsigned int i = 0; i < pageneed; ++i) {
		ites[i]->first = pid;
		ites[i]->second = i;
		pcb1.pageTable.push_back(getLoc(ites[i]));
	}
	return true;
}

void unput(int pid) {
	for (auto ite = Space.begin(); ite != Space.end(); ++ite) {
		if (ite->first == pid)
			ite->first = -1;
	}
	Processes.erase(find(Processes.begin(), Processes.end(), PCB(pid)));
}

void create() {
	int allsize, pid;
	scr_gotoxy (14, 10);
	cout << "                             ";
	scr_gotoxy (14, 11);
	cout << " 请输入进程所需空间:         ";
	scr_gotoxy (14, 12);
	cout << "                             ";
	scr_gotoxy (35, 11);
	cin >> allsize;
	for (pid = 0; ; pid++)
		if (find (Processes.begin(), Processes.end(), PCB (pid)) == Processes.end())
			break;
	Processes.push_back (PCB (pid, allsize));
	if (put(pid)) {
		char str[100];
		sprintf (str, "已创建进程, 进程id为: %d", pid);
		scr_status (str);
	} else {
		Processes.pop_back();
		scr_status("当前无法分配此分区！");
	}
}

void distroy() {
	int pid;
	scr_gotoxy (14, 10);
	cout << "                             ";
	scr_gotoxy (14, 11);
	cout << "     请输入进程id号:         ";
	scr_gotoxy (14, 12);
	cout << "                             ";
	scr_gotoxy (35, 11);
	cin >> pid;
	auto pt = find (Processes.begin(), Processes.end(), PCB (pid));
	if (pt == Processes.end()) {
		scr_status ("找不到对应进程");
		return;
	}
	unput(pid);
	char str[100];
	sprintf (str, "已将进程%d占用的页面释放", pid);
	scr_status (str);
}

void calc() {
	int pid;
	scr_gotoxy (14, 10);
	cout << "                             ";
	scr_gotoxy (14, 11);
	cout << "     请输入进程id号:         ";
	scr_gotoxy (14, 12);
	cout << "                             ";
	scr_gotoxy (35, 11);
	cin >> pid;
	getchar();
	auto pt = find (Processes.begin(), Processes.end(), PCB (pid));
	if (pt == Processes.end()) {
		scr_status ("找不到对应进程");
		return;
	}
	int logicLoc;
	scr_gotoxy (14, 10);
	cout << "                             ";
	scr_gotoxy (14, 11);
	cout << "     请输入逻辑地址: 0X      ";
	scr_gotoxy (14, 12);
	cout << "                             ";
	scr_gotoxy (37, 11);
	scanf("%x", &logicLoc);
	getchar();
	if (logicLoc > pt->allsize) {
		scr_status("超出进程总大小，查询无效");
		return;
	}
	int physiLoc = pt->pageTable[logicLoc / Pagesize] * Pagesize + logicLoc % Pagesize;
	char str[100];
	sprintf (str, "物理地址为: 0X%X", physiLoc);
	scr_gotoxy (14, 10);
	cout << "                             ";
	scr_gotoxy (14, 11);
	cout << setw(29) << str << "       ";
	scr_gotoxy (14, 12);
	cout << "                             ";
	getchar();
	scr_status (str);
}

void view(int start) {
	scr_gotoxy (25, 4);
	cout << "每个页面被划分为 " << Pagesize << " 空间";
	scr_gotoxy (25, 6);
	//cout << "       " << lastVisited;
	scr_gotoxy (0, start - 1);
	scr_line();
	scr_gotoxy (0, start - 1);
	cout << "+-----------------------+\n";
	cout << "|    (块号) pid/页号    |\n"; // start
	cout << "|-----------------------|\n";
	cout << "|     |     |     |     |\n"; // 2
	cout << "|-----------------------|\n";
	cout << "|     |     |     |     |\n"; // 4
	cout << "|-----------------------|\n";
	cout << "|     |     |     |     |\n"; // 6
	cout << "|-----------------------|\n";
	cout << "|     |     |     |     |\n"; // 8
	cout << "+-----------------------+\n";
	int i = 0;
	for (auto ite = Space.begin(); ite != Space.end(); ++ite, ++i) {
		scr_gotoxy(1 + (i % 4) * 6, start + 2 + (i / 4) * 2);
		char str[100] = "";
		if (ite->first != -1)
			sprintf (str, "%d/%X", ite->first, ite->second);
		else
			sprintf (str, "(%X)", getLoc(ite));
		cout << "|" << setw (4) << str << endl;
	}
}

void init() {
	if (Space.empty())
		Space.resize(16, pair<int, int>(-1, -1));
}

void showMenu() {
	while (1) {
		char index1;
		init();
		do {
			scr_clear();
			view(9);
			scr_gotoxy (0, 0);
			cout << "+--------------------+" << endl;
			cout << "| 1.创建进程         |" << endl;
			cout << "| 2.销毁进程         |" << endl;
			cout << "| 3.计算物理地址     |" << endl;
			cout << "| 0.返回上一级       |" << endl;
			cout << "|--------------------|" << endl;
			cout << "| 请输入:            |" << endl;
			cout << "+--------------------+" << endl;
			scr_gotoxy (11, 7);
			index1 = getchar();
			fflush (stdin);
		} while (index1 < '0' || index1 > '3');
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
			calc();
			break;
		}
		case '0':
			return;
		}
	}
}

}


#endif // FENYE_H_INCLUDED
