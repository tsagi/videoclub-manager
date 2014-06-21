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
  struct year *ptr;
};

struct year {
  int year;
  struct year *next;
  struct day *ptr;
};

struct day {
  int day;
  double expenses;
  struct movie *ptr;
  struct day *next;
};

struct movie {
  char id[5];
  struct movie *next;
};


// Functions Prototyping
void intro(void);
void initload(void);
void initSettings(void);
void gatherInput(char *filename);
void menu(void);
void selection1(void);
void insertProcedure(char *s);
void selection2(void);
int select2(void);
void deleteProcedure(int key);
void selection3(void);
void selection4(void);
void selection5(void);
int collectid(char *id);
void displaydetails(char *id);
void month_day(int year, int yearday, int *pmonth, int *pday);
void selection6(void);
int select6_1(void);
int select6_2(int select);
int collectids6(void);
void display6(char *filename, int select1);
void selection7(void);
int select7(void);
void display7(int select, int syear);
void setSettings(void);
void clearScreen(void);
int dayOfTheYear(void);
int hashinsert(char *key);
int hashsearch(char *key, int *collisions);
unsigned hash(char *key, int i);
unsigned djb2hash(char *str);
unsigned sdbmhash(char *str);
void nextprime(int p);
void heapSort(struct rec **htsorted, int array_size, int (*comp)(struct rec *, struct rec *));
void siftDown(struct rec **htsorted, int root, int bottom, int (*comp)(struct rec *, struct rec *));
void heapSortinayear(struct rec **htsorted, int array_size, int year);
void siftDowninayear(struct rec **htsorted, int root, int bottom, int year);
int cmpmovies(struct rec *p1, struct rec *p2);
int countmovies(struct rec *strct);
int cmpexpenses(struct rec *p1, struct rec *p2);
double countexpenses(struct rec *strct);
int cmpexpensesinayear(struct rec *p1, struct rec *p2, int year);
double countexpensesinayear(struct rec *strct, int year);
