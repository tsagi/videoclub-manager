#include "main.h"

int ahashsize, settings, temp_set;
int collisions;
int entries;
int numentrfile;

struct rec **ahash, **htsorted;
struct rec deleted; /* For every entry in the hash table that is being removed,
                     * its pointer is going to point to deleted */

// ncurses
int h, w;

int main(int argc, char *argv[]){
    enum {NewFile = '1', DeleteCards = '2', Collisions= '3' ,
        LoadFactor = '4', IdDetails = '5', ManyId = '6',
        TopId = '7', Settings = '8', Exit = '9'};

    int select;
    deleted.ptr = NULL;

    initscr();
    noecho();
    getmaxyx(stdscr, h, w);
    intro();
    
    initload();

    menu();
    refresh();

    while ((select = getch()) != Exit) {
        switch ( select ) {
            case NewFile:
                selection1();
                break;
            case DeleteCards:
                selection2();
                break;
            case Collisions:
                selection3();
                break;
            case LoadFactor:
                selection4();
                break;
            case IdDetails:
                selection5();
                break;
            case ManyId:
                selection6();
                break;
            case TopId:
                selection7();
                break;
            case Settings:
                setSettings();
                break;
            default:
                printw("    Invalid option..."); /****************************/
                refresh();                       /* Display that the option  */
                sleep(1);                        /* is invalid for a second. */
                move(h-1, 8);                    /* This can be interupted   */
                clrtoeol();                      /*by any input.             */
                refresh();                       /****************************/
                continue;
        }
        menu();
        refresh();

    }
    endwin();
    return 0;
}

void initload(void) {
    // initload: takes the name of the file containing the purchases and creates the hash table
    FILE *fp;
    char filename[30];

    initSettings();   // Set the option for the probe sequences
    do {
        clearScreen();
        move(h-3,0);

        printw("Type in the name of the file containing the records of purchases\n");
        printw("[Note: Type the name without its file extension]\n");
        gatherInput(filename);   // Take the name of the file with the purchases
        fp = fopen(filename, "r");   // Error checking
        if ( fp != NULL ) break;
        clearScreen();
        move(h-2,0);
        printw("The file %s was not accepted.\nType in the name again.", filename);
        refresh();
    } while ( fp == NULL );
    fclose(fp);
    insertProcedure(filename);   // Initiate the procedure for getting the information
}

void menu(void) {
    // menu: displays the main menu of the program
    clearScreen();
    move(h-11, 0);

    printw(" Menu:\n");
    printw("  1. Insert a new file of Purchases\n");
    printw("  2. Delete cards in the hash table\n");
    printw("  3. Display the number of collisions\n");
    printw("  4. Display the load factor in the hash table\n");
    printw("  5. Display the details of a card ID\n");
    printw("  6. Search for data about a card or a set of cards\n");
    printw("  7. Display the cards with the highest usage\n");
    printw("  8. Settings\n");
    printw("  9. Exit\n");
    printw("Select: ");
}

void selection1(void) {
    // selection1: takes a new file with purchases, deletes the current hash table and creates a new one
    FILE *fp;
    int cert, key;
    char filename[30], id[26];

    clearScreen();
    move(h-2,0);
    printw("Inserting a new file is going to delete every record in this hash table.\n");
    printw("Are you sure you want to proceed? (y/n): ");
    refresh();
    do {   // Error checking
        cert = toupper(getch());
        refresh();
    } while ( cert != 'Y' && cert != 'N' );
    if ( cert == 'N' ) {
        clearScreen();
        move(h-1,0);
        printw("[Press any key to continue] ");
        getch();   // Wait
        refresh();
        return;
    }   // Return to the main menu
    clearScreen();
    move(h-2,0);
    printw("Type in the name of the file containing the records of purchases\n");
    printw("[Note: Type the name without its file extension]");
    refresh();
    gatherInput(filename);   // Take the filename
    fp = fopen(filename, "r");   // Error checking
    if ( fp == NULL ) {
        clearScreen();
        move(h-2,0);
        printw("The file %s was not accepted.\n", filename);
        printw("[Press any key to continue] ");
        getch();   // Wait
        refresh();
        return; }
    fclose(fp);
    // The filename is valid. Now, delete everything in memory...
    clearScreen();
    move(h-1,0);
    printw("Deleting... ");
    refresh();
    fp = fopen("id_record.txt", "r");
    while ( !feof(fp) ) {
        fgets(id,26,fp);
        key = hashsearch(id, NULL);
        if ( key == -1 ) continue;   // If this card has already been deleted, then move on
        deleteProcedure(key);
    }
    fclose(fp);
    free(ahash);   // free the hash table
    free(htsorted);   // free the sorted table
    collisions = 0;   // Restore the counters
    entries = 0;
    clearScreen();
    move(h-1,0);
    printw("Done!");
    refresh();

    insertProcedure(filename);   // ... and initiate the inserting procedure
}

