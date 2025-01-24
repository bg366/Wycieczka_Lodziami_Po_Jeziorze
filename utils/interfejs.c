#include "interfejs.h"

/* Funkcja pomocnicza do wczytywania liczby całkowitej z stdin.
 * Zwraca 1 w przypadku powodzenia lub 0 w przypadku błędu. */
int wczytaj_int(int *liczba)
{
    int wynik = scanf("%d", liczba);
    if (wynik != 1)
    {
        // Wyczyść bufor ze zbędnych znaków
        while(getchar() != '\n');
        return 0;
    }
    return 1;
}

int setup_pamieci(int *N1, int *N2, int *K)
{
    // Wczytaj N1
    while (1)
    {
        printf("Podaj pojemnosc lodzi nr 1 (N1) [liczba calkowita]: ");
        if (!wczytaj_int(N1))
        {
            printf("Blad! Prosze podac poprawna liczbe calkowita.\n");
            continue;
        }
        if (*N1 <= 0)
        {
            printf("Pojemnosc lodzi (N1) musi byc liczba dodatnia!\n");
            continue;
        }
        break;
    }

    // Wczytaj N2
    while (1)
    {
        printf("Podaj pojemnosc lodzi nr 2 (N2) [liczba calkowita]: ");
        if (!wczytaj_int(N2))
        {
            printf("Blad! Prosze podac poprawna liczbe calkowita.\n");
            continue;
        }
        if (*N2 <= 0)
        {
            printf("Pojemnosc lodzi (N2) musi byc liczba dodatnia!\n");
            continue;
        }
        break;
    }

    // Wczytaj K (pojemnosc pomostow), sprawdzajac K < N1 i K < N2
    while (1) {
        printf("Podaj pojemnosc pomostu (K) [musi byc < N1 i < N2]: ");
        if (!wczytaj_int(K))
        {
            printf("Blad! Prosze podac poprawna liczbe calkowita.\n");
            continue;
        }
        if (*K <= 0)
        {
            printf("Pojemnosc pomostu (K) musi byc liczba dodatnia!\n");
            continue;
        }
        if (*K >= *N1 || *K >= *N2)
        {
            printf("K musi byc mniejsze niz N1 (%d) i N2 (%d)!\n", *N1, *N2);
            continue;
        }
        break;
    }

    // Wypisz podsumowanie
    printf("\n--- PODSUMOWANIE DANYCH PAMIECI ---\n");
    printf("Pojemnosc lodzi nr 1 (N1)   : %d\n", *N1);
    printf("Pojemnosc lodzi nr 2 (N2)   : %d\n", *N2);
    printf("Pojemnosc pomostu (K)       : %d\n\n", *K);

    return 0;
}

int setup_sternikow(int *Tp, int *Tk, int *T1, int *T2)
{
    // Wczytaj Tp
    while (1)
    {
        printf("Podaj czas poczatku rejsow (Tp) [liczba calkowita]: ");
        if (!wczytaj_int(Tp))
        {
            printf("Blad! Prosze podac poprawna liczbe calkowita.\n");
            continue;
        }
        // Jeżeli potrzebujesz sprawdzania, czy Tp >= 0 lub inny warunek, umieść go tutaj
        break;
    }

    // Wczytaj Tk
    while (1) {
        printf("Podaj czas zakonczenia rejsow (Tk) [liczba calkowita]: ");
        if (!wczytaj_int(Tk))
        {
            printf("Blad! Prosze podac poprawna liczbe calkowita.\n");
            continue;
        }
        if (*Tk <= *Tp)
        {
            printf("Czas zakonczenia (Tk) musi byc wiekszy niz czas rozpoczecia (Tp)!\n");
            continue;
        }
        break;
    }

    // Wczytaj T1 (czas trwania rejsu lodzi nr 1)
    while (1)
    {
        printf("Podaj czas trwania rejsu lodzi nr 1 (T1) [w sekundach]: ");
        if (!wczytaj_int(T1))
        {
            printf("Blad! Prosze podac poprawna liczbe calkowita.\n");
            continue;
        }
        if (*T1 <= 0)
        {
            printf("Czas trwania rejsu (T1) musi byc liczba dodatnia!\n");
            continue;
        }
        break;
    }

    // Wczytaj T2 (czas trwania rejsu lodzi nr 2)
    while (1)
    {
        printf("Podaj czas trwania rejsu lodzi nr 2 (T2) [w sekundach]: ");
        if (!wczytaj_int(T2))
        {
            printf("Blad! Prosze podac poprawna liczbe calkowita.\n");
            continue;
        }
        if (*T2 <= 0)
        {
            printf("Czas trwania rejsu (T2) musi byc liczba dodatnia!\n");
            continue;
        }
        break;
    }

    // Wypisz podsumowanie
    printf("\n--- PODSUMOWANIE DANYCH STERNIKOW ---\n");
    printf("Czas rozpoczecia rejsow (Tp): %d\n", *Tp);
    printf("Czas zakonczenia rejsow (Tk): %d\n", *Tk);
    printf("Czas trwania rejsu lodzi 1 (T1): %d sekund\n", *T1);
    printf("Czas trwania rejsu lodzi 2 (T2): %d sekund\n\n", *T2);

    return 0;
}
