#ifndef USER_CODE_H
#define USER_CODE_H

void user_code(void) __attribute__((section(".user_code")));
int sum(int a, int b) __attribute__((section(".user_code")));

#endif