void insertProcedure(char *filename) {
    // insertprocedure: builds the entire hash table
    FILE *fp, *fp2;
    char *line, *data, s1[300], s2[26];
    int i, j, key, day, year, entcode, final;
    double expenses;
    struct year *listptr;
    struct day *dtemp;
    struct prod *ptemp;

    // Build the hash table
    line = s1, data = s2;   // It wouldn't take this definition: char (*line)[200], (*data)[26];
    fp = fopen(filename, "r");   // Open the "filename" for reading
    clearScreen();
    move(h-2,0);
    if ( fp == NULL ) {   // Error checking
        move(h-2,0);
        printw("Error: %s was not opened.", filename);
        refresh();
        return;
    } else
        printw("%s was accepted.\nRunning... ", filename);
    refresh();
    fp2 = fopen("id_record.txt", "w");   // Keep the IDs as a backup, in case the table needs to be deleted
    settings = temp_set;   // Set settings
    fgets(line, 200, fp);   // Take the first line (Number of different cards IDs)
    numentrfile = atoi(line);
    nextprime( 2 * numentrfile );   // Define the size of the hash table
    ahash = (struct rec **) malloc( ahashsize * sizeof(struct rec *) ); // Allocate space for it
    htsorted = (struct rec **) malloc( numentrfile * sizeof(struct rec *) ); // Allocate space for the sorted table
    for ( i = 0; i < ahashsize; i++ ) {
        ahash[i] = NULL;   // Initialize the hash table
    }
    // Start reading lines (purchases)
    while ( !feof(fp) ) {   // While you haven't met the end-of-file, act as follows
        fgets(line, 200, fp);   // Take a line from the file
        i = 0;
        final = 1;   // It has to do with the final product in the line (Explained later)
        while ( isspace(*(line + i)) ) i++;   // Ignore any white spaces, and in the first non-
                                              // white space character that will be received, move on
        for ( j = 0; *(line + i) != ';'; i++, j++ )   // Collect ID
            *(data + j) = *(line + i);                // Copy the ID to data
        *(data + j) = '\0';   // null-terminate data
        key = hashinsert(data);   // Insert the ID and get the key to the table
        fprintf(fp2,data);   // Write down the ID to the backup file
        i++;   // Ignore the ';'
        for ( j = 0; *(line + i) != ';'; i++, j++ )   // Collect the day of the year
            *(data + j) = *(line + i);   // Copy the day to data
        *(data + j) = '\0';   // null-terminate data
        day = atoi(data);   // Hold the day
        i++;   // Ignore the ';'
        for ( j = 0; *(line + i) != ';'; i++, j++ )   // Collect the year
            *(data + j) = *(line + i);   // Copy the year to data
        *(data + j) = '\0';   // null-terminate data
        year = atoi(data);   // Hold the year
        i++;   // Ignore the ';'
        for ( j = 0; *(line + i) != ';'; i++, j++ )   // Collect the expenses
            *(data + j) = *(line + i);   // Copy the expenses to data
        *(data + j) = '\0';   // null-terminate data
        expenses = atof(data);   // Hold the expenses
        i++;   // Ignore the ';'
        listptr = ahash[key]->ptr;
        if ( listptr == NULL ) {   // If there is no list... start one
            listptr = (struct year *) malloc(sizeof(struct year));   // Build a struct year
            listptr->year = year;   //Set the year
            listptr->next = NULL;
            listptr->ptr = (struct day *) malloc(sizeof(struct day));   // Build a struct day
            listptr->ptr->day = day;   // Set the day
            listptr->ptr->expenses = expenses;   // Set the expenses
            listptr->ptr->ptr = NULL;   // Initialize pointer to prod
            listptr->ptr->next = NULL;
            ahash[key]->ptr = listptr;   // Make listptr's pointed struct the head of the list of years
        } else {   // Otherwise, a list is going to be there, and then...
            do {
                if ( listptr->year == year ) {   // ... check the year values... (if positive)
                    dtemp = (struct day *) malloc(sizeof(struct day));   // Build a struct day
                    dtemp->day = day;   // Set the day
                    dtemp->expenses = expenses;   // Set the expenses
                    dtemp->ptr = NULL;   // Initialize pointer to prod
                    dtemp->next = listptr->ptr;   // Make dtemp to point to the head of the list of days
                    listptr->ptr = dtemp;   // Make dtemp's pointed struct the new head of the list of days
                    break;
                } else
                    listptr = listptr->next;   // ... and if they don't match, check the next element of the list
            } while ( listptr != NULL );
            if ( listptr == NULL ) {   // At the end, if the year wasn't in the list, add it
                listptr = (struct year *) malloc(sizeof(struct year));   // Build a struct year
                listptr->year = year;   // Set the year
                listptr->next = ahash[key]->ptr;   // Make listptr to point to the head of the list of years
                listptr->ptr = (struct day *) malloc(sizeof(struct day));   // Build a struct day
                listptr->ptr->day = day;   // Set the day
                listptr->ptr->expenses = expenses;   // Set the expenses
                listptr->ptr->ptr = NULL;   // Initialize pointer to prod
                listptr->ptr->next = NULL;
                ahash[key]->ptr = listptr;   // Make listptr's pointed struct the new head of the list of years
            }
        }
        // By now we have created everything but the list of products and services. The pointer for that list is
        // listptr->ptr->ptr, and it's going to be used right away
        for ( j = 0; (*(line + i) != '\0' && *(line + i) != '\n') ||
                (final-- && (*(line + i) = ';') && (*(line + i + 1) = '\n')); i++, j++ ) {
            /* When last product apears, after the program reads its name, the first expresion is
             * going to fail, but the second one will hold the loop for one more iteration.
             * (final-- && *(line + i) = ';' && *(line + i + 1) = '\n'): Here, the first command decrements
             * the variable final and on the second iteration will fail the whole expression, the second one
             * makes sure that control will pass the if statement inside the loop, and the third one will
             * terminate the loop on the second iteration */
            if ( *(line + i) == ';' ) {   // When you've got the product ID
                *(data + j) = '\0';   // null-terminate data
                ptemp = (struct prod *) malloc(sizeof(struct prod));  // Allocate space for a struct prod
                strcpy(ptemp->id,data);   // Copy data to id
                ptemp->next = listptr->ptr->ptr;   // Make ptemp to point to the head of the list
                listptr->ptr->ptr = ptemp;   // Make ptemp's pointed struct the new head of the list
                j = -1;   // Prepare for the new product or service to come
                continue;
            }
            *(data + j) = *(line + i);   // Copy the product/service to data
        }
    }
    fclose(fp);
    fclose(fp2);
    clearScreen();
    move(h-2,0);
    printw("Done!\n");
    printw("[Press any key to continue] ");
    refresh();
    getch();   // Wait
}

void selection2(void) {
    // selection2: gets the name of the file containing the cards for deletion, and then deletes them from the ahash
    FILE *fp;
    int key, count = 0, colls = 0, numdels;
    char filename[30], id[28];

    if ( !select2() ) return;   // Return to the main menu
    clearScreen();
    move(h-2,0);
    printw("Type in the name of the file containing the IDs of the cards to delete.");
    printw("[Note: Type the name without its file extension]");
    refresh();
    gatherInput(filename);   // Take the filename
    fp = fopen(filename, "r");   // Error checking
    clearScreen();
    move(h-2,0);
    if ( fp == NULL ) {
        printw("The file %s was not accepted.\n", filename);
        printw("[Press any key to continue] ");
        refresh();
        getch();   // Wait
        return;
    }
    printw("%s was accepted.\n", filename);
    refresh();
    fgets(id,28,fp);   // Ignore the first line for now
    while ( !feof(fp) ) {   // While you haven't met the end-of-file, act as follows
        fgets(id,28,fp);   // Take an ID from the file
        id[25] = '\0';   // Replace '\n'
        key = hashsearch(id, &colls);   // Search for the ID
        if ( key == -1 ) {
            clearScreen();
            move(h-1,0);
            printw("     Error: %s does not exist.", id);
            refresh();
            continue; }
        deleteProcedure(key);   // Delete all the structs related
        entries--;   // Decrement total entries
        collisions -= colls;  // Subtract the number of collisions concerned with the id
        colls = 0;   // Restore the counter
        count++;
    }
    rewind(fp);
    fgets(id,28,fp);   // Read the first line again
    fclose(fp);
    numdels = atoi(id);   // Hold the number of requests for deletions
    clearScreen();
    move(h-2,0);
    printw("There have been %d successful deletions out of %d requests.\n", count, numdels);
    printw("[Press any key to continue] ");
    refresh();
    getch();   // Wait
}

