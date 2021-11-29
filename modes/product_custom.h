#ifndef PRODUCT_CUSTOM_H_INCLUDED
#define PRODUCT_CUSTOM_H_INCLUDED
#include "../env_adapter.h"
#include <iostream>
#include <vector>
#include <string>
#include <queue>
namespace PROCUS {
/**************
  生产者消费者问题专用程序逻辑
***************/
const int N = 3; // buffer容量 3

struct PCB {
	int pid;
	string status;
	int len;
	int run;
	bool isProducer;
	int PC;
	PCB (int id = -1, string st = "", bool is = true) : pid (id), status (st), isProducer(is), run(0), PC(0) {}
	friend bool operator == (const PCB & p1, const PCB & p2) { return p1.pid == p2.pid; }
};

// 获取PCB信息
PCB & getPCB(int pid);

// 阻塞与唤醒
void block(int pid);

struct Semaphore{
	int v;
	queue<int> que;
	Semaphore(int v1 = 0) : v(v1) {}
}; // 记录型信号量

bool wait(Semaphore & s, int pid) {
	s.v--;
	if (s.v < 0) {
		block(pid);
		s.que.push(pid);
		return false;
	}
	return true;
}

bool signal(Semaphore & s) {
	s.v++;
	if (s.v <= 0) {
		block(s.que.front());
		//getPCB(s.que.front()).PC++;
		s.que.pop();
	}
	return true;
}

Semaphore mutex(1), isempty(N), isfull(0);

int have = 0, used = 0;

// 生产者，生产一个资源
bool Producer(int pid) {
	bool isEnd = false;
	PCB & p = getPCB(pid);
	switch (p.PC) {
		case 0: {
			wait(isempty, pid);
			p.PC++;
			break;
		}
		case 1: {
			wait(mutex, pid);
			p.PC++;
			break;
		}
		case 2: {
			have++; // 生产资源
			p.PC++;
			break;
		}
		case 3: {
			signal(mutex);
			p.PC++;
			break;
		}
		case 4: {
			signal(isfull);
			p.PC++;
			break;
		}
		case 5: {
			isEnd = true;
		}
	}
	return isEnd;
}
// 消费者，消费一个资源
bool Customer(int pid) {
	bool isEnd = false;
	PCB & p = getPCB(pid);
	switch (p.PC) {
		case 0: {
			wait(isfull, pid);
			p.PC++;
			break;
		}
		case 1: {
			wait(mutex, pid);
			p.PC++;
			break;
		}
		case 2: {
			have--; // 消费资源
			p.PC++;
			break;
		}
		case 3: {
			used++;
			p.PC++;
			break;
		}
		case 4: {
			signal(mutex);
			p.PC++;
			break;
		}
		case 5: {
			signal(isempty);
			p.PC++;
			break;
		}
		case 6: {
			isEnd = true;
		}
	}
	return isEnd;
}
/**************
  通用进程管理程序
***************/
vector<PCB> Processes;

int Run = -1;
int timeslice = 3;
vector<int> Ready;
vector<int> Block;

PCB & getPCB(int pid) {
	static PCB nullPCB;
	auto ite = find(Processes.begin(), Processes.end(), pid);
	if (ite != Processes.end())
		return *ite;
	else
		return nullPCB;
}

void create(bool isProducer) {
	int pid;
	for (pid = 0; ; pid++)
		if (find (Processes.begin(), Processes.end(), PCB (pid)) == Processes.end())
			break;
	Processes.push_back (PCB (pid, "ready", isProducer));
	Ready.push_back (pid);
	char str[100];
	sprintf (str, "已创建进程, 进程id为: %d", pid);
	scr_status (str);
}

void vecChange (vector<int> & v1, vector<int> & v2, int pt) {
	v2.push_back (pt);
	for (auto ite = v1.begin(); ite != v1.end(); ite++) {
		if ( *ite == pt) {
			v1.erase (ite);
			break;
		}
	}
}

void runChange (vector<int> & v, int pt) {
	v.push_back (pt);
	Run = -1;
}

void block(int pid) {
	auto pt = find (Processes.begin(), Processes.end(), PCB (pid));
	char str[100];
	if (pt->status == "block") {
		pt->status = "ready";
		sprintf (str, "已将进程%d唤醒", pid);
		vecChange (Block, Ready, pid);
	} else if (pt->status == "ready") {
		pt->status = "block";
		sprintf (str, "已将进程%d阻塞", pid);
		vecChange (Ready, Block, pid);
	} else {
		pt->status = "block";
		sprintf (str, "已将进程%d阻塞（运行中断）", pid);
		runChange (Block, pid);
	}
	scr_status (str);
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
	} else if (timeslice == 0) {
		getPCB(Run).status = "ready";
		runChange(Ready, Run);
		runP();
	} else if (Run != -1) {
		timeslice--;
		getPCB(Run).run++;
		bool isEnd;
		if (getPCB(Run).isProducer)
			isEnd = Producer(Run);
		else
			isEnd = Customer(Run);
		if (isEnd) {
			Processes.erase(find(Processes.begin(), Processes.end(), PCB(Run)));
			Run = -1;
		}
	}
}

void view() {
	auto actorCheck = [](int pid) {
		if (getPCB(pid).isProducer == true)
			return "生产者";
		else
			return "消费者";
	};
	scr_gotoxy (25, 2);
	cout << "当前运行: ";
	if (Run != -1)
		cout << actorCheck(Run) << " " << Run << " (剩余" << timeslice << "个时间片)";
	scr_gotoxy (25, 4);
	cout << "就绪队列: ";
	for (auto p : Ready)
		cout << p << " ";
	scr_gotoxy (25, 6);
	cout << "阻塞队列: ";
	for (auto p : Block)
		cout << p << " ";
	scr_gotoxy (0, 8);
	scr_line();
	scr_gotoxy (0, 8);
	cout << "+---------------------------+\n";
	cout << "|       生产/消费状态       |\n";
	cout << "| 可用资源数:" << setw(14) << have << " |\n";
	cout << "| 已用资源数:" << setw(14) << used << " |\n";
	cout << "+---------------------------+\n";
	cout << "|       PCB列表             |\n";
	cout << "| id     状态    PC    角色 |" << endl;
	for (auto p : Processes)
		cout << "|" << setw (3) << p.pid << setw (9) << p.status << setw (6) << p.PC << setw (11) << actorCheck(p.pid) << " |\n";
	cout << "+---------------------------+\n";
	scr_gotoxy (30, 8);
	cout << "-----------------+";
	scr_gotoxy (30, 9);
	cout << "    信号量状态   |";
	scr_gotoxy (30, 10);
	cout << "   mutex:   " << setw(4) << mutex.v << " |";
	scr_gotoxy (30, 11);
	cout << " isempty:   " << setw(4) << isempty.v << " |";
	scr_gotoxy (30, 12);
	cout << "  isfull:   " << setw(4) << isfull.v << " |";
	scr_gotoxy (30, 13);
	cout << "-----------------+";
}

void showMenu() {
	while (1) {
		char index1;
		do {
			scr_clear();
			view();
			scr_gotoxy (0, 1);
			cout << "+--------------------+" << endl;
			cout << "| 1.创建生产者进程   |" << endl;
			cout << "| 2.创建消费者进程   |" << endl;
			cout << "| 3.进入下一个时间片 |" << endl;
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
			create(true);
			break;
		}
		case '2': {
			create(false);
			break;
		}
		case '3': {
			nextTimeslice();
			break;
		}
		case '0':
			return;
		}
	}
}
}

#endif // PRODUCT_CUSTOM_H_INCLUDED
