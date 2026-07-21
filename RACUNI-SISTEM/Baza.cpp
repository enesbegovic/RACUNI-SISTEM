#include<iostream>
#include<string>
#include<vector>
#include"Baza.h"
using namespace std;
Baza::Baza() {
	sqlite3_open("RACUNI-SISTEM.db", &db);
	sqlite3_exec(db, "PRAGMA foreign_keys=ON;", nullptr, nullptr, nullptr);
}
Baza::~Baza() {
	sqlite3_close(db);
}
void Baza::IzvrsiUpit(string sql, string PORUKAGRESKE) {
	char* errMsg = nullptr;
	int rc = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg);
	if (rc != SQLITE_OK) {
		cout << PORUKAGRESKE << ":" << errMsg << endl;
		sqlite3_free(errMsg);
	}
}
void Baza::KreirajTabele() {
	string sqlArtikli =
		"CREATE TABLE IF NOT EXISTS Artikli("
		"ID INTEGER PRIMARY KEY AUTOINCREMENT,"
		"Naziv TEXT NOT NULL,"
		"SerijskiBroj TEXT NOT NULL UNIQUE,"
		"Cijena REAL NOT NULL);";
	IzvrsiUpit(sqlArtikli, "PORUKA GRESKE");

	string sqlKupci =
		"CREATE TABLE IF NOT EXISTS Kupci("
		"ID INTEGER PRIMARY KEY AUTOINCREMENT,"
		"ImePrezime TEXT NOT NULL,"
		"JMBG TEXT NOT NULL UNIQUE);";
	IzvrsiUpit(sqlKupci, "PORUKA GRESKE");

	string sqlRacun =
		"CREATE TABLE IF NOT EXISTS Racuni("
		"ID INTEGER PRIMARY KEY AUTOINCREMENT,"
		"KupacID INTEGER NOT NULL,"
		"BrojRacuna TEXT NOT NULL UNIQUE,"
		"DatumKupovine TEXT NOT NULL,"
		"FOREIGN KEY (KupacID) REFERENCES Kupci(ID) ON DELETE RESTRICT);";
	IzvrsiUpit(sqlRacun, "PORUKA GRESKE");

	string sqlStavkeRacuna =
		"CREATE TABLE IF NOT EXISTS Stavke("
		"ID INTEGER PRIMARY KEY AUTOINCREMENT,"
		"RacunID INTEGER NOT NULL,"
		"ArtikalID INTEGER NOT NULL,"
		"Kolicina REAL NOT NULL,"
		"FOREIGN KEY (RacunID) REFERENCES Racuni(ID) ON DELETE CASCADE,"
		"FOREIGN KEY (ArtikalID) REFERENCES Artikli(ID) ON DELETE RESTRICT);";
	IzvrsiUpit(sqlStavkeRacuna, "PORUKA GRESKE");
}
void Baza::KreirajIndex() {
	IzvrsiUpit("CREATE INDEX IF NOT EXISTS idx_racuni_kupacid ON Kupci(ID);", "GRESKA PRI KREIRANJU INDEXA");
	IzvrsiUpit("CREATE INDEX IF NOT EXISTS idx_stavke_racunid ON Racuni(ID);", "GRESKA PRI KREIRANJU INDEXA");
	IzvrsiUpit("CREATE INDEX IF NOT EXISTS idx_stavke_artikalid ON Artikli(ID);", "GRESKA PRI KREIRANJU INDEXA");
}