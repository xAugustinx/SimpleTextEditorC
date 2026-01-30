#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <termios.h>
#include <unistd.h>

#define byte unsigned char
#define TRUE 1
#define FALSE 0
#define ilosLini 50
#define standardowyPlik "./gdfgkdfjgdfgjdfkgdfkgjkdfgdfjgkdkgdp"

FILE * plikKodu;

char * plikWFormieTablicyChar;
int rozmiarTablicy;
long pointerOfUser = 0;
int klawisz = 0;
byte turnON = TRUE;
char HOMEMode = 0;
byte czyUruchomionyPrzyStarcie = 0;
byte czyWpisany = 1;
byte czyNowyWpisany = 1;
char * plikWybranyPrzyOtwarciu;
byte czyPrzyOtwarciuZostalWybrany = 0;
int aktualnaLinia = 0;

void przygotujTerminal() {
    struct termios t;
    tcgetattr(STDIN_FILENO, &t);       
    t.c_lflag &= ~(ICANON | ECHO);       
    tcsetattr(STDIN_FILENO, TCSANOW, &t);  
}

int ileZnakow(int meow)
{
    int sprawdzany = meow;
    for (int i = 0; TRUE; i++){
        if (!sprawdzany) return i;
        sprawdzany/=10;
    }
}

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
                if (getchar() == 91){
                    char c = getchar();
                    if (c > 64 && c < 69)  klawisz = c + 191;
                }
            }
            if (klawisz == 256) {
                for (int i = pointerOfUser-1; i > 0; i--) if (plikWFormieTablicyChar[i+1] == '\n' || i == 0) {pointerOfUser = i; break;}
                if (aktualnaLinia > 0) aktualnaLinia--;
            }
            else if (klawisz == 257) {
                for (int i = pointerOfUser+1; i < rozmiarTablicy; i++) if (plikWFormieTablicyChar[i+1] == '\n' || i == rozmiarTablicy) {pointerOfUser = i; break;}
                aktualnaLinia++;
            }
            else if (klawisz == 258 ) pointerOfUser++;
            else if (klawisz == 259 &&  pointerOfUser > 0) pointerOfUser--;
            else if (klawisz == 27)
            {
                if (HOMEMode) HOMEMode--;
                else HOMEMode++;
            }

            czyWpisany = 1;
        }
        else
        {
            char wpisany[50];
            printf("\n Operacja: %c",'\n');

            scanf("%s",&wpisany);

            if (wpisany[0] == 'S')
            {
                if (!czyUruchomionyPrzyStarcie) wpisany[0] = 1;
                else wpisany[0] = 0;
                
                if (wpisany[0])
                {
                    printf("\nNazwa nowego pliku: %c",'\n');
                    scanf("%s",&wpisany);
                    FILE * tymczasowy = fopen(wpisany,"w");
                    for (int i = 1; i < rozmiarTablicy; i++) fprintf(tymczasowy,"%c",plikWFormieTablicyChar[i]);
                    fclose(tymczasowy);
                }
                else
                {
                    int rozmiarNowego = -1;
                    for (int i = 1; TRUE; i++) if (plikWybranyPrzyOtwarciu[i] == 0) {rozmiarNowego = i; break;}
                    char nazwa[rozmiarNowego];
                    for (int i = 1; i < rozmiarNowego+1; i++) nazwa[i-1] = plikWybranyPrzyOtwarciu[i];
                    FILE * tymczasowy = fopen(nazwa,"w");
                    for (int i = 1; i < rozmiarTablicy; i++) fprintf(tymczasowy,"%c",plikWFormieTablicyChar[i]);
                    fclose(tymczasowy);
                }
            }
            else if (wpisany[0] == 'q') turnON = FALSE;

            HOMEMode = 0;
        }
        usleep(10000);
    }
}

