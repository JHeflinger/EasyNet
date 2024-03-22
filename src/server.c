#include "easynet.h"
#include "easylog.h"

int main(int argc, char* argv[]) {
	char name[1024];
	en_hostname(name, 1024);
	INFO("The hostname is %s", name);
	return 0;
}
