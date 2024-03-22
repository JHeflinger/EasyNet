#pragma once
#include <stdio.h>

#define RESET "\033[0m"
#define RED "\033[31m"
#define BLUE "\033[34m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define PURPLE "\033[35m"

#define INFO(...)  printf("%s[INFO]%s  ", GREEN, RESET);  printf(__VA_ARGS__); printf("\n");
#define ERROR(...) printf("%s[ERROR]%s ", RED, RESET);    printf(__VA_ARGS__); printf("\n");
#define FATAL(...) printf("%s[FATAL]%s ", RED, RESET);    printf(__VA_ARGS__); printf("\n"); exit(1);
#define WARN(...)  printf("%s[WARN]%s  ", YELLOW, RESET); printf(__VA_ARGS__); printf("\n");
#define DEBUG(...) printf("%s[DEBUG]%s ", BLUE, RESET);   printf(__VA_ARGS__); printf("\n");
#define SCAN(...)  printf("%s[INPUT]%s ", PURPLE, RESET); scanf(__VA_ARGS__);
