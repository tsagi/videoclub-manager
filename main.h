#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
int getch(char *s, int num[]);