int select2(void) {
    // select2: gets an ID from the user, and deletes every struct in the ahash related with this ID
    // returns: 0 for returning t the main menu, or 1 for the second selection of the secondary menu
    int c, key, colls = 0;
    char id[26];
    clearScreen();
    move(h-5,0);
    printw("         [Press 1 or 2 for the choices]\n");
    printw("  2.1. Delete one specific card (Insert the ID from the keyboard)\n");
    printw("  2.2. Delete a set of cards (Insert the IDs from a file)\n");
    printw("  2.3. Return to the main menu\n");
    printw("Select: ");
    refresh();

    while( 1 ) {
        c = getch();
        if ( c != '1' && c != '2' && c != '3' ) continue;
        putchar(c);
        if ( c == '3' ) {
            return 0; }   // Return to the main menu
        else if ( c == '2' ) return 1;
        else break;
    }
    clearScreen();
    move(h-2,0);
    printw("[Note: Press Escape to abort]\n");
    printw("Insert the card ID to delete");
    refresh();
    if ( collectid(id) != 27 ) {   // Get the card ID, and if Escape hasn't been pressed, continue
        key = hashsearch(id, &colls);   // Search for the ID
        if ( key == -1 ) {   // Error checking
            clearScreen();
            move(h-2,0);
            printw("     Error: %s does not exist.\n", id);
            printw("[Press any key to continue] ");
            refresh();
            getch();   // Wait
            return 0;   // Return to the main menu
        }
        deleteProcedure(key);   // Delete the entry
        entries--;   // Decrement total entries
        collisions -= colls;  // Subtract the number of collisions concerned with the id
        clearScreen();
        move(h-2,0);
        printw("There has been 1 successful deletion.");
    }
    printw("[Press any key to continue] ");
    refresh();
    getch();   // Wait
    return 0;   // Return to the main menu
}

void deleteProcedure(int key) {
    // deleteprocedure: deletes every struct related with the entry of the ahash, being in the position key
    struct year *yptr, *p;
    struct day *dptr, *q;
    struct prod *pptr, *r;

    for ( yptr = ahash[key]->ptr; yptr != NULL; yptr = p ) {
        // free the list of years
        for ( dptr = yptr->ptr; dptr != NULL; dptr = q ) {
            // free the list of days of the current year
            for ( pptr = dptr->ptr; pptr != NULL; pptr = r ) {
                // free the list of products and services of the current day of the current year
                r = pptr->next;
                free(pptr);
            }
            q = dptr->next;
            free(dptr);
        }
        p = yptr->next;
        free(yptr);
    }
    free(ahash[key]);   // free the current rec of the hash table
    ahash[key] = &deleted;   // Signify the cell as one that there's been a deletion on
}

void selection3(void) {
    // selection3: displays the number of collisions in the ahash, and the number of entries
    clearScreen();
    move(h-3,0);
    printw("The number of collisions in the hash table\n");
    printw("\tis %d out of %d different entries.\n", collisions, entries);
    printw("[Press any key to continue] ");
    refresh();
    getch();   // Wait
}

void selection4(void) {
    // selection4: displays the load factor of the ahash
    clearScreen();
    move(h-2,0);
    printw("The load factor is %.3f.\n", (double) entries / ahashsize);
    printw("[Press any key to continue] ");
    refresh();
    getch();   // Wait
}

void selection5(void) {
    // selection5: handles the selection 5 of the main menu
    char id[26];
    clearScreen();
    move(h-2,0);
    printw("[Note: Press Escape to abort]");
    printw("Insert the ID and the details will appear!");
    refresh();
    if ( collectid(id) != 27 )   // Get the card ID, and if Escape hasn't been pressed, continue
        displaydetails(id);   // The card ID has been received. Now display the details
    printw("[Press any key to continue] ");
    refresh();
    getch();   // Wait
}

int collectid(char *id) {
    // collectid: writes, the ID entered, in the variable id
    // returns: 'escape' in a cancellation, or 1 on success
    int c, i = 0;
    clearScreen();
    move(h-2,0);
    printw(" Type here: ");
    refresh();
    while ( c = getch() ) {
        if ( c == 27 ) {
            return c; }   // If Escape was pressed, return
        if ( isspace(c) ) continue;   // Ignore any white spaces
        if ( c == '\b' ) {   // If backspace is pressed, cancel and start over
            printw("   X\nType again: ");
            i = 0;
            continue; }
        if ( i < 24 ) {
            id[i++] = c;
        } else {   // When the id has been almost entered
            id[i++] = c;
            id[i] = '\0';
            break;
        }
    }
    printw("  OK!");
    refresh();
    return 1;
}

void displaydetails(char *id) {
    // displaydetails: displays all the information in memory about the card with the ID id
    int i, key, month, day, usage = 0, t_usage;
    double exptotal = 0, t_exp;
    char *end;
    static char *month_name[] = { "Illegal month", "January", "February", "March", "April", "May",
        "June", "July", "August", "September", "October", "November", "December" };
    struct year *yptr;
    struct day *dptr;
    struct prod *pptr;

    key = hashsearch(id, NULL);
    clearScreen();
    if ( key == -1 ){
        move(h-1,0);
        printw("   Failure: %s does not exist.\n", id);
        refresh();
    }
    else {
        move(h-19,0);
        printw("               ID: %s\n", ahash[key]->id);
        printw("============================================\n");
        yptr = ahash[key]->ptr;
        while ( yptr != NULL ) {
            dptr = yptr->ptr;
            t_usage = 0;
            t_exp = 0;
            printw("             Year: %d\n", yptr->year);
            printw("=======================\n");
            while ( dptr != NULL ) {
                pptr = dptr->ptr;
                month_day(yptr->year, dptr->day, &month, &day);
                if ( day == 1 ) end = "st";
                else if ( day == 2 ) end = "nd";
                else if ( day == 3 ) end = "rd";
                else end = "th";
                printw("             Date: %s %d%s\n", month_name[month], day, end);
                printw("         Expenses: %.2f Euro\n", dptr->expenses);
                printw("Products/Services:");
                i = 0;
                while ( pptr != NULL ) {
                    switch ( i ) {
                        case 0:
                            i++;
                            break;
                        default:
                            if ( i++ % 10 == 0 ) printw("\n\t\t  ");
                    }
                    printw(" %s", pptr->id);
                    pptr = pptr->next;
                }
                printw("\n");
                t_usage++;
                t_exp += dptr->expenses;
                dptr = dptr->next;
            }
            printw(">>>> In %d, the expenses total was $%.2f\n", yptr->year, t_exp);
            printw(">>>> and the card was used %d times\n", t_usage);
            usage += t_usage;
            exptotal += t_exp;
            yptr = yptr->next;
        }
        printw("====================================\n");
        printw("The total money spent are: $%.2f\n", exptotal);
        printw("The card has been used %d times\n", usage);
        refresh();
    }
}

