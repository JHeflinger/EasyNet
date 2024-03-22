#pragma once
#include "netbasic.h"

EN_STATUS en_hostname(char* name, size_t bufferlength);

EN_STATUS en_hostname(char* name, size_t bufferlength) {
	return gethostname(name, bufferlength) == 0 ? EN_NONE : EN_ERROR;
}
