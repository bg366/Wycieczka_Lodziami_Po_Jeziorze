#include <stdio.h>
#include <stdlib.h>

#define RED     "\033[0;31m"
#define GREEN   "\033[0;32m"
#define YELLOW  "\033[0;33m"
#define BLUE    "\033[0;34m"
#define MAGENTA "\033[0;35m"
#define CYAN    "\033[0;36m"
#define WHITE   "\033[0;37m"
#define YELLOW  "\033[0;33m"
#define RESET   "\x1b[0m"

int setup_pamieci(int *N1, int *N2, int *K);
int setup_sternikow(int *Tp, int *Tk, int *T1, int *T2);