void month_day(int year, int yearday, int *pmonth, int *pday) {
    // month_day: takes the day of the year and returns the month and the day of the month
    int i, leap;
    static char daytab[2][13] = { {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},   // Non-leap years
        {0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31} };   // Leap years

    leap = year%4 == 0 && year%100 != 0 || year%400 == 0;
    for (i = 1; yearday > daytab[leap][i]; i++)
        yearday -= daytab[leap][i];
    *pmonth = i;
    *pday = yearday;
}

void selection6(void) {
    // selection6: takes the choice of the menu about what data the user wants to be displayed
    // and also gives the choice to either enter at the moment the card IDs or insert them from a file
    int select1, select2;
    char filename[30];

    select1 = select6_1();   // Take the first choice
    if ( select1 == '5' ) {
        return; }   // Return to the main menu
    select2 = select6_2(select1);   // Take the second choice
    if ( select2 == '3' ) {
        return; }   // Return to the main menu
    if ( select2 == '1' ) {   // If insertion from the keyboard was selected
        if ( !collectids6() ) {   // Collect the card IDs...
            printw("\n[Press any key to continue] ");
            refresh();
            getch();   // Wait
            return;
        }   // If no IDs were entered, return to the main menu
        display6("temp_ids.txt", select1);   // ... and display the data about them
    } else {   // If insertion from a file was selected
        printw("[Note: Type the name without its file extension]\n");
        refresh();
        gatherInput(filename);   // Take the name of the file...
        display6(filename, select1);   // ... and display the data about the card IDs in the file
    }
}

int select6_1(void){
    // select6_1: displays the primary menu of the selection 6 of the main menu
    int c;

    clearScreen();
    move(h-7,0);
    printw("         [Press 1 through 5 for the choices]\n");
    printw("  6.1. Display the set of products and services purchased\n");
    printw("  6.2. Display the expenses total\n");
    printw("  6.3. Display the complete movement\n");
    printw("  6.4. Display the most desirable product or service\n");
    printw("  6.5. Return to the main menu");
    printw("Select: ");
    refresh();

    while( 1 ) {
        switch( c = getch() ) {   // Error Checking
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
                break;
            default:
                continue;
        }
        putchar(c);
        return c;
    }
}

int select6_2(int select1) {
    // select6_2: displays the secondary menu of the selection 6 of the main menu
    int c;
    clearScreen();
    move(h-5,0);
    printw("         [Press 1 or 2 for the choices]\n");
    printw("  6.%c.1. Insert the card IDs from the keyboard\n", select1);
    printw("  6.%c.2. Insert the card IDs from a file\n", select1);
    printw("  6.%c.3. Return to the main menu\n\n", select1);
    printw("Select: ");
    refresh();

    while( 1 ) {
        c = getch();
        if ( c != '1' && c != '2' && c != '3' ) continue;   // Error Checking
        putchar(c);
        if ( c == '2' ) printw("\n");
        return c;
    }
}

int collectids6(void) {
    // collectids6: collects card ids from the input and stores them in a file
    int c, i = 0, count = 0;
    char id[26];
    FILE *fp;

    fp = fopen("temp_ids.txt", "w");   // Open a file for writing
    fwrite("Ignore this line\n", sizeof(char), 17, fp);   // Write a line to comply with the pattern
    clearScreen();
    move(h-2,0);
    printw("[Note: Press Enter when you are done]\n");
    printw("Type here: ");
    refresh();
    while ( (c = getch()) != '\r' ) {
        if ( c == ' ' || c == '\t' ) continue;   // Ignore any spaces or tabs
        if ( c == '\b' ) {   // If backspace is pressed, cancel and start over
            clearScreen();
            move(h-1,0);
            printw("Type again: ");
            refresh();
            i = 0;
            continue; }
        if ( i < 24 ) {   // Note the characters
            id[i++] = c;
        } else {   // When the id has been almost entered
            id[i++] = c;
            id[i] = '\n';
            i = 0;   // Rewind
            count++;   // Increment the number of cards to be deleted
            fwrite(id, sizeof(char), 26, fp);   // Write down the id
            clearScreen();
            move(h-2,0);
            printw("  OK!\n  Next: ");
            refresh();
            continue;
        }
    }
    clearScreen();
    move(h-1,0);
    if ( i != 0 ) printw("  Failed!\n");
    else printw(" -\n");
    refresh();
    clearScreen();
    move(h-2,0);
    printw("\nYou entered %d IDs.\n", count);
    refresh();
    if ( !count ) {
        printw("Aborting...");
        refresh();
        return 0; }
    fclose(fp);
    return count;
}

