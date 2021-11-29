#ifndef READ_WRITE_RFIRST_H_INCLUDED
#define READ_WRITE_RFIRST_H_INCLUDED
#include "../env_adapter.h"
#include <iostream>
#include <vector>
#include <string>
#include <queue>
namespace RW_R {
struct PCB {
	int pid;
	string status;
	int len;
	int run;
	bool isWriter;
	int PC;
	int data;
	PCB (int id = -1, string st = "", bool is = true, int d = 0) : pid (id), status (st), isWriter (is), data (d), run (0), PC (0) {}
	friend bool operator == (const PCB & p1, const PCB & p2) {
		return p1.pid == p2.pid;
	}
};

// 获取PCB信息
PCB & getPCB (int pid);

// 阻塞与唤醒
void block (int pid);

struct Semaphore {
	int v;
	queue<int> que;
	Semaphore (int v1 = 0) : v (v1) {}
}; // 记录型信号量

bool wait (Semaphore & s, int pid) {
	s.v--;
	if (s.v < 0) {
		block (pid);
		s.que.push (pid);
		return false;
	}
	return true;
}

bool signal (Semaphore & s) {
	s.v++;
	if (s.v <= 0) {
		block (s.que.front());
		s.que.pop();
	}
	return true;
}

Semaphore wmutex (1), rmutex (1);
int readCount (0);

int data = 100;

// 读者
bool Reader (int pid) {
	bool isEnd = false;
	PCB & p = getPCB (pid);
	switch (p.PC) {
	case 0: {
		wait (rmutex, pid);
		p.PC++;
		break;
	}
	case 1: {
		if (readCount == 0)
			wait (wmutex, pid);
		p.PC++;
		break;
	}
	case 2: {
		readCount++;
		p.PC++;
		break;
	}
	case 3: {
		signal (rmutex);
		p.PC++;
		break;
	}
	case 4: {
		char str[100];
		sprintf (str, "进程%d读到的数据为%d", pid, data);
		scr_status (str);
		p.PC++;
		break;
	}
	case 5: {
		wait (rmutex, pid);
		p.PC++;
		break;
	}
	case 6: {
		readCount--;
		p.PC++;
		break;
	}
	case 7: {
		if (readCount == 0)
			signal (wmutex);
		p.PC++;
		break;
	}
	case 8: {
		signal (rmutex);
		p.PC++;
		break;
	}
	case 9: {
		isEnd = true;
	}
	}
	return isEnd;
}
// 写者
bool Writer (int pid) {
	bool isEnd = false;
	PCB & p = getPCB (pid);
	switch (p.PC) {
	case 0: {
		wait (wmutex, pid);
		p.PC++;
		break;
	}
	case 1: {
		data = getPCB (pid).data;
		char str[100];
		sprintf (str, "进程%d写入了%d", pid, data);
		scr_status (str);
		p.PC++;
		break;
	}
	case 2: {
		signal (wmutex);
		p.PC++;
		break;
	}
	case 3: {
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

PCB & getPCB (int pid) {
	static PCB nullPCB;
	auto ite = find (Processes.begin(), Processes.end(), pid);
	if (ite != Processes.end())
		return *ite;
	else
		return nullPCB;
}

void create (bool isWriter) {
	int pid, newdata = 0;
	if (isWriter) {
		scr_gotoxy (14, 10);
		cout << "                             ";
		scr_gotoxy (14, 11);
		cout << "     请输入要写入的数值:     ";
		scr_gotoxy (14, 12);
		cout << "                             ";
		scr_gotoxy (39, 11);
		cin >> newdata;
	}
	for (pid = 0; ; pid++)
		if (find (Processes.begin(), Processes.end(), PCB (pid)) == Processes.end())
			break;
	Processes.push_back (PCB (pid, "ready", isWriter, newdata));
	Ready.push_back (pid);
	char str[100];
	sprintf (str, "已创建进程, 进程id为: %d", pid);
	scr_status (str);
}

void vecChange (vector<int> & v1, vector<int> & v2, int pt) {
	v2.push_back (pt);
	for (auto ite = v1.begin(); ite != v1.end(); ite++) {
		if (*ite == pt) {
			v1.erase (ite);
			break;
		}
	}
}

void runChange (vector<int> & v, int pt) {
	v.push_back (pt);
	Run = -1;
}

void block (int pid) {
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
	auto runP = [] {
		if (Ready.empty())
			return;
		timeslice = 3;
		Run = Ready[0];
		getPCB (Run).status = "running";
		Ready.erase (Ready.begin());
	};

	// 时间片轮转调度
	if (Run == -1) {
		runP();
	} else if (timeslice == 0) {
		getPCB (Run).status = "ready";
		runChange (Ready, Run);
		runP();
	} else if (Run != -1) {
		timeslice--;
		getPCB (Run).run++;
		bool isEnd;
		if (getPCB (Run).isWriter)
			isEnd = Writer (Run);
		else
			isEnd = Reader (Run);
		if (isEnd) {
			Processes.erase (find (Processes.begin(), Processes.end(), PCB (Run)));
			Run = -1;
		}
	}
}

void view() {
	auto actorCheck = [] (int pid) {
		if (getPCB (pid).isWriter == true)
			return "写者";
		else
			return "读者";
	};
	scr_gotoxy (25, 2);
	cout << "当前运行: ";
	if (Run != -1)
		cout << actorCheck (Run) << " " << Run << " (剩余" << timeslice << "个时间片)";
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
	cout << "|   当前数据:" << setw (14) << data << " |\n";
	cout << "+---------------------------+\n";
	cout << "|       PCB列表             |\n";
	cout << "| id     状态    PC    角色 |" << endl;
	for (auto p : Processes)
		cout << "|" << setw (3) << p.pid << setw (9) << p.status << setw (6) << p.PC << setw (10) << actorCheck (p.pid) << " |\n";
	cout << "+---------------------------+\n";
	scr_gotoxy (30, 8);
	cout << "-----------------+";
	scr_gotoxy (30, 9);
	cout << "    信号量状态   |";
	scr_gotoxy (30, 10);
	cout << "    rmutex: " << setw (4) << rmutex.v << " |";
	scr_gotoxy (30, 11);
	cout << "    wmutex: " << setw (4) << wmutex.v << " |";
	scr_gotoxy (30, 12);
	cout << " readCount: " << setw (4) << readCount << " |";
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
			cout << "| 1.创建读者进程     |" << endl;
			cout << "| 2.创建写者进程     |" << endl;
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
			create (false);
			break;
		}
		case '2': {
			create (true);
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
#endif // READ_WRITE_RFIRST_H_INCLUDED
