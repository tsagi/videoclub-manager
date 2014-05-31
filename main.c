#include "main.h"

int hashtabsize, settings, temp_set;
int collisions = 0;
int entries = 0;
WINDOW* logo;

int h, w;

int main(int argc, char *argv[]){
    FILE* stream = fopen("data.csv", "r");    
    char line[1024];
    while (fgets(line, 1024, stream))
    {
        char* tmp = strdup(line);
        // NOTE strtok clobbers tmp
        free(tmp);
    }

    enum {NewRental = '1', Return = '2', CancelRental= '3' ,
        Settings = '8', Exit = '9'};

    int select;
    
    initscr();
    noecho();
    getmaxyx(stdscr, h, w);
    intro();

    FILE* set = fopen("settings.txt", "r");
    if (!set){
        fclose(set);
        initSettings();
    }
    menu();
    refresh();

    while ((select = getch()) != Exit) {
        switch ( select ) {
            case NewRental:
                printw("    New Rental");
                refresh();
                break;
            case Return:
                printw("    Return");
                refresh();
                break;
            case CancelRental:
                printw("    Cancel Rental");
                refresh();
                break;
            case Settings:
                setSettings();
                break;
            default:
                printw("    Invalid option...");    /****************************************/
                refresh();                          /* Display that the option is invalid   */ 
                sleep(1);                           /* for a second. This can be interupted */ 
                move(h-1, 8);                       /* by any input.                        */
                clrtoeol();                         /****************************************/
                refresh();
                continue;
        }
        menu();
        refresh();
       
    }
    endwin();
    return 0;
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
    printw("                     '+:uX!<<< .::+''\n\n\n"); 

}


const char* getfield(char* line, int num)
{
    const char* tok;
    for (tok = strtok(line, ",");
            tok && *tok;
            tok = strtok(NULL, ",\n"))
    {
        if (!--num)
            return tok;
    }
    return NULL;
}

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

void initSettings(){

    clearScreen();
    move(h-10, 0);
	printw("Choose your preferable technique for computing the Probe Sequences\n");
	printw("==================================================================\n");
	printw("[Note: Changes are not going to affect the current table]\n");
	printw("=========================================================\n");
	printw("[Press 1 or 2 for the choices]\n");
	printw("==============================\n");
	printw("1. Linear Probing\n");
	printw("2. Double Hashing\n");
	printw("\nSelect: ");
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

void menu() {
    clearScreen();
    move(h-9, 0);
    // menu: displays the main menu of the program
    printw("\n\nMenu:\n");
    printw("  1. Rent a Movie\n");
    printw("  2. Return a Movie\n");
    printw("  3. Cancel Rental\n");
    printw("  8. Settings\n");
    printw("  9. Exit\n");
    printw("Select: ");
}

void clearScreen(){
    int line = h-11;
    for(line;line<=h;line++){
        move(line, 0);
        clrtoeol();
    }
}