void display6(char *filename, int select) {
    // display6: takes the choice of the user and displays the corresponding data
    FILE *fp;
    int i, j, key, month, day, usage = 0, t_usage;
    int pns[1000], max, posi, posj, allmax = 0, allposi = 0, allposj = 0;
    double exptotal = 0, t_exp;
    char id[28], *end, *maxid;
    static char *month_name[] = { "Illegal month", "January", "February", "March", "April", "May",
        "June", "July", "August", "September", "October", "November", "December" };
    struct year *yptr;
    struct day *dptr;
    struct prod *pptr;

    fp = fopen(filename, "r");
    clearScreen();
    if ( fp == NULL ) {
        move(h-2,0);
        printw("\nError: %s was not accepted.\n", filename);
        printw("\n[Press any key to continue] ");
        refresh();
        getch();   // Wait
        return; 
    }
    fgets(id, 28, fp);   // Take an ID from the file
    id[25] = '\0';   // Replace '\n'
    while ( !feof(fp) ) {   // While you haven't met the end-of-file, act as follows
        key = hashsearch(id, NULL);   // Search for the ID
        if ( key == -1 ) {   // If the id is not in the table
            printw("\n            Error: %s does not exist.\n\n", id);
            fgets(id, 28, fp);   // Take the next ID from the file
            id[25] = '\0';   // Replace '\n'
            continue; }
        switch ( select ) {
            case '1':
                clearScreen();
                move(h-11,0);
                printw("               ID: %s\n", ahash[key]->id);
                printw("============================================\n");
                printw("Products/Services:\n");
                yptr = ahash[key]->ptr;
                i = 0;
                while ( yptr != NULL ) {
                     dptr = yptr->ptr;
                    while ( dptr != NULL ) {
                        pptr = dptr->ptr;
                        while ( pptr != NULL ) {
                            switch ( i ) {
                                case 0:
                                    i++;
                                    break;
                                default:
                                    if ( i++ % 10 == 0 ) printw("\n\t\t  ");
                            }
                            printw(" %s", pptr->id);
                            pptr = pptr->next;
                        }
                        dptr = dptr->next;
                    }
                    yptr = yptr->next;
                }
                refresh();
                break;
            case '2':
                clearScreen();
                move(h-11,0);
                printw("               ID: %s\n", ahash[key]->id);
                printw("============================================\n");
                yptr = ahash[key]->ptr;
                t_usage = 0;
                t_exp = 0;
                while ( yptr != NULL ) {
                    dptr = yptr->ptr;
                    while ( dptr != NULL ) {
                        t_usage++;
                        t_exp += dptr->expenses;
                        dptr = dptr->next;
                    }
                    yptr = yptr->next;
                }
                exptotal += t_exp;
                printw("   Expenses Total: $%.2f\n", t_exp);
                printw("       Times Used: %d", t_usage);
                refresh();
                break;
            case '3':
                clearScreen();
                move(h-14,0);
                printw("               ID: %s\n", ahash[key]->id);
                printw("============================================\n");
                yptr = ahash[key]->ptr;
                usage = 0;
                exptotal = 0;
                while ( yptr != NULL ) {
                    dptr = yptr->ptr;
                    t_usage = 0;
                    t_exp = 0;
                    printw("             Year: %d\n", yptr->year);
                    printw("=======================\n");
                    while ( dptr != NULL ) {
                        pptr = dptr->ptr;
                        month_day(yptr->year, dptr->day, &month, &day);
                        if ( day == 1 ) end = "st";
                        else if ( day == 2 ) end = "nd";
                        else if ( day == 3 ) end = "rd";
                        else end = "th";
                        printw("             Date: %s %d%s\n", month_name[month], day, end);
                        printw("         Expenses: $%.2f\n", dptr->expenses);
                        printw("Products/Services:");
                        i = 0;
                        while ( pptr != NULL ) {
                            switch ( i ) {
                                case 0:
                                    i++;
                                    break;
                                default:
                                    if ( i++ % 10 == 0 ) printw("\n\t\t  ");
                            }
                            printw(" %s", pptr->id);
                            pptr = pptr->next;
                        }
                        refresh();
                        t_usage++;
                        t_exp += dptr->expenses;
                        dptr = dptr->next;
                    }
                    printw("\n>>>> In %d, the expenses total was $%.2f\n", yptr->year, t_exp);
                    printw(">>>> and the card was used %d times\n", t_usage);
                    usage += t_usage;
                    exptotal += t_exp;
                    yptr = yptr->next;
                }
                printw("====================================\n");
                printw("The total money spent are: $%.2f\n", exptotal);
                printw("The card has been used %d times.\n", usage);
                refresh();
                break;
            case '4':
                clearScreen();
                move(h-14,0);
                printw("               ID: %s\n", ahash[key]->id);
                printw("============================================\n");
                for ( i = 0; i < 100; i++ ) pns[i] = 0;
                yptr = ahash[key]->ptr;
                while ( yptr != NULL ) {
                    dptr = yptr->ptr;
                    while ( dptr != NULL ) {
                        pptr = dptr->ptr;
                        while ( pptr != NULL ) {
                                pns[atoi(pptr->id + 1)]++;
                            }
                            pptr = pptr->next;
                        }
                        dptr = dptr->next;
                    }
                    yptr = yptr->next;
                }
                max = pns[0];
                // In case several products or services have the same number of purchases, then the p. or s. that
                // is going to be considered as the most desirable is the one with the smallest consecutive number
                for ( i = 0; i < 1000; i++ ){
                        if ( pns[i] > max ) {
                            max = pns[i];
                            posi = i;
                        }
                printw("        ProductID: %s%d\n", posi < 100 ? (posi < 10 ? "00": "0"): "",posi);
                printw("        Purchased: %d times\n", max);
                if ( max > allmax ) {
                    allmax = max;
                    allposi = posi;
                    maxid = ahash[key]->id;
                }
        }
        refresh();
        fgets(id, 28, fp);   // Take the next ID from the file
        id[25] = '\0';   // Replace '\n'
    }
    fclose(fp);
    switch ( select ) {
        case '2':
            clearScreen();
            move(h-3,0);
            printw("===================================================\n");
            printw("All the cardholders together have spent: $%.2f\n", exptotal);
            break;
        case '4':
            clearScreen();
            move(h-8,0);
            printw("_____________________________________________________________\n");
            printw("=============================================================\n");
            printw("The most desirable product/service among all the cardholders:\n");
            printw("============================================\n");
            printw("               ID: %s\n", maxid);
            printw("Product/ServiceID: %c%s%d\n",allposi?'S':'P',(allposj<100)?(allposj<10)?"00":"0":"",allposj);
            printw("        Purchased: %d times\n", allmax);
    }
    printw("[Press any key to continue] ");
    refresh();
    getch();   // Wait
}

void selection7(void) {
    // selection7: gets the choice of the user and displays the data accordingly
    int select, year = 0;
    char syear[5];

    select = select7();   // Get the choice of the user
    if ( select == '4' ) return;   // Return to the main menu
    if ( select == '3' ) {
        clearScreen();
        move(h-1,0);
        printw("Specify the year: ");
        refresh();
        scanw("%s", &syear);   // Get the desired year to be displayed
        year = atoi(syear);
    }
    display7(select, year);   // Display the data
}

int select7(void) {
    // select7: takes the choice of the menu from the user
    int c;
    clearScreen();
    move(h-1,0);
    printw("         [Press 1 through 4 for the choices]\n");
    printw("  7.1. Display those with the most products/services purchased\n");
    printw("  7.2. Display those with the biggest expenses in general\n");
    printw("  7.3. Display those with the biggest expenses in a year\n");
    printw("  7.4. Return to the main menu\n");
    printw("Select: ");
    refresh();
    while(1) {
        switch( c = getch() ) {   // Error Checking
            case '1':
            case '2':
            case '3':
            case '4':
                break;
            default:
                continue;
        }
        return c;
    }
}

