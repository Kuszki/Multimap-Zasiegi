# Multimap-Zasiegi
Klient bazy danych MariaDB projektu Multimap

# Licencja
Niniejszy program jest wolnym oprogramowaniem – możesz go rozpowszechniać dalej i/lub modyfikować na warunkach Powszechnej Licencji Publicznej GNU wydanej przez Fundację Wolnego Oprogramowania, według wersji 3 tej Licencji lub dowolnej z późniejszych wersji.
Niniejszy program rozpowszechniany jest z nadzieją, iż będzie on użyteczny – jednak BEZ ŻADNEJ GWARANCJI, nawet domyślnej gwarancji PRZYDATNOŚCI HANDLOWEJ albo PRZYDATNOŚCI DO OKREŚLONYCH ZASTOSOWAŃ. Bliższe informacje na ten temat można uzyskać z Powszechnej Licencji Publicznej GNU.
Kopia Powszechnej Licencji Publicznej GNU powinna zostać ci dostarczona razem z tym programem. Jeżeli nie została dostarczona, odwiedź http://www.gnu.org/licenses/.

# Obsługa programu

## Obsługa techniczna

### Aktualizacja listy plików

W celu aktualizacji listy plików należy wybrać opcje `Program -> Skanuj katalog`. Zależnie od organizacji plików możliwe jest wyszukiwanie dokumentów w folderach o organizacji `Rocznik/Operat`, `Operat` oraz skanowanie dowolnej organizacji pobierając z nazwy pliku informacje o operacie.
Opcja skanowania katalogu powinna być używana przez administratora, jedynie przed rozpoczęciem projektu, lub w przypadku potrzeby uzupełnienia dodanych dokumentów.

### Aktualizacja typów dokumentów

W celu nadania rodzajów dokumentów należy wybrać opcję `Program -> Aktualizuj dokumenty`. Po wybraniu pliku tekstowego o strukturze `Dokument Rodzaj` dla podanych dokumentów zaktualizowany zostanie ich rodzaj.
Istnieje możliwość automatycznej aktualizacji słownika rodzajów dokumentów przez wybór opcji `Dodawaj nowe wartości do słownika`.

## Edycja właściwości dokumentów i operatów

Z poziomu widoku tabeli operatów i dokumentów istnieje możliwość aktualizacji wybranych pól. Edycja pola jest możliwa poprzez podwójne kliknięcie w edytowane pole. Dane zapisywane są automatycznie po zakończeniu edycji.

## Manipulacja obrazem dokumentu

W celu zmiany opcji wyświetlania dokumentu należy użyć poleceń z menu `Widok` zgodnie z ich opisem.

## Edycja zmian związanych z dokumentem

### Blokowanie dokumentu

W celu zablokowania dokumentu i uniemożliwienia innym użytkownikom edycji dokumentu należy zastosować opcję `Dokument -> Zablokuj dokument`. Po poprawnym zablokowaniu dokumentu jego edycja będzie możliwa.
W przypadku, gdy po naciśnięciu odpowiedniej opcji dokument pozostaje niezablokowany, istnieje możliwość, że został on zablokowany przez innego użytkownika.

### Zapis zmian dokumentu

Po wprowadzeniu wszystkich zmian związanych z dokumentem i ich weryfikacji, w celu zapisu zmian należy użyć opcji `Dokument -> Zapisz`. Zapisany dokument zostanie oznaczony jako przetworzony.
Należy upewnić się, że pozyskane zostały wszystkie informacje. Dokument oznaczony jako przetworzony nie będzie więcej przydzielony żadnemu użytkownikowi. Jego edycja będzie jednak dalej możliwa.
Zapis zmian dokumentu nie powoduje jego odblokowania. Dokument będzie nadal przypisany do bieżącego użytkownika.

### Odblokowanie dokumentu

W celu zwolnienia blokady dokumentu należy wybrać opcje `Dokument -> Odblokuj dokument`. Wszystkie niezapisane zmiany związane z dokumentem zostaną porzucone.

### Nawigacja pomiędzy dokumentami

Wyboru dokumentu można dokonać poprzez znaznaczenie do na liście dokumentów. Istnieje także możliwość szybkiego wyboru dokumentu z listy zablokowanych przez użytkownika dokumentów poprzez podwójne kliknięcie na dokument w przyborniku `Zablokowane`.
Klawisze nawigacji `Następny dokument` oraz `Poprzedni dokument` umożliwiają szybkie przejście pomiędzy zablokowanymi dokumentami.

### Szybka blokada dokumentów

Akcja `Dokument -> Zablokuj kolejne dokumenty` umożliwia szybką blokade `N` (ustalane w opcjach programu) dokumentów oznaczonych jako nieprzetworzone.

### Dodawanie grupy zmian

Dla wyświetlonego dokumentu w celu utworzenia nowej grupy zmian należy użyć opcji `Dokument -> Dodaj grupę zmian`. Dla dodanej grupy należy uzupełnić wszystkie pola formularza. Niezapisana, poprawnie wprowadzona grupa zmian będzie oznaczona ikoną `+`.
Numery działek mogą zostać wprowadzane jeden po drugim, lub automatycznie zinterpretowane (oddzielone spacją, przecinkiem, średnikiem lub kropką).
Zmiana dokumentu nie spowoduje utraty informacji o dodanej zmianie. W przypadku błędów w formularzu, obok numeru grupy wyświetlona zostanie ikona błędu.

### Usuwanie grupy zmian

Dla wyświetlonego dokumentu w celu usunięcia grupy zmian należy użyć opcji `Dokument -> Usuń grupę zmian`. Usunięcie grupy spowoduje przywrócenie jej właściwości do stanu początkowego (wycofanie edytowanych pól) oraz jej oznaczenie jako usuwana.
Zmiana dokumentu nie spowoduje utraty informacji o usuniętej zmianie. Dla oznaczonej do usunięcia zmiany nie ma możliwości edycji jej właściwości. W celu przywrócenia usuniętej zmiany należy użyć opcji `Dokument -> Cofnij zmiany grupy`.

### Edycja istniejącej grupy zmian

Zmiana wartości dowolnego pola opisującego grupę zmian spowoduje oznaczenie jej jako edytowaną. W przypadku błędów w formularzu, obok numeru grupy wyświetlona zostanie ikona błędu. W celu przywrócenia oryginalnych wartości należy użyć opcji `Dokument -> Cofnij zmiany grupy`.

### Wycofywanie zmian

Akcja `Dokument -> Cofnij zmiany grupy` umożliwia wycofanie wartości właściwości zmiany, usunięcie dodanej zmiany lub przywrócenie usuniętej zmiany.

## Zmiana opcji programu

### Zmiana rodzaju rezerwowanych dokumentów

Zakładka `Rezerwacje` umożliwia ustalenie liczby rezerwowanych za pomocą akcji `Dokument -> Zablokuj kolejne dokumenty` dokumentów oraz umożliwia zdefiniowanie, jakiego rodzaju dokumenty zostaną zarezerwowane.

### Zmiana widoczności kolumn w tabelach

Zakładki `Dokumenty` oraz `Operaty` umożliwiają ustalenie, które kolumny mają być widoczne podczas przeglądania dokumentów. Brak wskazanych kolumn spowoduje wyświetlanie wszystkich kolumn.
