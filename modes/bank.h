#ifndef BANK_H_INCLUDED
#define BANK_H_INCLUDED
#include "../env_adapter.h"
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <queue>
#include <iomanip>
#include <map>
#include <iterator>
#include <algorithm>
namespace BANK {
struct PCB {
	int pid;
	vector<int> MAX;
	vector<int> Alloc;
	PCB (int id = -1) : pid (id) {}
	friend bool operator == (const PCB & p1, const PCB & p2) {
		return p1.pid == p2.pid;
	}
};

vector<PCB> Processes;
vector<int> Available;
string SafeStr;

PCB & getPCB (int pid) {
	static PCB nullPCB;
	auto ite = find (Processes.begin(), Processes.end(), pid);
	if (ite != Processes.end())
		return *ite;
	else
		return nullPCB;
}

void init() {
	scr_gotoxy (14, 10);
	cout << "                             ";
	scr_gotoxy (14, 11);
	cout << "     请输入资源个数:         ";
	scr_gotoxy (14, 12);
	cout << "                             ";
	scr_gotoxy (35, 11);
	unsigned int cnt;
	cin >> cnt;
	scr_gotoxy (14, 10);
	cout << "                             ";
	scr_gotoxy (14, 11);
	cout << " 请输入资源数组:             ";
	scr_gotoxy (14, 12);
	cout << "                             ";
	scr_gotoxy (31, 11);
	getchar();
	string line;
	getline(cin, line);
	stringstream ss(line);
	vector<int> avails;
	copy(istream_iterator<int>(ss), istream_iterator<int>(), back_inserter(avails));
	if (avails.size() != cnt) {
		scr_status("输入有误，未初始化");
	} else {
		Available.insert(Available.begin(), avails.begin(), avails.end());
		scr_status("已初始化");
	}
}
bool checkSafe(string & safeStr) {
	vector<int> work = Available;
	map<int, bool> finish;
	string safeStr1 = "";
	while(1) {
		auto ite = find_if(Processes.begin(), Processes.end(), [&](const PCB & pcb1) {
			return (finish[pcb1.pid] == false) && ([&]{
				for (unsigned int j = 0; j < pcb1.MAX.size(); ++j)
					if (pcb1.MAX[j] - pcb1.Alloc[j] > work[j])
						return false;
				return true;
			}());
		});
		if (ite != Processes.end()) {
			for (unsigned int j = 0; j < ite->MAX.size(); ++j)
				work[j] += ite->Alloc[j];
			finish[ite->pid] = true;
			stringstream ss;
			ss << setw(3) << ite->pid;
			safeStr1 += ss.str();
		} else {
			return [&]{
				for (auto ite2 = finish.begin(); ite2 != finish.end(); ++ite2)
					if (!(ite2->second))
						return false;
				safeStr = safeStr1;
				return true;
			}();
		};
	}
	return false;
}

void create () {
	int pid;
	for (pid = 0; ; pid++)
		if (find (Processes.begin(), Processes.end(), PCB (pid)) == Processes.end())
			break;
	scr_gotoxy (14, 10);
	cout << "                             ";
	scr_gotoxy (14, 11);
	cout << " 请输入需求数组:             ";
	scr_gotoxy (14, 12);
	cout << "                             ";
	scr_gotoxy (31, 11);
	getchar();
	string line;
	getline(cin, line);
	int need1;
	unsigned int cnt = 0;
	PCB pcb1(pid);
	stringstream ss(line);
	while (ss >> need1) {
		++cnt;
		pcb1.MAX.push_back(need1);
	}
	if (cnt != Available.size()) {
		scr_status("输入有误，未创建进程");
	} else {
		pcb1.Alloc.resize(cnt, 0);
		Processes.push_back(pcb1);
		if (checkSafe(SafeStr)) {
			char str[100];
			sprintf (str, "已创建进程, 进程id为: %d", pid);
			scr_status (str);
		} else {
			Processes.pop_back();
			scr_status ("不安全！未创建进程");
		}
	}
}

void nextTimeslice() {
	// 运行一个进程
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
	scr_gotoxy (14, 10);
	cout << "                             ";
	scr_gotoxy (14, 11);
	cout << " 请输入申请资源:             ";
	scr_gotoxy (14, 12);
	cout << "                             ";
	scr_gotoxy (31, 11);
	getchar();
	string line;
	getline(cin, line);
	PCB & pcb1 = getPCB(pid);
	stringstream ss(line);
	vector<int> reqs;
	copy(istream_iterator<int>(ss), istream_iterator<int>(), back_inserter(reqs));
	if (reqs.size() != Available.size()) {
		scr_status("输入有误，未分配资源");
	} else {
		auto alloc_bak = pcb1.Alloc;
		auto avail_bak = Available;
		unsigned int i, cnt = 0;
		bool safe = false;
		for (i = 0; i < reqs.size(); ++i) {
			if (reqs[i] > Available[i] || reqs[i] + pcb1.Alloc[i] > pcb1.MAX[i]) {
				pcb1.Alloc = alloc_bak;
				Available = avail_bak;
				scr_status ("资源申请量异常！取消分配");
				return;
			}
			pcb1.Alloc[i] += reqs[i];
			Available[i] -= reqs[i];
			safe = checkSafe(SafeStr);
			if (!safe)
				break;
			if (pcb1.Alloc[i] == pcb1.MAX[i])
				++cnt;
		}
		if (safe) {
			// 释放全部占有资源
			if (cnt == pcb1.MAX.size()) {
				for (unsigned int i = 0; i < pcb1.MAX.size(); ++i)
					Available[i] += pcb1.MAX[i];
				Processes.erase(find(Processes.begin(), Processes.end(), PCB(pid)));
				checkSafe(SafeStr);
				scr_status("进程运行完毕，资源已回收");
			} else {
				scr_status (("安全序列为" + SafeStr).c_str());
			}
		} else {
			pcb1.Alloc = alloc_bak;
			Available = avail_bak;
			scr_status ("不安全！取消分配");
		}
	}
}

void view() {
	scr_gotoxy (25, 2);
	cout << "当前资源: ";
	scr_gotoxy (28, 4);
	[&]{
		for (unsigned int i = 0; i < Available.size(); ++i)
			cout << setw(3) << char('a' + i);
	}();
	scr_gotoxy (28, 5);
	for (auto v : Available)
		cout << setw(3) << (v);
	scr_gotoxy (0, 8);
	scr_line();
	scr_gotoxy (0, 8);
	cout << "+------------------------------+\n";
	cout << "|       资源分配列表           |\n";
	cout << "| id         当前分配/最大需求 |" << endl;
	[&]{
		for (auto p : Processes) {
			cout << "|" << setw (3) << p.pid << setw(26);
			stringstream ss;
			for (unsigned int i = 0; i < p.MAX.size(); ++i)
				ss << " " << p.Alloc[i] << "/" << p.MAX[i];
			cout << ss.str() << " |\n";
		}
	}();
	cout << "+------------------------------+\n";
	scr_gotoxy (32, 8);
	cout << "+---------------------------+";
	scr_gotoxy (32, 9);
	cout << "|         安全序列          |";
	scr_gotoxy (32, 10);
	cout << "| " << setw(25) << SafeStr << " |";
	scr_gotoxy (32, 11);
	cout << "+---------------------------+";
}

void showMenu() {
	while (1) {
		char index1;
		do {
			scr_clear();
			view();
			scr_gotoxy (0, 1);
			cout << "+--------------------+" << endl;
			cout << "| 1.初始化系统资源   |" << endl;
			cout << "| 2.创建进程         |" << endl;
			cout << "| 3.下一次资源分配   |" << endl;
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
			if (Available.empty())
				init();
			else
				scr_status("已存在系统资源，未建立");
			break;
		}
		case '2': {
			if (!Available.empty())
				create();
			else
				scr_status("请先初始化系统资源！");
			break;
		}
		case '3': {
			if (!Available.empty())
				nextTimeslice();
			else
				scr_status("请先初始化系统资源！");
			break;
		}
		case '0':
			return;
		}
	}
}
}

#endif // BANK_H_INCLUDED
