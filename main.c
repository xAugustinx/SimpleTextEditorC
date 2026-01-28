#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <termios.h>

#define byte unsigned char
#define TRUE 1
#define FALSE 0

#define ilosLini 50
#define standardowyPlik "./main.d"

FILE * plikKodu;

char * plikWFormieTablicyChar;
int rozmiarTablicy;
long pointerOfUser = 0;
int klawisz = 0;
byte turnON = TRUE;
char HOMEMode = 0;

byte czyWpisany = 0;
byte czyNowyWpisany = 1;
char * plikWybranyPrzyOtwarciu;
byte czyPrzyOtwarciuZostalWybrany = 0;

void przygotujTerminal() {
    struct termios t;
    tcgetattr(STDIN_FILENO, &t);           // Pobierz aktualne ustawienia
    t.c_lflag &= ~(ICANON | ECHO);         // Wyłącz tryb kanoniczny i echo
    tcsetattr(STDIN_FILENO, TCSANOW, &t);  // Zastosuj zmiany natychmiast
}

void * wieczneSczytywanie(void * arg)
{
    while (turnON)
    {
        if (!HOMEMode)
        {
            czyNowyWpisany = TRUE;

            klawisz = getchar();
            if (klawisz == 27)
            {
                if (getchar() == 91)
                {
                    char c = getchar();
                    if (c > 64 && c < 69)  klawisz = c + 191;
                }
            }

            if (klawisz == 256) {
                for (int i = pointerOfUser-1; i > -1; i--)
                {
                    if (plikWFormieTablicyChar[i+1] == '\n' || i == 0) {pointerOfUser = i; break;}
                }
            }
            else if (klawisz == 257) {
                for (int i = pointerOfUser+1; i < rozmiarTablicy; i++)
                {
                    if (plikWFormieTablicyChar[i+1] == '\n' || i == rozmiarTablicy) {pointerOfUser = i; break;}
                }
            }
            else if (klawisz == 258) pointerOfUser++;
            else if (klawisz == 259) pointerOfUser--;
            else if (klawisz == 27)
            {
                if (HOMEMode) HOMEMode--;
                else HOMEMode++;
            }

            czyWpisany = 1;
        }
        else
        {
            char wpisany[100];
            printf("\n Operacja: %c",'\n');

            scanf("%s",&wpisany);

            if (wpisany[0] == 'S')
            {
                printf("\n Zapisać w otwartym pliku ? T/N%c",'\n');
                scanf("%s",&wpisany);
                if (wpisany[0] == 'N' || wpisany[0] == 'n' )
                {
                    printf("\nNazwa nowego pliku: %c",'\n');
                    scanf("%s",&wpisany);
                    FILE * tymczasowy = fopen(wpisany,"w");
                    for (int i = 1; i < rozmiarTablicy; i++) fprintf(tymczasowy,"%c",plikWFormieTablicyChar[i]);
                    fclose(tymczasowy);
                }
                else if (wpisany[0] == 'T' || wpisany[0] == 't' )
                {
                    int rozmiarNowego = -1;
                    for (int i = 1; TRUE; i++)
                    {
                        if (plikWybranyPrzyOtwarciu[i] == 0) {rozmiarNowego = i; break;}
                    }
                    char nazwa[rozmiarNowego];
                    for (int i = 1; i < rozmiarNowego+1; i++)
                    {
                        nazwa[i-1] = plikWybranyPrzyOtwarciu[i];
                    }
                    FILE * tymczasowy = fopen(nazwa,"w");
                    for (int i = 1; i < rozmiarTablicy; i++) fprintf(tymczasowy,"%c",plikWFormieTablicyChar[i]);
                    fclose(tymczasowy);
                }
            }
            else if (wpisany[0] == 'q') turnON = FALSE;

            HOMEMode = 0;
        }
        
    }
}

