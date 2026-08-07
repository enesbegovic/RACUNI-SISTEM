# Racuni-Sistem

Konzolna C++ aplikacija za upravljanje kupcima, artiklima i računima, sa SQLite bazom podataka.

## Napredni koncepti korišteni
- Prepared statements (sqlite3_prepare_v2, bind, step) — zaštita od SQL injection
- Transakcije (BEGIN/COMMIT/ROLLBACK)
- FOREIGN KEY constraints sa ON DELETE RESTRICT/CASCADE
- ISO format datuma i julianday() funkcije
- CREATE INDEX i EXPLAIN QUERY PLAN

## Struktura
- Kupci, Artikli, Racuni, StavkeRacuna (vezna tabela za više artikala po računu)