void display7(int select, int year) {
    // display7: displays data of the best buyers
    int num, i, j;
    double expenses;
    char snum[10];
    struct year *yptr;
    struct day *dptr;
    struct prod *pptr;
    clearScreen();
    move(h-3,0);
    printw("Insert the number of cards");
    printw("you would like to be displayed (from 1 to %d)\n", numentrfile);
    printw(" Type here [Press Enter when you are done]: ");
    scanw("%d", &num);   // Take the number of the cards to be displayed
    refresh();
    while ( num < 1 || num > numentrfile ) {   // Range checking
        move(h-1,0);
        printw(" Out of range. Please try again: ");
        refresh();
        scanw("%s", snum);
        num = atoi(snum);
    }

    clearScreen();
    move(h-11,0);
    switch ( select ) {   // Entitle the procedure
        case '1':
            printw("Best Buyers regarding their Possessions\n");
            printw("=======================================\n");
            break;
        case '2':
            printw("Best Buyers regarding their Expenses\n");
            printw("====================================\n");
            break;
        case '3':
            printw("\n\nBest Buyers regarding their Expenses in Year %d\n", year);
            printw("=================================================\n");
            break;
    }
    refresh();
    move(h-9,0);
    switch ( select ) {
        case '1':
            heapSort(htsorted, numentrfile, cmpproducts);   // Sort the IDs according to their products/services
            for ( i = numentrfile - 1; i > numentrfile - 1 - num; i-- ) {
                printw("               ID: %s\n", htsorted[i]->id);
                printw("============================================\n");
                printw("            Count: %d\n",countproducts(htsorted[i]));
                printw("Products/Services:");
                yptr = htsorted[i]->ptr;
                j = 0;
                while ( yptr != NULL ) {
                    dptr = yptr->ptr;
                    while ( dptr != NULL ) {
                        pptr = dptr->ptr;
                        while ( pptr != NULL ) {
                            switch ( j ) {
                                case 0:
                                    j++;
                                    break;
                                default:
                                    if ( j++ % 10 == 0 ) printw("\n\t\t  ");
                            }
                            printw(" %s", pptr->id);
                            pptr = pptr->next;
                        }
                        dptr = dptr->next;
                    }
                    yptr = yptr->next;
                }
            }
            printw("\n");
            refresh();
            break;
        case '2':
            heapSort(htsorted, numentrfile, cmpexpenses);   // Sort the IDs according to their expenses
            for ( i = numentrfile - 1; i > numentrfile - 1 - num; i-- ) {
                printw("               ID: %s\n", htsorted[i]->id);
                printw("============================================\n");
                yptr = htsorted[i]->ptr;
                expenses = 0;
                while ( yptr != NULL ) {
                    dptr = yptr->ptr;
                    while ( dptr != NULL ) {
                        expenses += dptr->expenses;
                        dptr = dptr->next;
                    }
                    yptr = yptr->next;
                }
                printw("   Expenses Total: $%.2f", expenses);
            }
            refresh();
            break;
        case '3':
            heapSortinayear(htsorted, numentrfile, year);   // Sort the IDs according to their expenses in the year specified
            for ( i = numentrfile - 1; i > numentrfile - 1 - num; i-- ) {
                printw("               ID: %s\n", htsorted[i]->id);
                printw("============================================\n");
                yptr = htsorted[i]->ptr;
                expenses = 0;
                while ( yptr != NULL ) {
                    if ( yptr->year != year ) {
                        yptr = yptr->next;
                        continue; }
                    dptr = yptr->ptr;
                    while ( dptr != NULL ) {
                        expenses += dptr->expenses;
                        dptr = dptr->next;
                    }
                    break;
                }
                printw("   Expenses Total: $%.2f", expenses);
            }
    }
    printw("\n[Press any key to continue] ");
    refresh();
    getch();   // Wait
    return;
}

void setsettings(void) {
    // setsettings: displays the current computation technique for the probe sequences
    // and gives the choice to change it
    int c;
    clearScreen();
    move(h-3,0);
    switch ( settings ) {
        case 0:
            printw("The current technique for computing the Probe Sequences is \"Linear Probing\"\n");
            break;
        case 1:
            printw("The current technique for computing the Probe Sequences is \"Double Hashing\"\n");
    }
    printw("===========================================================================\n");
    printw("[Press Enter to change the setting, or Space to return to the main menu] ");
    refresh();
    while ( 1 ) {
        c= getch();
        if ( c == '\r' ) break;
        if ( c == ' ' ) return;
    }

    clearScreen();
    move(h-3,0);

    printw("Choose your preferable technique for computing the Probe Sequences\n");
    printw("==================================================================\n");
    printw("[Note: Changes are not going to affect the current table]\n");
    printw("=========================================================\n");
    printw("[Press 1 or 2 for the choices]\n");
    printw("==============================\n");
    printw("1. Linear Probing\n");
    printw("2. Double Hashing\n");
    printw("Select: ");
    refresh();
    c = getch();
    while ( c != '1' && c != '2' ) c = getch();
    switch ( c ) {
        case '1':
            temp_set = 0;
            break;
        case '2':
            temp_set = 1;
    }
}

int hashinsert(char *id) {
    // hashinsert: finds a position in the ahash and places a pointer to a struct rec that it creates
    // reutrns: the position in the ahash, or -1 on failure
    int i = 0, j;
    clearScreen();
    move(h-1,0);
    while ( i < ahashsize ) {   // While the cells haven't all been checked
        if ( ahash[j = hash(id,i)] == NULL || ahash[j] == &deleted ) {
            // If the cell is not occupied... allocate the some space
            ahash[j] = (struct rec *) malloc(sizeof(struct rec));
            if ( ahash[j] == NULL ) {   // Error checking
                printw("Memory was not allocated. Insertion failed!\n");
                refresh();
                return -1; }
            // Every element in the htsorted table is going to point to the exact structs ahash does
            htsorted[entries] = ahash[j];
            entries++;
            strcpy(ahash[j]->id,id);   // Copy the ID
            ahash[j]->ptr = NULL;   // Initialize the pointer
            return j;   // Return position on the hash table
        }else if ( strcmp(ahash[j]->id,id) == 0 ) { // If the ids match
            return j;   // Return position on the hash table
        } else {
            collisions++;
            i++; }
    }
    printw("Error: The table is full!");
    refresh();
    return -1;
}

int hashsearch(char *id, int *colls) {
    // hashsearch: searches for an ID in the ahash
    // reutrns: the position of the ID, or -1 on failure
    int i = 0, j;

    while ( i < ahashsize ) {   // While the cells haven't all been checked
        if ( ahash[j = hash(id,i)] == NULL )
            return -1;   // The card with this id does not exist
        // If there hasn't been a deletion at the cell, there will be a card there. Compare the two ids
        else if ( ahash[j] != &deleted && strcmp(ahash[j]->id, id) == 0 )
            return j;   // Return position on the hash table
        if ( colls != NULL ) *colls++;
        /* In case you are searching for an ID to delete, note the collisions that will happen,
         * so they can be removed from the total amount of collisions in the hash table later */
        i++;
    }
    return -1;
}

