#ifndef PROCESS_H_INCLUDED
#define PROCESS_H_INCLUDED
#include <string>
#include <vector>
#include <iomanip>
#include <iostream>
#include <algorithm>
#include <cstdio>
#include <thread>
#include <chrono>
#include "../env_adapter.h"
namespace PROCESS {
struct PCB {
	int pid;
	string status;
	int len;
	int run;
	int wait;
	PCB (int id = -1, string st = "", int l = -1) : pid (id), status (st), len (l), run (0), wait(0) {}
	friend bool operator == (const PCB & p1, const PCB & p2) {
		return p1.pid == p2.pid;
	}
};
vector<PCB> Processes;

int Run = -1;
int timeslice = 3;
vector<int> Ready;
vector<int> Block;
vector<int> Suspend;

string Algo = "FCFS";

PCB & getPCB(int pid) {
	static PCB nullPCB;
	auto ite = find(Processes.begin(), Processes.end(), pid);
	if (ite != Processes.end())
		return *ite;
	else
		return nullPCB;
}

void refreshReady() {
	if (Algo == "SJF") {
		sort (Ready.begin(), Ready.end(), [] (int p1, int p2) {
			return getPCB(p1).len - getPCB(p1).run < getPCB(p2).len - getPCB(p2).run;
		});
	} else if (Algo == "HRRN") {
		sort (Ready.begin(), Ready.end(), [] (int p1, int p2) {
			auto calc = [](int len, int wait) {
				return double(len + wait) / len;
			};
			return calc(getPCB(p1).len, getPCB(p1).wait) > calc(getPCB(p2).len, getPCB(p2).wait);
		});
	}
}

void create() {
	int len, pid;
	scr_gotoxy (14, 10);
	cout << "                             ";
	scr_gotoxy (14, 11);
	cout << "     请输入进程长度:         ";
	scr_gotoxy (14, 12);
	cout << "                             ";
	scr_gotoxy (35, 11);
	cin >> len;
	for (pid = 0; ; pid++)
		if (find (Processes.begin(), Processes.end(), PCB (pid)) == Processes.end())
			break;
	Processes.push_back (PCB (pid, "ready", len));
	Ready.push_back (pid);
	char str[100];
	sprintf (str, "已创建进程, 进程id为: %d", pid);
	scr_status (str);
	refreshReady();
}

void vecChange (vector<int> & v1, vector<int> & v2, int pt) {
	v2.push_back (pt);
	for (auto ite = v1.begin(); ite != v1.end(); ite++) {
		if ( *ite == pt) {
			v1.erase (ite);
			break;
		}
	}
	refreshReady();
}

void runChange (vector<int> & v, int pt) {
	v.push_back (pt);
	Run = -1;
	refreshReady();
}

void block() {
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
	char str[100];
	if (pt->status == "block") {
		pt->status = "ready";
		sprintf (str, "已将进程%d唤醒", pid);
		vecChange (Block, Ready, pid);
	} else if (pt->status == "ready") {
		pt->status = "block";
		sprintf (str, "已将进程%d阻塞", pid);
		vecChange (Ready, Block, pid);
	} else if (pt->status == "sus-block") {
		pt->status = "sus-ready";
		sprintf (str, "已将进程%d唤醒（挂起中）", pid);
		//vecChange(Ready, Block, pid);
	} else if (pt->status == "sus-ready") {
		pt->status = "sus-block";
		sprintf (str, "已将进程%d阻塞（挂起中）", pid);
		//vecChange(Ready, Block, pid);
	} else {
		pt->status = "block";
		sprintf (str, "已将进程%d阻塞（运行中断）", pid);
		runChange (Block, pid);
	}
	scr_status (str);
}

void suspend() {
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
	char str[100];
	if (pt->status == "sus-ready") {
		pt->status = "ready";
		sprintf (str, "已将进程%d激活", pid);
		vecChange (Suspend, Ready, pid);
	} else if (pt->status == "sus-block") {
		pt->status = "block";
		sprintf (str, "已将进程%d激活", pid);
		vecChange (Suspend, Block, pid);
	} else if (pt->status == "ready") {
		pt->status = "sus-ready";
		sprintf (str, "已将进程%d挂起", pid);
		vecChange (Ready, Suspend, pid);
	} else if (pt->status == "block") {
		pt->status = "sus-block";
		sprintf (str, "已将进程%d挂起", pid);
		vecChange (Block, Suspend, pid);
	} else {
		pt->status = "sus-ready";
		sprintf (str, "已将进程%d挂起（运行中断）", pid);
		runChange (Suspend, pid);
	}
	scr_status (str);
}

void algoChange() {
	int i;
	scr_gotoxy (14, 8);
	cout << "                             ";
	scr_gotoxy (14, 9);
	cout << "     当前算法: " << setw (6) << Algo << "        ";
	scr_gotoxy (14, 10);
	cout << "   1.FCFS 2.SJF 3.HRRN       ";
	scr_gotoxy (14, 11);
	cout << "     请输入算法序号:         ";
	scr_gotoxy (14, 12);
	cout << "                             ";
	scr_gotoxy (35, 11);
	cin >> i;
	switch (i) {
	case 1: {
		Algo = "FCFS";
		scr_status ("切换到先来先服务算法");
		sort (Ready.begin(), Ready.end());
		break;
	}
	case 2: {
		Algo = "SJF";
		scr_status ("切换到短作业优先算法");
		break;
	}
	case 3: {
		Algo = "HRRN";
		scr_status ("切换到高响应比优先算法");
		break;
	}
	default:
		scr_status ("输入有误，未切换算法");
	}
	refreshReady();
}

void nextTimeslice() {
	// 装载一个进程
	auto runP = []{
		if (Ready.empty())
			return;
		timeslice = 3;
		Run = Ready[0];
		getPCB(Run).status = "running";
		Ready.erase(Ready.begin());
	};

	// 时间片轮转调度
	if (Run == -1) {
		runP();
	} else if (getPCB(Run).run == getPCB(Run).len) {
		//int ttt;
		//cout << "执行之前 "<<Run->pid << "?" << Block[0]->pid;
		//cin >>  ttt;
		Processes.erase(find(Processes.begin(), Processes.end(), PCB(Run)));
		Run = -1;
		runP();
		//cout << "after " <<Run->pid << "?" << Block[0]->pid;
		//cin >> ttt;
	} else if (timeslice == 0) {
		getPCB(Run).status = "ready";
		runChange(Ready, Run);
		runP();
	}

	// 执行一个时间片
	if (Run != -1) {
		timeslice--;
		getPCB(Run).run++;
		for (int i = 0; i < Processes.size(); i++)
			if (i != Run)
				getPCB(i).wait++;
		refreshReady();
	}
}

void view() {
	scr_gotoxy (25, 2);
	cout << "当前运行: ";
	if (Run != -1)
		cout << Run << " (剩余" << timeslice << "个时间片)";
	scr_gotoxy (25, 4);
	cout << "就绪队列: ";
	for (auto p : Ready)
		cout << p << " ";
	scr_gotoxy (25, 6);
	cout << "阻塞队列: ";
	for (auto p : Block)
		cout << p << " ";
	scr_gotoxy (25, 8);
	cout << "  已挂起: ";
	for (auto p : Suspend)
		cout << p << " ";
	scr_gotoxy (0, 10);
	scr_line();
	scr_gotoxy (0, 10);
	cout << "+---------------------------------+\n";
	cout << "|       PCB列表                   |\n";
	cout << "| id     状态  长度  运行  已等待 |" << endl;
	for (auto p : Processes)
		cout << "|" << setw (3) << p.pid << setw (9) << p.status << setw (6) << p.len << setw (6) << p.run << setw (9) << p.wait << "|" << endl;
	cout << "+---------------------------------+\n";
	scr_gotoxy (0, 10);
}

void showMenu() {
	while (1) {
		char index1;
		do {
			scr_clear();
			view();
			scr_gotoxy (0, 0);
			cout << "+--------------------+" << endl;
			cout << "| 1.创建进程         |" << endl;
			cout << "| 2.阻塞/唤醒进程    |" << endl;
			cout << "| 3.挂起/激活进程    |" << endl;
			cout << "| 4.更改调度算法     |" << endl;
			cout << "| 5.进入下一个时间片 |" << endl;
			cout << "| 0.返回上一级       |" << endl;
			cout << "|--------------------|" << endl;
			cout << "| 请输入:            |" << endl;
			cout << "+--------------------+" << endl;
			scr_gotoxy (11, 9);
			index1 = getchar();
			fflush (stdin);
		} while (index1 < '0' || index1 > '5');
		switch (index1) {
		case '1': {
			create();
			break;
		}
		case '2': {
			block();
			break;
		}
		case '3': {
			suspend();
			break;
		}
		case '4': {
			algoChange();
			break;
		}
		case '5': {
			nextTimeslice();
			break;
		}
		case '0':
			return;
		}
	}
}

}
#endif // PROCESS_H_INCLUDED
