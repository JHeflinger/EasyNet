#ifndef EASYLOG
#define EASYLOG

#include <stdio.h>

#define EZN_RESET "\033[0m"
#define EZN_RED "\033[31m"
#define EZN_BLUE "\033[34m"
#define EZN_GREEN "\033[32m"
#define EZN_YELLOW "\033[33m"
#define EZN_PURPLE "\033[35m"
#define EZN_CYAN "\033[36m"

#define EZN_INFO(...)  {printf("%s[INFO]%s  ", EZN_GREEN, EZN_RESET);  printf(__VA_ARGS__); printf("\n");}
#define EZN_FATAL(...) {printf("%s[FATAL]%s ", EZN_RED, EZN_RESET);    printf(__VA_ARGS__); printf("\n"); exit(1);}
#define EZN_WARN(...)  {printf("%s[WARN]%s  ", EZN_YELLOW, EZN_RESET); printf(__VA_ARGS__); printf("\n");}
#define EZN_DEBUG(...) {printf("%s[DEBUG]%s ", EZN_BLUE, EZN_RESET);   printf(__VA_ARGS__); printf("\n");}
#define EZN_CUSTOM(precursor, ...) {printf("%s[%s]%s  ", EZN_CYAN, precursor, EZN_RESET);   printf(__VA_ARGS__); printf("\n");}
#define EZN_SCAN(...)  {printf("%s[INPUT]%s ", EZN_PURPLE, EZN_RESET); scanf(__VA_ARGS__);}
#define EZN_ASSERT(check, ...) {if (!check) EZN_FATAL(__VA_ARGS__);}

#endif
