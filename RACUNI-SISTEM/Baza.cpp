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
void Baza::DodajArtikal(string Naziv, string SerijskiBroj, double Cijena) {
	sqlite3_stmt* stmtProvjera;
	int count = 0;
	string sqlProvjera =
		"SELECT COUNT (*) FROM Artikli WHERE SerijskiBroj=?;";
	int rc = sqlite3_prepare_v2(db, sqlProvjera.c_str(), -1, &stmtProvjera, nullptr);
	if (rc != SQLITE_OK) {
		cout << "GRESKA PRI PRIPREMI PROVJERE:" << sqlite3_errmsg(db) << endl;
		return;
	}
	sqlite3_bind_text(stmtProvjera, 1, SerijskiBroj.c_str(), -1, SQLITE_TRANSIENT);
	if (sqlite3_step(stmtProvjera) == SQLITE_ROW) {
		count = sqlite3_column_int(stmtProvjera, 0);
	}
	sqlite3_finalize(stmtProvjera);

	if (count > 0) {
		cout << "GRESKA PRI DODAVANJU ARTIKLA-ARTIKAL VEC POSTOJI" << endl;
		return;
	}
	sqlite3_stmt* stmt;
	string sql =
		"INSERT INTO Artikli(Naziv, SerijskiBroj, Cijena) VALUES(?, ?, ?);";
	int rc1 = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
	if (rc1 != SQLITE_OK) {
		cout << "GRESKA PRI PRIPREMI DODAVANJA ARTIKLA:" << sqlite3_errmsg(db) << endl;
		return;
	}
	sqlite3_bind_text(stmt, 1, Naziv.c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(stmt, 2, SerijskiBroj.c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(stmt, 3, to_string(Cijena).c_str(), -1, SQLITE_TRANSIENT);

	rc1 = sqlite3_step(stmt);
	sqlite3_finalize(stmt);
	if (rc1 != SQLITE_DONE) {
		cout << "GRESKA PRI DODAVANJU ARTIKLA:" << sqlite3_errmsg(db) << endl;
		return;
	}
}
void Baza::PrikaziSveArtikle() {
	sqlite3_stmt* stmt;
	string sql =
		"SELECT * FROM Artikli;";
	int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
	if (rc != SQLITE_OK) {
		cout << "GRESKA PRI PRIPREMI PRIKAZA:" << sqlite3_errmsg(db) << endl;
		return;
	}

	while (sqlite3_step(stmt) == SQLITE_ROW) {
		int ID = sqlite3_column_int(stmt, 0);
		string Naziv = (const char*)sqlite3_column_text(stmt, 1);
		string SerijskiBroj = (const char*)sqlite3_column_text(stmt, 2);
		double Cijena = sqlite3_column_double(stmt, 3);

		cout << "ID:" << ID << endl;
		cout << "Naziv Artikla:" << Naziv << endl;
		cout << "Serijski broj:" << SerijskiBroj << endl;
		cout << "Cijena:" << Cijena << " KM" << endl;
		cout << "---" << endl;
	}
	sqlite3_finalize(stmt);
}
void Baza::DodajKupca(string ImePrezime, string JMBG) {
	sqlite3_stmt* stmtProvjera;
	int count = 0;
	string sqlProvjera =
		"SELECT COUNT (*) FROM Kupci WHERE JMBG=?;";
	int rc = sqlite3_prepare_v2(db, sqlProvjera.c_str(), -1, &stmtProvjera, nullptr);
	if (rc != SQLITE_OK) {
		cout << "GRESKA PRI PRIPREMI PROVJERE:" << sqlite3_errmsg(db) << endl;
		return;
	}
	sqlite3_bind_text(stmtProvjera, 1, JMBG.c_str(), -1, SQLITE_TRANSIENT);
	if (sqlite3_step(stmtProvjera) == SQLITE_ROW) {
		count = sqlite3_column_int(stmtProvjera, 0);
	}
	sqlite3_finalize(stmtProvjera);
	if (count > 0) {
		cout << "GRESKA PRI DODAVANJU KUPCA-KUPAC VEC POSTOJI" << endl;
		return;
	}
	sqlite3_stmt* stmt;
	string sql =
		"INSERT INTO Kupci(ImePrezime, JMBG) VALUES(?, ?);";
	int rc1 = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
	if (rc1 != SQLITE_OK) {
		cout << "GRESKA PRI PRIPREMI PROVJERE:" << sqlite3_errmsg(db) << endl;
		return;
	}
	sqlite3_bind_text(stmt, 1, ImePrezime.c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(stmt, 2, JMBG.c_str(), -1, SQLITE_TRANSIENT);

	rc1 = sqlite3_step(stmt);
	sqlite3_finalize(stmt);
	if (rc1 != SQLITE_DONE) {
		cout << "GRESKA PRI DODAVANJU KUPCA:" << sqlite3_errmsg(db) << endl;
		return;
	}
}
void Baza::PrikaziSveKupce() {
	sqlite3_stmt* stmt;
	string sql =
		"SELECT * FROM Kupci;";
	int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
	if (rc != SQLITE_OK) {
		cout << "GRESKA PRI PRIPREMI PRIKAZA:" << sqlite3_errmsg(db) << endl;
		return;
	}
	while (sqlite3_step(stmt) == SQLITE_ROW) {
		int ID = sqlite3_column_int(stmt, 0);
		string ImePrezime = (const char*)sqlite3_column_text(stmt, 1);
		string JMBG = (const char*)sqlite3_column_text(stmt, 2);

		cout << "ID:" << ID << endl;
		cout << "Ime i Prezime Kupca:" << ImePrezime << endl;
		cout << "JMBG:" << JMBG << endl;
		cout << "---" << endl;
	}
	sqlite3_finalize(stmt);
}