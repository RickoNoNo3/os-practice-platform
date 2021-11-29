#ifndef FENQU_H_INCLUDED
#define FENQU_H_INCLUDED
#include <string>
#include <vector>
#include <iomanip>
#include <iostream>
#include <algorithm>
#include <cstdio>
#include <thread>
#include <chrono>
#include "../env_adapter.h"
namespace FENQU {
struct PCB {
	int pid;
	int allsize;
	int loc;
	PCB (int id = -1, int size1 = 0) : pid (id), allsize(size1), loc(-1) {}
	friend bool operator == (const PCB & p1, const PCB & p2) {
		return p1.pid == p2.pid;
	}
};

vector<PCB> Processes;
vector<pair<int, int>> Space; // pair<pid, allsize>
using SpacePair = vector<pair<int, int>>::iterator;

int lastVisited = 0;
string Algo = "FF";

PCB & getPCB(int pid) {
	static PCB nullPCB;
	auto ite = find(Processes.begin(), Processes.end(), pid);
	if (ite != Processes.end())
		return *ite;
	else
		return nullPCB;
}

int getLoc(SpacePair ite) {
	int sum = 0;
	while (--ite >= Space.begin()) {
		sum += ite->second;
	}
	return sum;
}

SpacePair findSpace(int allsize) {
	auto ite = Space.end();
	if (Algo == "FF") {
		for (ite = Space.begin(); ite != Space.end(); ite++) {
			if (ite->first == -1 && ite->second >= allsize)
				break;
		}
	} else if (Algo == "NF") {
		ite = Space.begin();
		for (ite = Space.begin(); ite != Space.end(); ite++) {
			if (getLoc(ite) >= lastVisited)
				break;
		}
		auto tite = ite;
		auto found = false;
		do {
			if (tite >= Space.end())
				tite = Space.begin();
			if (tite->first == -1 && tite->second >= allsize) {
				found = true;
				break;
			}
			++tite;
		} while (tite != ite);
		if (found)
			ite = tite;
		else
			ite = Space.end();
	} else if (Algo == "BF") {
		int minSpace = 99999;
		auto minIte = ite;
		for (ite = Space.begin(); ite != Space.end(); ite++) {
			if (ite->first == -1 && ite->second >= allsize) {
				if (minSpace > ite->second) {
					minSpace = ite->second;
					minIte = ite;
				}
			}
		}
		ite = minIte;
	} else if (Algo == "WF") {
		int maxSpace = -99999;
		auto maxIte = ite;
		for (ite = Space.begin(); ite != Space.end(); ite++) {
			if (ite->first == -1 && ite->second >= allsize) {
				if (maxSpace < ite->second) {
					maxSpace = ite->second;
					maxIte = ite;
				}
			}
		}
		ite = maxIte;
	}
	lastVisited = getLoc(ite);
	return ite;
}

bool put(int pid) {
	PCB & pcb1 = getPCB(pid);
	auto ite = findSpace(pcb1.allsize);
	if (ite == Space.end())
		return false;
	int remainSize = ite->second - pcb1.allsize;
	ite->first = pid;
	ite->second = pcb1.allsize;
	pcb1.loc = getLoc(ite);
	if (remainSize)
		Space.insert(ite + 1, pair<int, int>(-1, remainSize));
	return true;
}

SpacePair merge(SpacePair ite1, SpacePair ite2) {
	ite1->second += ite2->second;
	Space.erase(ite2);
	return ite1;
}

void unput(int pid) {
	auto ite = find_if(Space.begin(), Space.end(), [&](const pair<int, int> & p1) {
		return p1.first == pid;
	});
	ite->first = -1;
	if (ite != Space.begin()) {
		if ((ite - 1)->first == -1) {
			ite = merge(ite - 1, ite);
		}
	}
	if (ite != Space.end() - 1) {
		if ((ite + 1)->first == -1) {
			ite = merge(ite, ite + 1);
		}
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

void algoChange() {
	int i;
	scr_gotoxy (14, 8);
	cout << "                             ";
	scr_gotoxy (14, 9);
	cout << "     当前算法: " << setw (6) << Algo << "        ";
	scr_gotoxy (14, 10);
	cout << "     1.FF 2.NF 3.BF 4.WF     ";
	scr_gotoxy (14, 11);
	cout << "     请输入算法序号:         ";
	scr_gotoxy (14, 12);
	cout << "                             ";
	scr_gotoxy (35, 11);
	cin >> i;
	switch (i) {
	case 1: {
		Algo = "FF";
		scr_status ("切换到首次适应算法");
		break;
	}
	case 2: {
		Algo = "NF";
		scr_status ("切换到循环适应算法");
		break;
	}
	case 3: {
		Algo = "BF";
		scr_status ("切换到最佳适应算法");
		break;
	}
	case 4: {
		Algo = "WF";
		scr_status ("切换到最坏适应算法");
		break;
	}
	default:
		scr_status ("输入有误，未切换算法");
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
	sprintf (str, "已将进程%d占用的分区释放", pid);
	scr_status (str);
}

void view(int start) {
	scr_gotoxy (25, 4);
	cout << "上一次寻找的结束地址: ";
	scr_gotoxy (25, 6);
	cout << "       " << lastVisited;
	scr_gotoxy (0, start - 1);
	scr_line();
	scr_gotoxy (0, start - 1);
	cout << "+---------------------+\n";
	cout << "|      内存状态       |\n";
	cout << "| pid    始址    长度 |" << endl;
	for (auto ite = Space.begin(); ite != Space.end(); ++ite)
		cout << "|" << setw (3) << ite->first << setw (9) << getLoc(ite) << setw (8) << ite->second << " |" << endl;
	cout << "+---------------------+\n";
	scr_gotoxy (0, start);
}

void compact() {
	sort(Space.begin(), Space.end(), [&](pair<int, int> p1, pair<int, int> p2) {
		if (p1.first == -1)
			return false;
		else if (p2.first == -1)
			return true;
		else
			return p1.first < p2.first;
	});
	SpacePair ite;
	for (ite = Space.begin(); ite != Space.end(); ++ite)
		if (ite->first != -1)
			getPCB(ite->first).loc = getLoc(ite);
	while ((ite - 1) != Space.begin() && (ite - 2)->first == (ite - 1)->first) {
		merge(ite - 2, ite - 1);
		ite = Space.end();
	}
}

void init() {
	if (Space.empty())
		Space.push_back(pair<int, int>(-1, 1024));
}

void showMenu() {
	while (1) {
		char index1;
		init();
		do {
			scr_clear();
			view(10);
			scr_gotoxy (0, 0);
			cout << "+--------------------+" << endl;
			cout << "| 1.创建分区         |" << endl;
			cout << "| 2.释放分区         |" << endl;
			cout << "| 3.更改适应算法     |" << endl;
			cout << "| 4.紧凑             |" << endl;
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
			compact();
			break;
		}
		case '0':
			return;
		}
	}
}

}


#endif // FENQU_H_INCLUDED
