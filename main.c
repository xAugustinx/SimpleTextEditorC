#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <termios.h>
#include <unistd.h>

#define byte unsigned char
#define TRUE 1
#define FALSE 0
#define iloscLini 50

byte czyPrzyOtwarciuZostalWybrany = FALSE;
byte turnON = TRUE;

byte czyAkuratZostalWpisany = TRUE;
int klawisz = 0;
byte HOMEMode = 0;

//plik
FILE * plikKodu;
int rozmiarPliku;

//blokada
pthread_mutex_t blokada;

//linia itd
int aktualnyZnak = 1;
int aktualnaLinia = 0;

//tablice Dynamiczne
char * plikWTablicy;
char * plikWybranyPrzyOtwarciu;

char zwracanieKoloru(char wejsciowaTablica[], int rozmiarTablicy)
{
    const char slowaNaCiemnyNiebieski[] = "int|char|unsigned|bool|struct|void|file|zmienna|byte|for|while|else|if|{|}|return|";
    byte czyZgodny = 1;
    int licznikZnakowKreski = 0;
    int licznikStrikteTablicy = 0;
    int pierwszy = -1;

    for (int i = 0; i < sizeof(slowaNaCiemnyNiebieski); i++)
    {
        if (wejsciowaTablica[licznikStrikteTablicy] != 9)
        {
            if (pierwszy == -1) pierwszy = licznikStrikteTablicy;
            if (slowaNaCiemnyNiebieski[i] == '|' )
            {
                if (czyZgodny && !wejsciowaTablica[licznikStrikteTablicy]) {
                    if (licznikZnakowKreski > 9) return 'f';
                    else return 'n';
                }
                licznikZnakowKreski++;
                licznikStrikteTablicy = -1;
                czyZgodny = TRUE;
            }
            if (licznikStrikteTablicy != -1) {
                if (licznikStrikteTablicy < rozmiarTablicy) {
                    if (wejsciowaTablica[licznikStrikteTablicy] != slowaNaCiemnyNiebieski[i] && wejsciowaTablica[licznikStrikteTablicy]-32 != slowaNaCiemnyNiebieski[i]) czyZgodny = FALSE;
                }
                else czyZgodny = FALSE;
            }
            licznikStrikteTablicy++;
        }
        else {licznikStrikteTablicy++; i--;}
    }
    if (wejsciowaTablica[pierwszy] == '#' ) return 'f';
    else if (wejsciowaTablica[rozmiarTablicy -2] == ')' || wejsciowaTablica[rozmiarTablicy -2] == ';' ) return 'z';
    else if (wejsciowaTablica[pierwszy] == '/' ) if (rozmiarTablicy > pierwszy) if (wejsciowaTablica[pierwszy+1] == '/') return '/';
    
    return 0;
}

int ileCyfr(int liczba)
{
    int meow = liczba;
    for (int i = 1; 1; i++ )
    {
        meow = meow/10;
        if (meow == 0) return i;
    }
}

void przygotujTerminal() {
    struct termios t;
    tcgetattr(STDIN_FILENO, &t);       
    t.c_lflag &= ~(ICANON | ECHO);       
    tcsetattr(STDIN_FILENO, TCSANOW, &t);  
}


void *wieczneSczytywanie(void * arg)
{
    while (TRUE)
    {
        pthread_mutex_lock(&blokada);
        if (!turnON) break;
        pthread_mutex_unlock(&blokada);

        if (!HOMEMode)
        {
            klawisz = getchar();
            if (klawisz == 27) if (getchar() == 91)
            {
                char c = getchar();
                if (c > 64 && c < 69)  klawisz = c + 191;
                else HOMEMode = 1;
            }

            if (klawisz == 258 && aktualnyZnak < rozmiarPliku ) {
                aktualnyZnak++;
                if (plikWTablicy[aktualnyZnak] == '\n') aktualnaLinia++;
            }
            else if (klawisz == 256 && aktualnaLinia > 0) {
                for (int i = aktualnyZnak-1; i > 0; i--) if (plikWTablicy[i] == '\n' || i == 1) 
                {aktualnyZnak = i; break;}
                aktualnaLinia--;
            }
            else if (klawisz == 257 && aktualnyZnak < rozmiarPliku) {
                for (int i = aktualnyZnak+1; i < rozmiarPliku; i++) if (plikWTablicy[i] == '\n' || i == rozmiarPliku) 
                {aktualnyZnak = i; break;}
                aktualnaLinia++;
            }
            else if (klawisz == 259 &&  aktualnyZnak > 1) {
                aktualnyZnak--;
                if (plikWTablicy[aktualnyZnak] == '\n') aktualnaLinia--;
            }

            czyAkuratZostalWpisany = TRUE;
        }
        else
        {
            char bufor[100];
            system("clear");
            printf("\033[1;33m%s", "s/o - zapisz\nq - wyjdź \n");
            klawisz = getchar();
            if (klawisz == 's' || klawisz == 'S') {}
            else if (klawisz == 'q' || klawisz == 'Q') {turnON = FALSE; break;}
            

        }
        usleep(10000);
    }
}