unsigned hash(char *id, int i) {
    // hashing: implements the fragmentation
    // returns: a position in the hash table
    if ( settings )
        return ( djb2hash(id) + i * sdbmhash(id) ) % ahashsize;   // In case of double hashing
    else
        return ( djb2hash(id) + i ) % ahashsize;   // In case of linear probing
}

unsigned djb2hash(char *str) {
    // djb2hash: the first hash function
    // returns: a position in the hash table
    unsigned long long hash = 5381;
    int c;

    while ( c = *str++ )
        hash = ( (hash << 5) + hash ) + c;
    return hash % ahashsize;
}

unsigned sdbmhash(char *str) {
    // sdbmhash: the second hash function
    // returns: a position in the hash table
    unsigned long long hash = 0;
    int c;

    while ( c = *str++ )
        hash = c + (hash << 6) + (hash << 16) - hash;
    return (hash % (ahashsize - 1)) + 1;
}

void nextprime(int p) {
    // nextprime: takes a number and computes the immediate next greater prime number
    int d;

    for ( ; ; p++ ) {
        for ( d = 2; d < p; d++ )
            if ( p % d == 0 ) break;
        if ( d == p ) break;
    }
    ahashsize = p;
}

void heapSort(struct rec **htsorted, int array_size, int (*comp)(struct rec *, struct rec *)) {
    // heapSort: implementation of the heap sorting
    int i;
    struct rec *temp;

    for ( i = array_size / 2; i >= 0; i-- )
        siftDown(htsorted, i, array_size - 1, comp);

    for ( i = array_size - 1; i >= 1; i-- ) {
        temp = htsorted[0];
        htsorted[0] = htsorted[i];
        htsorted[i] = temp;
        siftDown(htsorted, 0, i-1, comp);
    }
}

void siftDown(struct rec **htsorted, int root, int bottom, int (*comp)(struct rec *, struct rec *)) {
    // siftDown: used by heapSort
    int done, maxChild;
    struct rec *temp;

    done = 0;
    while ( root * 2 <= bottom && !done ) {
        if ( root * 2 == bottom )
            maxChild = root * 2;
        else if ( comp(htsorted[root * 2], htsorted[root * 2 + 1]) > 0 )
            maxChild = root * 2;
        else
            maxChild = root * 2 + 1;

        if ( comp(htsorted[root], htsorted[maxChild]) < 0 ) {
            temp = htsorted[root];
            htsorted[root] = htsorted[maxChild];
            htsorted[maxChild] = temp;
            root = maxChild;
        } else
            done = 1;
    }
}

void heapSortinayear(struct rec **htsorted, int array_size, int year) {
    // heapSortinayear: implementation of the heap sorting modified to carry argument 'year'
    int i;
    struct rec *temp;

    for ( i = array_size / 2; i >= 0; i-- )
        siftDowninayear(htsorted, i, array_size - 1, year);

    for ( i = array_size - 1; i >= 1; i-- ) {
        temp = htsorted[0];
        htsorted[0] = htsorted[i];
        htsorted[i] = temp;
        siftDowninayear(htsorted, 0, i-1, year);
    }
}

void siftDowninayear(struct rec **htsorted, int root, int bottom, int year) {
    // siftDowninayear: used by heapSortinayear
    int done, maxChild;
    struct rec *temp;

    done = 0;
    while ( root * 2 <= bottom && !done ) {
        if ( root * 2 == bottom )
            maxChild = root * 2;
        else if ( cmpexpensesinayear(htsorted[root * 2], htsorted[root * 2 + 1], year) > 0 )
            maxChild = root * 2;
        else
            maxChild = root * 2 + 1;

        if ( cmpexpensesinayear(htsorted[root], htsorted[maxChild], year) < 0 ) {
            temp = htsorted[root];
            htsorted[root] = htsorted[maxChild];
            htsorted[maxChild] = temp;
            root = maxChild;
        } else
            done = 1;
    }
}

int cmpproducts(struct rec *rec1, struct rec *rec2) {
    // cmpproducts: compares the amount of products purchased by two cardholders
    // returns: +1, 0, -1 for rec1>rec2, rec1==rec2, rec1<rec2, respectively
    int count1, count2;

    count1 = countproducts(rec1);
    count2 = countproducts(rec2);
    return count1 <= count2 ? ( count1 < count2 ? -1 : 0 ) : 1;
}

int countproducts(struct rec *strct) {
    // countproducts: counts the products rented with the ID strct->id
    struct year *yptr;
    struct day *dptr;
    struct prod *pptr;
    int count = 0;

    yptr = strct->ptr;
    while ( yptr != NULL ) {
        dptr = yptr->ptr;
        while ( dptr != NULL ) {
            pptr = dptr->ptr;
            while ( pptr != NULL ) {
                count++;
                pptr = pptr->next;
            }
            dptr = dptr->next;
        }
        yptr = yptr->next;
    }
    return count;
}

int cmpexpenses(struct rec *rec1, struct rec *rec2) {
    // cmpexpenses: compares the expenses of two cardholders
    // returns: +1, 0, -1 for rec1>rec2, rec1==rec2, rec1<rec2, respectively
    double count1, count2;

    count1 = countexpenses(rec1);
    count2 = countexpenses(rec2);
    return count1 <= count2 ? ( count1 < count2 ? -1 : 0 ) : 1;
}

double countexpenses(struct rec *strct) {
    // countexpenses: counts the expenses made with the card with the ID strct->id
    struct year *yptr;
    struct day *dptr;
    double exp = 0;

    yptr = strct->ptr;
    while ( yptr != NULL ) {
        dptr = yptr->ptr;
        while ( dptr != NULL ) {
            exp += dptr->expenses;
            dptr = dptr->next;
        }
        yptr = yptr->next;
    }
    return exp;
}

int cmpexpensesinayear(struct rec *rec1, struct rec *rec2, int year) {
    // cmpexpensesinayear: modified cmpexpenses to carry the year
    double count1, count2;

    count1 = countexpensesinayear(rec1, year);
    count2 = countexpensesinayear(rec2, year);
    return count1 <= count2 ? ( count1 < count2 ? -1 : 0 ) : 1;
}

