#pragma once
#include <stdio.h>

#define RESET "\033[0m"
#define RED "\033[31m"
#define BLUE "\033[34m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define PURPLE "\033[35m"
#define CYAN "\033[36m"

#define EZN_INFO(...)  {printf("%s[INFO]%s  ", GREEN, RESET);  printf(__VA_ARGS__); printf("\n");}
#define EZN_FATAL(...) {printf("%s[FATAL]%s ", RED, RESET);    printf(__VA_ARGS__); printf("\n"); exit(1);}
#define EZN_WARN(...)  {printf("%s[WARN]%s  ", YELLOW, RESET); printf(__VA_ARGS__); printf("\n");}
#define EZN_DEBUG(...) {printf("%s[DEBUG]%s ", BLUE, RESET);   printf(__VA_ARGS__); printf("\n");}
#define EZN_CUSTOM(precursor, ...) {printf("%s[%s]%s  ", CYAN, precursor, RESET);   printf(__VA_ARGS__); printf("\n");}
#define EZN_SCAN(...)  {printf("%s[INPUT]%s ", PURPLE, RESET); scanf(__VA_ARGS__);}
#define EZN_ASSERT(check, ...) {if (!check) EZN_FATAL(__VA_ARGS__);}