void * pisanieWPliku(void * arg)
{
    while (TRUE)
    {
        pthread_mutex_lock(&blokada);
        if (!turnON) break;
        pthread_mutex_unlock(&blokada);

        if (czyAkuratZostalWpisany)
        {
            system("clear");

            int iloscPrzerwN = 0;
            int odKtoregoZaczacWypisywanie = -1;

            if (klawisz > 31 && klawisz < 127 || klawisz == 10 || klawisz == 9)
            {
                rozmiarPliku++;
                char * tymczasowy = realloc(plikWTablicy, rozmiarPliku);
                plikWTablicy = tymczasowy;
                for (int i = rozmiarPliku-1; i > aktualnyZnak; i--) plikWTablicy[i] = plikWTablicy[i-1];
                plikWTablicy[aktualnyZnak] = klawisz;
                aktualnyZnak++;

                if (klawisz == 10) aktualnaLinia++;
            }
            else if (klawisz == 127 && aktualnyZnak > 1)
            {
                if (plikWTablicy[aktualnyZnak-1] == '\n') aktualnaLinia--;
                for (int i = aktualnyZnak-1; i < rozmiarPliku; i++) plikWTablicy[i] = plikWTablicy[i+1];
                rozmiarPliku--;
                char * tymczasowy = realloc(plikWTablicy,rozmiarPliku);
                plikWTablicy = tymczasowy;
                aktualnyZnak--;
            }
            
            for (int i = aktualnyZnak-1; i > 0; i--)
            {
                if (plikWTablicy[i] == '\n') iloscPrzerwN++;
                if (iloscPrzerwN == iloscLini/2  || i == 1 )
                {
                    odKtoregoZaczacWypisywanie = i;
                    break;
                }
            }

            int aktualnaLiniaWWypisywaniu = aktualnaLinia - iloscPrzerwN;
            //tymczasoweWybieranieKolorow
            int pierwszyZdania = -1;
            int ostatniZdania = -1;

            int iloscKolorow = 1;
            char * kolory = malloc(iloscKolorow);

            iloscPrzerwN = 0;

            for (int i = odKtoregoZaczacWypisywanie; iloscPrzerwN < iloscLini && i < rozmiarPliku ; i++ )
            {
                if (plikWTablicy[i] == '\n') iloscPrzerwN++;
                if (i == odKtoregoZaczacWypisywanie) pierwszyZdania = i;

                if ((plikWTablicy[i] != ' ' && plikWTablicy[i] != '\n') && (plikWTablicy[i-1] == ' ' || plikWTablicy[i-1] == '\n')) pierwszyZdania = i;
                else if ((plikWTablicy[i-1] != ' ' && plikWTablicy[i-1] != '\n') && (plikWTablicy[i] == ' ' || plikWTablicy[i] == '\n')) ostatniZdania = i;

                if (pierwszyZdania != -1 && ostatniZdania != -1)
                {
                    char slowoBoze[ostatniZdania - pierwszyZdania+1]; slowoBoze[ostatniZdania - pierwszyZdania] = 0;
                    for (int i = pierwszyZdania; i < ostatniZdania; i++) slowoBoze[i - pierwszyZdania] = plikWTablicy[i];
                    iloscKolorow++;
                    char * tymczasowy = realloc(kolory, iloscKolorow);
                    kolory = tymczasowy;
                    kolory[iloscKolorow -1] = zwracanieKoloru(slowoBoze,sizeof(slowoBoze));
                    pierwszyZdania = -1; ostatniZdania = -1;
                }
                
            }
                   
            //koniecTymczasowegoWybieraniaKolorow

            iloscPrzerwN = 0;
            int ktoreSlowo = 1;
            char czyBylKomentarz = FALSE;

            for (int i = odKtoregoZaczacWypisywanie; iloscPrzerwN < iloscLini && i < rozmiarPliku; i++ )
            {
                printf("\033[0m");
                if ((plikWTablicy[i] != ' ' && plikWTablicy[i] != '\n') && (plikWTablicy[i-1] == ' ' || plikWTablicy[i-1] == '\n')) ktoreSlowo++;   
                if (i == aktualnyZnak) printf("\033[1;36m|\033[0m");


                char kolor = kolory[ktoreSlowo];

                if (kolor == '/' || czyBylKomentarz)      {printf("\033[0;37m%c", plikWTablicy[i+1]); czyBylKomentarz = TRUE;}
                else if (kolor == 'n') printf("\033[0;34m%c", plikWTablicy[i]);
                else if (kolor == 'f') printf("\033[1;35m%c", plikWTablicy[i]);
                else if (kolor == 'z') printf("\033[1;33m%c", plikWTablicy[i]);
                else printf("%c",plikWTablicy[i]);
                

                printf("\033[0m");
                if (plikWTablicy[i] == '\n' || i == odKtoregoZaczacWypisywanie) 
                {
                    czyBylKomentarz = FALSE;
                    iloscPrzerwN++;
                    for (int i = 4 - ileCyfr(iloscPrzerwN + aktualnaLiniaWWypisywaniu); i >= 0; i--  ) printf(" ");
                    printf("\033[32m%d |",iloscPrzerwN + aktualnaLiniaWWypisywaniu);
                
                }
            }

            printf("\n\n\033[1;33m Aktualny Znak: %d\033[0m\n""\033[1;35m Znak Wcisniety: %d\033[0m\n""\033[1;34m HOME Mode: %d\033[0m\n",aktualnyZnak, klawisz, HOMEMode);
            czyAkuratZostalWpisany = FALSE;
            free(kolory);
        }

        usleep(10000);
    }
}


