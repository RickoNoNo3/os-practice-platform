#ifndef FRUIS_H_INCLUDED
#define FRUIS_H_INCLUDED
#include <string>
#include <queue>
using namespace std;
namespace FRUITS {
const int N = 3; // buffer容量 3

struct PCB {
	int pid;
	string status;
	int len;
	int run;
	int actor; // 0:爸爸 1:妈妈 2:儿子 3:女儿
	int PC;
	PCB (int id = -1, string st = "", int ac = -1) : pid (id), status (st), actor(ac), run(0), PC(0) {}
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

Semaphore mutex(1), haveOrange(0), haveApple(0), isempty(N);

int oranges = 0, oranges_used = 0;
int apples = 0, apples_used = 0;

// 生产者(爸爸或妈妈)，生产一个资源
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
			if (getPCB(pid).actor == 0) // 爸爸生产苹果
				apples++;
			else                        // 妈妈生产橘子
				oranges++;
			p.PC++;
			break;
		}
		case 3: {
			signal(mutex);
			p.PC++;
			break;
		}
		case 4: {
			if (getPCB(pid).actor == 0) // 爸爸释放苹果信号量
				signal(haveApple);
			else                        // 妈妈释放橘子信号量
				signal(haveOrange);
			p.PC++;
			break;
		}
		case 5: {
			isEnd = true;
		}
	}
	return isEnd;
}
// 消费者(儿子或女儿)，消费一个资源
bool Customer(int pid) {
	bool isEnd = false;
	PCB & p = getPCB(pid);
	switch (p.PC) {
		case 0: {
			if (getPCB(pid).actor == 2) // 儿子等待橘子信号量
				wait(haveOrange, pid);
			else                        // 女儿等待苹果信号量
				wait(haveApple, pid);
			p.PC++;
			break;
		}
		case 1: {
			wait(mutex, pid);
			p.PC++;
			break;
		}
		case 2: {
			if (getPCB(pid).actor == 2)
				oranges--;
			else
				apples--;
			p.PC++;
			break;
		}
		case 3: {
			if (getPCB(pid).actor == 2)
				oranges_used++;
			else
				apples_used++;
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
	int pid, actor;
	scr_gotoxy (14, 9);
	cout << "                             ";
	scr_gotoxy (14, 10);
	if (isProducer)
		cout << "     0:爸爸  1:妈妈          ";
	else
		cout << "     0:儿子  1:女儿          ";
	scr_gotoxy (14, 11);
	cout << "     请输入要创建的进程:     ";
	scr_gotoxy (14, 12);
	cout << "                             ";
	scr_gotoxy (39, 11);
	cin >> actor;
	if(actor != 0 && actor != 1) {
		scr_status("输入有误，未创建进程");
		return;
	}
	if (!isProducer)
		actor += 2;
	for (pid = 0; ; pid++)
		if (find (Processes.begin(), Processes.end(), PCB (pid)) == Processes.end())
			break;
	Processes.push_back (PCB (pid, "ready", actor));
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
	if (pt == Processes.end())
		return;
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
		if (getPCB(Run).actor <= 1)
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
		switch (getPCB(pid).actor) {
		case 0: return "爸爸";
		case 1: return "妈妈";
		case 2: return "儿子";
		case 3: return "女儿";
		}
		return "";
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
	cout << "| 盘中苹果数:" << setw(14) << apples << " |\n";
	cout << "| 盘中橘子数:" << setw(14) << oranges << " |\n";
	cout << "| 已吃苹果数:" << setw(14) << apples_used << " |\n";
	cout << "| 已吃橘子数:" << setw(14) << oranges_used << " |\n";
	cout << "+---------------------------+\n";
	cout << "|       PCB列表             |\n";
	cout << "| id     状态    PC    角色 |" << endl;
	for (auto p : Processes)
		cout << "|" << setw (3) << p.pid << setw (9) << p.status << setw (6) << p.PC << setw (10) << actorCheck(p.pid) << " |\n";
	cout << "+---------------------------+\n";
	scr_gotoxy (30, 8);
	cout << "-----------------+";
	scr_gotoxy (30, 9);
	cout << "    信号量状态   |";
	scr_gotoxy (30, 10);
	cout << "      mutex:" << setw(4) << mutex.v << " |";
	scr_gotoxy (30, 11);
	cout << "    isempty:" << setw(4) << isempty.v << " |";
	scr_gotoxy (30, 12);
	cout << "  haveApple:" << setw(4) << haveApple.v << " |";
	scr_gotoxy (30, 13);
	cout << " haveOrange:" << setw(4) << haveOrange.v << " |";
	scr_gotoxy (30, 14);
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
			cout << "| 1.创建爸爸/妈妈    |" << endl;
			cout << "| 2.创建儿子/女儿    |" << endl;
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


#endif // FRUIS_H_INCLUDED
