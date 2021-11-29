#ifndef ENV_ADAPTER_H_INCLUDED
#define ENV_ADAPTER_H_INCLUDED
#include <iostream>
#include <string>
#include <cstdlib>
using namespace std;

#if defined(__APPLE__) && (defined(__GNUC__) || defined(__xlC__) || defined(__xlc__))
#  define I_OS_MAC
#elif !defined(SAG_COM) && (!defined(WINAPI_FAMILY) || WINAPI_FAMILY==WINAPI_FAMILY_DESKTOP_APP) && (defined(WIN64) || defined(_WIN64) || defined(__WIN64__))
#  define I_OS_WIN
#elif !defined(SAG_COM) && (defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__))
#  define I_OS_WIN
#else
#  define I_OS_LINUX
#endif

#ifdef I_OS_WIN
#include <windows.h>
#endif

string statusbar;

void scr_gotoxy(int x, int y) {
#if defined I_OS_MAC
	"MAC";
#elif defined I_OS_WIN
	HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleCursorPosition(h, COORD{(short)x, (short)y});
#elif defined I_OS_LINUX
	printf("%c[%d;%df", 0x1B, y, x);
#endif
}

void scr_line() {
	cout << "------------------------------------------------------\n";
}

void scr_status(string str) {
	statusbar = str;
	scr_gotoxy(0, 24);
	scr_line();
	cout << statusbar;
}

void scr_clear() {
#if defined I_OS_MAC
	"MAC";
#elif defined I_OS_WIN
	system("mode con:cols=25 lines=7");
	system("cls");
#elif defined I_OS_LINUX
	system("clear");
#endif
	scr_status(statusbar);
}

void scr_init() {
	scr_clear();
	scr_gotoxy(0, 0);
}

#endif // ENV_ADAPTER_H_INCLUDED