int main(int argc, char *qrgv[])
{
    przygotujTerminal();
    if (argc > 1) {
        byte rozmiar;
        for (rozmiar = 0; TRUE; rozmiar++) if (qrgv[1][rozmiar] == 0) break;
        char pierwszyArgument[rozmiar+1];
        pierwszyArgument[rozmiar] = 0;
        for (byte i = 0; i < rozmiar; i++) pierwszyArgument[i] = qrgv[1][i];
        plikKodu = fopen(pierwszyArgument, "r");
        czyPrzyOtwarciuZostalWybrany = TRUE;
        plikWybranyPrzyOtwarciu = malloc(sizeof(pierwszyArgument)+1 * sizeof(char) );
        for (int i = 1; i < sizeof(pierwszyArgument)+1; i++) plikWybranyPrzyOtwarciu[i] = pierwszyArgument[i-1];
    }

    if (czyPrzyOtwarciuZostalWybrany) {
        fseek(plikKodu, 0, SEEK_END);
        rozmiarPliku = ftell(plikKodu)+1;
    }
    else rozmiarPliku = 1;

    plikWTablicy = malloc(rozmiarPliku);

    if (czyPrzyOtwarciuZostalWybrany) for (int i = 1; i < rozmiarPliku; i++)
    {
        fseek(plikKodu, i-1, 0);
        plikWTablicy[i] = fgetc(plikKodu);
    }

    printf("%d\n",rozmiarPliku);
    
    pthread_t input, plikT;
    pthread_create(&input,NULL,*wieczneSczytywanie,NULL);
    pthread_create(&plikT,NULL,*pisanieWPliku,NULL);
    pthread_join(input,NULL);
    pthread_join(plikT,NULL); 

    free(plikWybranyPrzyOtwarciu);
    free(plikWTablicy);
    fclose(plikKodu);
}