#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#if defined(_WIN32) || defined(_WIN64)
    #define OS_WIN 1
    #include <windows.h>
    #include <conio.h>
#else
    #define OS_WIN 0
    #include <unistd.h>
    #include <ncurses.h>
    #include <curses.h>
#endif

// Declarations
struct rec {
	char id[26];
	struct year *ptr; };

struct year {
	int year;
	struct year *next;
	struct day *ptr; };

struct day {
	int day;
	int enterprise;
	double expenses;
	struct comserv *ptr;
	struct day *next; };

struct comserv {
	char id[5];
	struct comserv *next; };

void intro(void);
void initload(void);
void menu(void);
void gatherfilename(char *filename);
const char* getfield(char* l, int n);
void insertprocedure(char *s);
void clearScreen(void);
void setSettings(void);
void initSettings(void);
