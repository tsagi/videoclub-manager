#include "main.h"

int hashtabsize, settings, temp_set;
int collisions = 0;
int entries = 0;

int main(int argc, char *argv[]){
    FILE* stream = fopen("data.csv", "r");
    
    char line[1024];
    while (fgets(line, 1024, stream))
    {
        char* tmp = strdup(line);
        // NOTE strtok clobbers tmp
        free(tmp);
    }

    enum {NewRental = 1, Return = 2, CancelRental= 3 ,
        Settings = 8, Exit=9};

    int select; 
    
    do{
        clearScreen(); 
        intro();
        menu();
        scanf("%d", &select);
        switch ( select ) {
            case NewRental:
                printf("New Rental");
                break;
            case Return:
                printf("Return");
                break;
            case CancelRental:
                printf("Cancel Rental");
                break;
            case Settings:
                printf("Settings");
                break;
            default:
                printf("Invalid option or exiting");
                break;
        }       
    }while (select != Exit);

    return 0;
}

void intro(){
    printf("\n\n");
    printf("                     :+""  ~<<::""+:\n");
    printf("                +Xi<<<<!<  `<<!?!<<<HMti%L\n");
    printf("            :?HMMMM:<<<!<~ <<<!X<<<!MM88MMh?x\n");
    printf("          !HMRMMRMMM:<<<!< <<<!!<<<MR88MRMMRMH?.\n");
    printf("        ?NMMMMMMMMMMM<<<?<  <<!!<<XM88RMMMMMMMMM?\n");
    printf("      !88888MMMMMMRMMk<<!!  <<H!<<M88MRMMRMMMRMMRM!\n");
    printf("     <M8888888MMMMMMMM:<<!  <<H<<488RMMMMMMMMMMMMMM>:\n");
    printf("   xHMRMMR888888RMMMMMM<<!< <!!<<988RMMMRMMRMMMMM?!<<%\n");
    printf("  :XMMMMMMMM88888MMMMMMH<<~ ~~~<X8RMMMMMMMMMMM!!<~    k\n");
    printf("  <<<!MMRMMRMMR8888MMP.n~       #R.#MMRMMRM?<~~   .nMMh.\n");
    printf(" !MMH:<<<!*MMMMMMM8Pu! n'       '+ 'h!MM!!~   :@MMMMMMM/\n");
    printf(".HMRMMRMMMH:<<'*RM M @             * '   .nMMMMMMMRMMRMMk\n");
    printf("MMMMMMMMMMMMMMMMx < '      .u.        4'MMMMMMMMMMMMMMMM9\n");
    printf("!RMMRMMMRMMRMMMMMX M     @P   #8     4 MMRMMMRMMRMMMMMMR<\n");
    printf("!MMM__     ___     _    '8     8!  ____ _       _     MMM!\n");
    printf("kMMR\\ \\   / (_) __| | ___  ___    / ___| |_   _| |__  MM!\n");
    printf("MMMM \\ \\ / /| |/ _` |/ _ \\/ _ \\  | |   | | | | | '_ \\ MM9\n");
    printf("'9MM  \\ V / | | (_| |  __/ (_) | | |___| | |_| | |_) |MR!\n");
    printf(" >MM   \\_/  |_|\\__,_|\\___|\\___/   \\____|_|\\__,_|_.__/ ~<\n");    
    printf("  !RMM#~   :<:MMRMMMMH.*n:      :*.HRMMMRM8888888MRMMM!\n");
    printf("  !     <<:tMMMMMMMMMM8RM<::: :<<XMMMMMMMMMR88888888MM!\n");
    printf("   ~ <<<XHMRMMMMMMRMM8RM<<<<< `!<<MRMMRMMRMMMRR888888#\n");
    printf("     :HMMMMMMMMMMMM988MM<<X!<~'~<<<MMMMMMMMMMMMMR88#!\n");
    printf("      ~MMRMMMRMMRMM88MM<<<?<<  <<<<!RMMMRMMRMMMMMM!\n");
    printf("        xMMMMMMMM988MM%<<<?<<: <!<<<?MMMMMMMMMMMX\n");
    printf("          !?MMMM@88MMR<<<<!<<<  <:<<<MRMMRMMMP!\n");
    printf("            'X*988RMM!<<<?!<<~  <!<<<<MMMMM?'\n");
    printf("                !X*MM<<<<H!<<`  <?<<<<<)!\n");
    printf("                     '+:uX!<<< .::+''\n\n\n"); 

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

void menu() {
    // menu: displays the main menu of the program
    printf("\n\nMenu:\n");
    printf("  1. Rent a Movie\n");
    printf("  2. Return a Movie\n");
    printf("  3. Cancel Rental\n");
    printf("  8. Settings\n");
    printf("  9. Exit\n");
    printf("Select: ");
}

void clearScreen(){
    if(OS_WIN)
        system("cls");    /*   DOS   */
    else
        system("clear");  /*   UNIX   */
}

