# RACUNI-SISTEM

![Build and Test](https://github.com/enesbegovic/RACUNI-SISTEM/actions/workflows/build.yml/badge.svg)

Konzolna C++ aplikacija za izdavanje računa (artikli, kupci, računi sa stavkama), sa SQLite bazom podataka. Sav rad sa bazom ide preko prepared statements.

## Tehnologije

- C++ (Visual Studio, konzolna aplikacija)
- SQLite3
- Git / GitHub za verzionisanje

## Struktura baze

### Artikli
- `ID` (PRIMARY KEY AUTOINCREMENT)
- `Naziv`
- `BrojArtikla` (UNIQUE)
- `Cijena`

### Kupci
- `ID` (PRIMARY KEY AUTOINCREMENT)
- `ImePrezime`
- `JMBG` (UNIQUE)

### Racuni
- `ID` (PRIMARY KEY AUTOINCREMENT)
- `KupacID` (FOREIGN KEY -> Kupci.ID)
- `Datum`

### Stavke
- `ID` (PRIMARY KEY AUTOINCREMENT)
- `RacunID` (FOREIGN KEY -> Racuni.ID)
- `ArtikalID` (FOREIGN KEY -> Artikli.ID)
- `Kolicina`
- `UkupanIznosRacuna`

## Funkcionalnosti

**Artikli**
- Dodavanje / prikaz artikala, provjera duplikata prije unosa

**Kupci**
- Dodavanje / prikaz kupaca, provjera duplikata prije unosa

**Racuni**
- Izdavanje računa sa više stavki u jednoj transakciji (BEGIN/COMMIT/ROLLBACK) — ako kupac ili artikal ne postoje, cijela transakcija se poništava i ni račun ni stavke se ne dodaju

## Ključni koncepti korišteni u projektu

- Prepared statements za sve upite (bind/step/finalize umjesto sqlite3_exec)
- FOREIGN KEY constraint-i sa ON DELETE RESTRICT (PRAGMA foreign_keys=ON)
- Transakcijska zaštita (BEGIN/COMMIT/ROLLBACK) za operacije koje mijenjaju više tabela odjednom
- Provjera postojanja/duplikata prije INSERT-a (SELECT COUNT)
- Paznja na redoslijed bind() poziva - mora odgovarati redoslijedu ? znakova u upitu, ne redoslijedu promjenljivih u kodu

## Pokretanje

1. Otvoriti `.slnx` fajl u Visual Studio-u
2. Build (Debug/x64)
3. Pokrenuti - baza (`RACUNI-SISTEM.db`) se automatski kreira pri prvom pokretanju ako ne postoji

## Testiranje

Projekat sadrži 7 jediničnih testova (Google Test) koji pokrivaju rad sa Artiklima, Kupcima i izdavanjem Računa sa Stavkama, uključujući provjeru transakcijske zaštite (rollback pri nepostojećem kupcu ili artiklu).

Testovi se nalaze u `RACUNI-SISTEM_Testovi` projektu i mogu se pokrenuti kroz Visual Studio Test Explorer (Test → Run All Tests).

## CI/CD

Svaki push na `master` granu automatski pokreće build i sve testove preko GitHub Actions (vidi `.github/workflows/build.yml`).
