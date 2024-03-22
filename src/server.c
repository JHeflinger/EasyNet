#include "easynet.h"
#include "easylog.h"

int main(int argc, char* argv[]) {
	char name[1024];
	EZN_STATUS status = ezn_hostname(name, 1024);
	if (status == EZN_NONE) {
		EZN_INFO("The hostname is %s", name);
	} else {
		EZN_FATAL("Hostname could not be detected, internal error found");
	}
	return 0;
}
