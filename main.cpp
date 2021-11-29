#include <iostream>
#include "env_adapter.h"
#include "menu.h"
using namespace std;

int main() {
	scr_init();
	while(1) {
		showMenu();
	}
	return 0;
}
