#ifndef USER_ENTRY_H
#define USER_ENTRY_H

void user_mode_entry_point() __attribute__((section(".user_code")));

#endif