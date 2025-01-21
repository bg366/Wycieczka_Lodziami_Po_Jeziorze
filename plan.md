# Przebieg programu
---

1. Inicjalizacja
   - Interfejs pytający użytkownika o dane (wraz z weryfikacją poprawnego typu danych)
   - Czyszczenie plików
   - Stworzenie potrzebnych procesów
      - Sternicy
      - Kasjer
      - Policjant
      - Generator pasażerów
2. Zamykanie programu

## Sternik

1. Sprawdza czy istnieje log łodzi i tworzy go jeśli nie ma
2. Odpala potrzebne wątki

Wątki:
- Czekający na sygnały od policjanta
- Obsługujący załadunek
   - Wpuszcza i wypuszcza ludzi na/ze statku
- Odpowiadający za rejs
- Czekający na godzinę zamknięcia

## Kasjer

1. Sprawdza czy istnieje log biletów i tworzy go jeśli nie
2. Odpala potrzebne wątki

Wątki:
 - Obsługujący kolejkę (Komunikuje się z pasażerami FIFO, dodaje ich do logu biletów, a następnie wysyła do kolejki)
 - Czekający na sygnał od sternika o zamknięciu łodzi

## Policjant

1. Tworzy 2 wątki - każdy generuje losowo czy zatrzyma łódź i o której, a następnie czeka aż do wygenerowanego czasu by wysłać sygnał sternikowi

## Generator Pasażerów

1. Co jakiś interwał tworzy proces losowego pasażera

## Pasażerowie

1. Generuje swoje dane (wiek, czy ma dziecko, jaką łódkę wybrał i czy powtarza wycieczkę) - domyślnie nie powtarza wycieczki, dziecko to osobny wątek
2. Odpala potrzebne wątki
3. Komunikuje się z kasjerem poprzez kolejkę komunikatów
4. Po otrzymaniu pozwolenia od sternika wchodzi na pokład statku (aktualizuje łódź)
5. Czeka na sygnał od sternika by zejść ze statku

Wątki:
 - Komunikujący się z kasjerem (czekający na sygnał)
 - Przenoszący do kolejki, a następnie czekający na sygnał od sternika
 - Przenoszący do łodzi, a następnie czekający na sygnał od sternika
 - Czekający na zamknięcie portu (tylko jeśli nie wszedł jeszcze na pomost)

# Pamięć Współdzielona

1. Łodzie (listy fifo do których będę przypisywać pasażerów)
   - Nr1 tablica FIFO
   - Nr2 tablica FIFO
2. Kolejki (listy fifo do których będę przypisywać pasażerów)
   - Do łodzi nr1 tablica FIFO
   - Do łodzi nr2 tablica FIFO
3. Pointery do semaforów (dla pomostów)
   - Pomost nr1
   - Pomost nr2

# Setup

## Obsluga

- Zapytaj uzytkownika o dane
- Stworz pamiec i alokuj pamiec
- Stworz procesy (kazdy ma miec init)

## Pamięć współdzielona

- Stworz kolejke (w pliku najlepiej)
- Init
- Dolacz/odlacz proces
- Usun kolejke

## FIFO

- Stworz liste (alokowana dynamicznie)
- Dodaj element
- Usun element
- Sprawdz dlugosc
- Usun liste

## Kolejka komunikatow

- Stworz kolejke
- Dodaj/usun wiadomosc
- Czytaj wiadomosc
- Sprawdz czy jest nowa wiadomosc
- Usun kolejke

## Procesy

- Stworz proces
- Podlacz sie do pamieci
- Usun proces (odlacz go od pamieci)

# Pliki

1. Init
2. Sternik
3. Kasjer
4. Policjant
5. Pasażerowie
6. Utils
    1. Pamięć współdzielona
    2. Generator Pasażerów
    3. Pliki
    4. Błędy
    5. Zamykanie procesów


# Funkcje pomocnicze

1. Handling plików
   - Czyszczenie plików jeśli istnieją
   - Sprawdzenie czy plik istnieje, jeśli nie to go tworzy i otwiera
2. Handling pamięci współdzielonej
   - Czyszczenie pamięci jeśli istnieje
   - Sprawdznie czy pamięć istnieje, jeśli nie to ją tworzy
3. Obsługa błędów
4. Zamykanie procesów / wątków
