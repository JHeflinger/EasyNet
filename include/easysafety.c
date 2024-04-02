#include "easysafety.h"

EZN_STATUS ezn_safe_clean() {
    EZN_BOOL are_servers_open;
	EZN_BOOL are_clients_open;
    if (ezn_servers_are_open(&are_servers_open) == EZN_NONE) {
        if (are_servers_open == EZN_TRUE) {
            EZN_WARN("Open servers detected - implicitly cleaning");
            if (ezn_clean_servers() == EZN_ERROR) {
                EZN_WARN("Unable to clean servers during safe cleanup");
                return EZN_ERROR;
            }
        }
    } else {
        EZN_WARN("Could not check if any servers were open during safe cleanup");
        return EZN_ERROR;
    }
	if (ezn_clients_are_open(&are_clients_open) == EZN_NONE) {
		if (are_clients_open == EZN_TRUE) {
			EZN_WARN("Connected clients detected - implicitly cleaning");
			if (ezn_clean_clients() == EZN_ERROR) {
				EZN_WARN("Unable to clean clients during safe cleanup");
				return EZN_ERROR;
			}
		}
	} else {
		EZN_WARN("Could not check if any clients were connected during safe cleanup");
		return EZN_ERROR;
	}
    return EZN_NONE;
}