double countexpensesinayear(struct rec *strct, int year) {
    // countexpensesinayear: modified countexpenses to count only the expenses in the specified year
    struct year *yptr;
    struct day *dptr;
    double exp = 0;

    yptr = strct->ptr;
    while ( yptr != NULL ) {
        if ( yptr->year != year ) {
            yptr = yptr->next;
            continue; }
        dptr = yptr->ptr;
        while ( dptr != NULL ) {
            exp += dptr->expenses;
            dptr = dptr->next;
        }
        break;
    }
    return exp;
}



void intro(){
    move(0, 0);

    printw("                     :+""  ~<<::""+:\n");
    printw("                +Xi<<<<!<  `<<!?!<<<HMti%L\n");
    printw("            :?HMMMM:<<<!<~ <<<!X<<<!MM88MMh?x\n");
    printw("          !HMRMMRMMM:<<<!< <<<!!<<<MR88MRMMRMH?.\n");
    printw("        ?NMMMMMMMMMMM<<<?<  <<!!<<XM88RMMMMMMMMM?\n");
    printw("      !88888MMMMMMRMMk<<!!  <<H!<<M88MRMMRMMMRMMRM!\n");
    printw("     <M8888888MMMMMMMM:<<!  <<H<<488RMMMMMMMMMMMMMM>:\n");
    printw("   xHMRMMR888888RMMMMMM<<!< <!!<<988RMMMRMMRMMMMM?!<<%\n");
    printw("  :XMMMMMMMM88888MMMMMMH<<~ ~~~<X8RMMMMMMMMMMM!!<~    k\n");
    printw("  <<<!MMRMMRMMR8888MMP.n~       #R.#MMRMMRM?<~~   .nMMh.\n");
    printw(" !MMH:<<<!*MMMMMMM8Pu! n'       '+ 'h!MM!!~   :@MMMMMMM/\n");
    printw(".HMRMMRMMMH:<<'*RM M @             * '   .nMMMMMMMRMMRMMk\n");
    printw("MMMMMMMMMMMMMMMMx < '      .u.        4'MMMMMMMMMMMMMMMM9\n");
    printw("!RMMRMMMRMMRMMMMMX M     @P   #8     4 MMRMMMRMMRMMMMMMR<\n");
    printw("!MMM__     ___     _    '8     8!  ____ _       _     MMM!\n");
    printw("kMMR\\ \\   / (_) __| | ___  ___    / ___| |_   _| |__  MM!\n");
    printw("MMMM \\ \\ / /| |/ _` |/ _ \\/ _ \\  | |   | | | | | '_ \\ MM9\n");
    printw("'9MM  \\ V / | | (_| |  __/ (_) | | |___| | |_| | |_) |MR!\n");
    printw(" >MM   \\_/  |_|\\__,_|\\___|\\___/   \\____|_|\\__,_|_.__/ ~<\n");
    printw("  !RMM#~   :<:MMRMMMMH.*n:      :*.HRMMMRM8888888MRMMM!\n");
    printw("  !     <<:tMMMMMMMMMM8RM<::: :<<XMMMMMMMMMR88888888MM!\n");
    printw("   ~ <<<XHMRMMMMMMRMM8RM<<<<< `!<<MRMMRMMRMMMRR888888#\n");
    printw("     :HMMMMMMMMMMMM988MM<<X!<~'~<<<MMMMMMMMMMMMMR88#!\n");
    printw("      ~MMRMMMRMMRMM88MM<<<?<<  <<<<!RMMMRMMRMMMMMM!\n");
    printw("        xMMMMMMMM988MM%<<<?<<: <!<<<?MMMMMMMMMMMX\n");
    printw("          !?MMMM@88MMR<<<<!<<<  <:<<<MRMMRMMMP!\n");
    printw("            'X*988RMM!<<<?!<<~  <!<<<<MMMMM?'\n");
    printw("                !X*MM<<<<H!<<`  <?<<<<<)!\n");
    printw("                     '+:uX!<<< .::+''\n");

}



//+++++++++++++++++++++++++++HELPERS++++++++++++++++++++++++++

void setSettings() {
    // setsettings: displays the current computation technique for the probe sequences
    // and gives the choice to change it
    int c;

    FILE* set = fopen("settings.txt", "r");
    if (set) {
        char line[1024];
        while (fgets(line, 1024, set)){
            char* tmp = strdup(line);
            settings = atoi(tmp);
            // NOTE strtok clobbers tmp
            free(tmp);
        }
        fclose(set);
    }
    clearScreen();
    move(h-3, 0);
    switch ( settings ) {
        case 0:
            printw("The current technique for computing the Probe Sequences is \"Linear Probing\"\n");
            break;
        case 1:
            printw("The current technique for computing the Probe Sequences is \"Double Hashing\"\n");
            break;
    }
    printw("===========================================================================\n");
    printw("[Press Enter to change the setting, or Space to return to the main menu] ");
    refresh();
    while ( 1 ) {
        c = getch();
        if ( c == '\n' ) break;
        if ( c == ' ' ) return;
    }

    initSettings();
}

void gatherInput(char *filename) {
    // gatherInput: gets input and displays it real time while using ncurses
    int c, i = 0;

    move(h-1,0);
    printw(" Type here: ");
    refresh();
    while ( (c = getch()) != '\n' || !i ) {
        if ( c == '\n' && !i ) {    // Don't accept #zero characters
            move(h-1,0);
            printw(" Type again: ");
            refresh();
        } 
        if ( isspace(c) )   // Ignore any spaces or tabs
            continue;
        /* There is no need for any further error checking. Mistakes in the filenames
         * will be recognized by a later use of fopen as failures to open the files. */
        filename[i++] = c;   // Write down the character
        printw("%c", c);
        refresh();
    }   // The filename has been taken
}

void initSettings(){

    clearScreen();
    move(h-10, 0);
    printw("Choose your preferable technique for computing the Probe Sequences\n");
    printw("[Note: Changes are not going to affect the current table]\n");
    printw("[Press 1 or 2 for the choices]\n\n");
    printw("1. Linear Probing\n");
    printw("2. Double Hashing\n\n");
    printw("Select: ");
    refresh();
    int c = getch();
    while ( c != '1' && c != '2' ) c = getch();
    switch ( c ) {
        case '1':
            temp_set = 0;
            break;
        case '2':
            temp_set = 1;
    }
    FILE* set = fopen("settings.txt", "w");
    fprintf(set,"%d", temp_set);
    fclose(set);

}

int dayOfTheYear() {
    time_t rawtime;
    struct tm *info;
    char buffer[80];

    time( &rawtime );

    info = localtime( &rawtime );
    return info->tm_yday;
}

void clearScreen(){
    int line = h-14;
    for(line;line<=h;line++){
        move(line, 0);
        clrtoeol();
    }
}