void * pisanieWPliku(void * arg)
{
    char * kolorySlow;
    while (turnON)
    {
        byte czyPowtorzyc = TRUE;  
        if (czyWpisany && !HOMEMode)
        {
                czyNowyWpisany = FALSE;

                if (klawisz == 127 && pointerOfUser > 0) {
                    for (int i = pointerOfUser; i < rozmiarTablicy; i++) plikWFormieTablicyChar[i] = plikWFormieTablicyChar[i + 1];
                    
                    rozmiarTablicy--;
                    pointerOfUser--;
                    char *tymczasowy = realloc(plikWFormieTablicyChar, (rozmiarTablicy + 1) * sizeof(char));
                    plikWFormieTablicyChar = tymczasowy;
                    
                }
                else if (klawisz > 31 && klawisz < 126 || klawisz == 10 || klawisz == 32 || klawisz == 9) {
                    if (klawisz == 10) klawisz = '\n';
                    else if (klawisz == 32) klawisz = ' ';

                    rozmiarTablicy++;
                    char *tymczasowy = realloc(plikWFormieTablicyChar, (rozmiarTablicy + 1) * sizeof(char));
                    plikWFormieTablicyChar = tymczasowy;

                    for (int i = rozmiarTablicy -1; i > pointerOfUser; i--) plikWFormieTablicyChar[i] = plikWFormieTablicyChar[i-1];
            
                    pointerOfUser++;
                    plikWFormieTablicyChar[pointerOfUser] = klawisz;

                }
                system("clear");
                int ostatni = 0;
                int iloscN = 0;
                int odKtoregoZaczacWypisywanie = -1;
                int aktualnaLiniaDoWypisania = aktualnaLinia;

                int iloscSlow = 1;
                kolorySlow = malloc(iloscSlow * sizeof(char));
                

                for (int i = pointerOfUser - 1; i >= 0; i--) {
                    if (plikWFormieTablicyChar[i] == '\n') 
                    {
                        iloscN++;
                        aktualnaLiniaDoWypisania--;
                    }
                    if (iloscN == ilosLini/2) { 
                        odKtoregoZaczacWypisywanie = i + 1; 
                        break;
                    }
                    if (i == 0) {odKtoregoZaczacWypisywanie = 0; aktualnaLiniaDoWypisania = 0;} 
                }
                //tymczasoweWybieranieKolorow
                iloscN = 0;
                int pierwszyZdania = -1;
                int ostatniZdania = -1;
                


                for (int i = odKtoregoZaczacWypisywanie+1; i < rozmiarTablicy; i++) {
                    if (i > 1)
                    {
                        if ((plikWFormieTablicyChar[i] != ' ' && plikWFormieTablicyChar[i] != '\n') && (plikWFormieTablicyChar[i-1] == ' ' || plikWFormieTablicyChar[i-1] == '\n')) pierwszyZdania = i;
                        else if ((plikWFormieTablicyChar[i-1] != ' ' && plikWFormieTablicyChar[i-1] != '\n') && (plikWFormieTablicyChar[i] == ' ' || plikWFormieTablicyChar[i] == '\n')) ostatniZdania = i;
                        
                        if (pierwszyZdania != -1 && ostatniZdania != -1) {
                            char slowoBoze[ostatniZdania - pierwszyZdania + 1];  slowoBoze[ostatniZdania - pierwszyZdania] = 0;
                            for (int y = pierwszyZdania; y < ostatniZdania; y++) slowoBoze[y - pierwszyZdania] = plikWFormieTablicyChar[y];

                            iloscSlow++;
                            char * tymczasowy = realloc(kolorySlow, iloscSlow * sizeof(char) );
                            kolorySlow = tymczasowy;
                            kolorySlow[iloscSlow-1] = zwracanieKoloru(slowoBoze,sizeof(slowoBoze));
                            pierwszyZdania = -1; ostatniZdania = -1;
                        }
                    }
                    else if (plikWFormieTablicyChar[1] != '\n' && plikWFormieTablicyChar[1] != ' ' ) pierwszyZdania =  1;
                }
                //koniecTymczasowegoWybieraniaKolorow
                int slowoLiczba = 0;
                iloscN = 0;
                char kolor = 0;
                byte czyBylKomentarz = FALSE;

                for (int i = odKtoregoZaczacWypisywanie; i < rozmiarTablicy; i++){
                    if (plikWFormieTablicyChar[i+1] == '\n') iloscN++;
                    if (iloscN > ilosLini - 1  ) break;
                    if (i == pointerOfUser) printf("\033[1;36m|\033[0m");
                    if (plikWFormieTablicyChar[i+1] == '\n')  czyBylKomentarz = FALSE;
                    
                    if (((plikWFormieTablicyChar[i+1] != ' ' && plikWFormieTablicyChar[i+1] != '\n') && (plikWFormieTablicyChar[i] == ' ' || plikWFormieTablicyChar[i] == '\n')) || i == odKtoregoZaczacWypisywanie) 
                    {
                        slowoLiczba++;
                        kolor = kolorySlow[slowoLiczba];
                    }

                    if (kolor == '/' || czyBylKomentarz)      {printf("\033[0;37m%c", plikWFormieTablicyChar[i+1]); czyBylKomentarz = TRUE;}
                    else if (kolor == 'n') printf("\033[0;34m%c", plikWFormieTablicyChar[i+1]);
                    else if (kolor == 'f') printf("\033[1;35m%c", plikWFormieTablicyChar[i+1]);
                    else if (kolor == 'z') printf("\033[1;33m%c", plikWFormieTablicyChar[i+1]);
                    else printf("%c",plikWFormieTablicyChar[i+1]);
                    
                    printf("\033[0m");
                    if (plikWFormieTablicyChar[i+1] == '\n' || i == odKtoregoZaczacWypisywanie)
                    {
                        int f = 4 - ileZnakow(aktualnaLiniaDoWypisania);
                        for (int bb = 0; bb < f; bb++) printf(" ");
                        printf("%d |",aktualnaLiniaDoWypisania);
                        aktualnaLiniaDoWypisania++;
                    }
                }
                printf("\n\n \033[1;33m Aktualny Znak: %d\033[0m\n""\033[1;35m Znak Wcisniety: %d\033[0m\n""\033[1;34m HOME Mode: %d\033[0m\n",pointerOfUser, klawisz, HOMEMode);

            if (czyPowtorzyc) { printf("\n");  czyWpisany = FALSE;}
        }
        usleep(10000);
    }
    free(kolorySlow);
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
        czyUruchomionyPrzyStarcie = 1;
    }

    if (czyUruchomionyPrzyStarcie) {
        fseek(plikKodu, 0, SEEK_END);
        rozmiarTablicy = ftell(plikKodu)+1;
    }
    else rozmiarTablicy = 1;

    plikWFormieTablicyChar = malloc((rozmiarTablicy+1) * sizeof(char) );
    for (int i = 0; i < rozmiarTablicy+1 && czyUruchomionyPrzyStarcie; i++){
        fseek(plikKodu,i,0);
        plikWFormieTablicyChar[i+1] = fgetc(plikKodu);
    }
    plikWFormieTablicyChar[rozmiarTablicy] = 0;

    pthread_t input, plikT;
    pthread_create(&input,NULL,*wieczneSczytywanie,NULL);
    pthread_create(&plikT,NULL,*pisanieWPliku,NULL);
    pthread_join(input,NULL);
    pthread_join(plikT,NULL);
    free(plikWFormieTablicyChar);
    free(plikWybranyPrzyOtwarciu);
    system("clear");
}