void * pisanieWPliku(void * arg)
{
    while (turnON)
    {
        byte czyPowtorzyc = TRUE;  
        if (czyWpisany && !HOMEMode)
        {
                czyNowyWpisany = FALSE;

                if (klawisz == 127)
                {
                    for (int i = pointerOfUser; i < rozmiarTablicy; i++)
                    {
                        plikWFormieTablicyChar[i] = plikWFormieTablicyChar[i + 1];
                    }
                    rozmiarTablicy--;
                    pointerOfUser--;

                    char *tymczasowy = realloc(plikWFormieTablicyChar, (rozmiarTablicy + 1) * sizeof(char));
                    plikWFormieTablicyChar = tymczasowy;
                    
                }
                else if (klawisz > 41 && klawisz < 126 || klawisz == 10 || klawisz == 32)
                {
                    if (klawisz == 10) klawisz = '\n';
                    else if (klawisz == 32) klawisz = ' ';

                    rozmiarTablicy++;
                    char *tymczasowy = realloc(plikWFormieTablicyChar, (rozmiarTablicy + 1) * sizeof(char));
                    plikWFormieTablicyChar = tymczasowy;

                    for (int i = rozmiarTablicy -1; i > pointerOfUser; i--)
                    {
                        plikWFormieTablicyChar[i] = plikWFormieTablicyChar[i-1];
                    }

                    

                    pointerOfUser++;

                    plikWFormieTablicyChar[pointerOfUser] = klawisz;

                }


                system("clear");
                int ostatni = 0;
                int iloscN = 0;
                int odKtoregoZaczacWypisywanie = -1;


                for (int i = pointerOfUser - 1; i >= 0; i--) {

                    if (plikWFormieTablicyChar[i] == '\n') iloscN++;
                    if (iloscN == ilosLini/2) { 
                        odKtoregoZaczacWypisywanie = i + 1; 
                        break;
                    }
                    if (i == 0) odKtoregoZaczacWypisywanie = 0; 
            
                }

                iloscN = 0;
                for (int i = odKtoregoZaczacWypisywanie; i < rozmiarTablicy; i++)
                {
                    if (plikWFormieTablicyChar[i+1] == '\n') iloscN++;
                    if (iloscN > ilosLini - 1  ) break;
                    
                    if (i == pointerOfUser) printf("\033[1;4;37m|\033[0m");

                    printf("%c",plikWFormieTablicyChar[i+1]);
                }

                printf("\n\n\n Aktualny Znak: %d\n Znak Wciśnięty: %d\n HOME Mode: %d\n",pointerOfUser,klawisz,HOMEMode);

            
            if (czyPowtorzyc) { printf("\n");  czyWpisany = FALSE;}
        }
    }
}


int main(int argc, char *qrgv[])
{
    przygotujTerminal();

    if (argc > 1) 
    {
        byte rozmiar;
        for (rozmiar = 0; TRUE; rozmiar++) if (qrgv[1][rozmiar] == 0) break;
        char pierwszyArgument[rozmiar+1];
        pierwszyArgument[rozmiar] = 0;
        for (byte i = 0; i < rozmiar; i++) pierwszyArgument[i] = qrgv[1][i];

        plikKodu = fopen(pierwszyArgument, "r");

        czyPrzyOtwarciuZostalWybrany = TRUE;

        plikWybranyPrzyOtwarciu = malloc(sizeof(pierwszyArgument)+1 * sizeof(char) );

        for (int i = 1; i < sizeof(pierwszyArgument)+1; i++)
        {
            plikWybranyPrzyOtwarciu[i] = pierwszyArgument[i-1];
        }

    }
    else { plikKodu = fopen(standardowyPlik,"r");}
    

    fseek(plikKodu, 0, SEEK_END);
    rozmiarTablicy = ftell(plikKodu)+1;

    plikWFormieTablicyChar = malloc(rozmiarTablicy * sizeof(char) );

    for (int i = 0; i < rozmiarTablicy+1; i++){
        fseek(plikKodu,i,0);
        plikWFormieTablicyChar[i+1] = fgetc(plikKodu);
    }

    pthread_t input, plikT;
    pthread_create(&input,NULL,*wieczneSczytywanie,NULL);
    pthread_create(&plikT,NULL,*pisanieWPliku,NULL);
    pthread_join(input,NULL);
    pthread_join(plikT,NULL);
    free(plikWFormieTablicyChar);
    
}