#ifndef MENU_H_INCLUDED
#define MENU_H_INCLUDED
#include <iostream>
#include <cstdio>
#include "env_adapter.h"
#include "modes.h"
void showMenu2() {
	while (1) {
		char index1;
		do {
			scr_clear();
			scr_gotoxy (0, 2);
			cout << "    +-------------------------+" << endl;
			cout << "    | 1.生产者消费者问题      |" << endl;
			cout << "    | 2.水果盘问题            |" << endl;
			cout << "    | 3.读者写者问题-读者优先 |" << endl;
			cout << "    | 4.读者写者问题-写者优先 |" << endl;
			cout << "    | 5.银行家算法            |" << endl;
			cout << "    | 0.返回上一级            |" << endl;
			cout << "    |-------------------------|" << endl;
			cout << "    | 请输入:                 |" << endl;
			cout << "    +-------------------------+" << endl;
			scr_gotoxy (15, 10);
			index1 = getchar();
			fflush (stdin);
		} while (index1 < '0' || index1 > '6');
		switch (index1) {
		case '1': {
			PROCUS::showMenu();
			break;
		}
		case '2': {
			FRUITS::showMenu();
			break;
		}
		case '3': {
			RW_R::showMenu();
			break;
		}
		case '4': {
			RW_W::showMenu();
			break;
		}
		case '5': {
			BANK::showMenu();
			break;
		}
		case '0':
			return;
		}
	}
}

void showMenu3() {
	while (1) {
		char index1;
		do {
			scr_clear();
			scr_gotoxy (0, 2);
			cout << "    +-----------------+" << endl;
			cout << "    | 1.分区分配      |" << endl;
			cout << "    | 2.分页分配      |" << endl;
			cout << "    | 0.返回上一级    |" << endl;
			cout << "    |-----------------|" << endl;
			cout << "    | 请输入:         |" << endl;
			cout << "    +-----------------+" << endl;
			scr_gotoxy (15, 7);
			index1 = getchar();
			fflush (stdin);
		} while (index1 < '0' || index1 > '2');
		switch (index1) {
		case '1': {
			FENQU::showMenu();
			break;
		}
		case '2': {
			FENYE::showMenu();
			break;
		}
		case '0':
			return;
		}
	}
}

void showMenu4() {

}
void showMenu5() {

}

void showMenu() {
	while (1) {
		char index0;
		do {
			scr_clear();
			scr_gotoxy (0, 3);
			cout << "     +----------------+" << endl;
			cout << "     | 选择一个功能   |" << endl;
			cout << "     |----------------|" << endl;
			cout << "     | 1.进程操作     |" << endl;
			cout << "     | 2.进程相关问题 |" << endl;
			cout << "     | 3.分区和分页   |" << endl;
			cout << "     | 4.页面置换     |" << endl;
			cout << "     | 0.退出程序     |" << endl;
			cout << "     |----------------|" << endl;
			cout << "     | 请输入:        |" << endl;
			cout << "     +----------------+" << endl;
			scr_gotoxy (16, 12);
			index0 = getchar();
			fflush (stdin);
		} while (index0 < '0' || index0 > '4');
		switch (index0) {
		case '1': {
			PROCESS::showMenu();
			break;
		}
		case '2': {
			showMenu2();
			break;
		}
		case '3': {
			showMenu3();
			break;
		}
		case '4': {
			PAGE_REP::showMenu();
			break;
		}
		case '5': {

		}
		case '0':
			exit (0);
		}
	}
}
#endif // MENU_H_INCLUDED
