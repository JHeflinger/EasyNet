#pragma once
#include "netbasic.h"
#include "easyserver.h"

EZN_STATUS ezn_safe_clean();

EZN_STATUS ezn_safe_clean() {
    EZN_BOOL are_servers_open;
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
    return EZN_NONE;
}