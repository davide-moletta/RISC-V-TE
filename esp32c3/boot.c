#include "mdk.h"

void _start(void) __attribute__((section(".machine_setup")));

extern int main(void);
extern char _sbss, _ebss, _end, _eram;

static char *s_heap_start, *s_heap_end, *s_brk;

void *sbrk(int diff) {
  char *old = s_brk;
  if (&s_brk[diff] > s_heap_end) return NULL;
  s_brk += diff;
  return old;
}

void _start(void) {
  s_heap_start = s_brk = &_end, s_heap_end = &_eram;
  for (char *p = &_sbss; p < &_ebss;) *p++ = '\0';
  soc_init();
  main();
  for (;;) (void) 0;
}
