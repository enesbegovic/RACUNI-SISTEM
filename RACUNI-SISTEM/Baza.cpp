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
void Baza::DodajRacunIStavkeRacuna(string KupacJMBG, string BrojRacun, string DatumKupovine, vector<pair<string, double>>StavkeRacuna) {
	IzvrsiUpit("BEGIN TRANSACTION;", "GRESKA PRI POKRETANJU TRANSAKNCIJE");
	sqlite3_stmt* stmtProvjeraKupca;
	int KupacID = 0;
	string sqlProvjerKupca =
		"SELECT ID FROM Kupci WHERE JMBG=?;";
	int rc = sqlite3_prepare_v2(db, sqlProvjerKupca.c_str(), -1, &stmtProvjeraKupca, nullptr);
	if (rc != SQLITE_OK) {
		cout << "GRESKA PRI PRIPREMI PROVJERE KUPCA:" << sqlite3_errmsg(db) << endl;
		IzvrsiUpit("ROLLBACK;", "GRESKA PRI ROLLBACK-u");
		return;
	}
	sqlite3_bind_text(stmtProvjeraKupca, 1, KupacJMBG.c_str(), -1, SQLITE_TRANSIENT);
	if (sqlite3_step(stmtProvjeraKupca) == SQLITE_ROW) {
		KupacID = sqlite3_column_int(stmtProvjeraKupca, 0);
	}
	sqlite3_finalize(stmtProvjeraKupca);
	if (KupacID == 0) {
		cout << "GRESKA PRI DODAVANJU RACUNA-TRAZENI KUPAC NE POSTOJI" << endl;
		IzvrsiUpit("ROLLBACK;", "GRESKA PRI ROLLBACK-u");
		return;
	}
	sqlite3_stmt* stmtRacun;
	string sqlRacun =
		"INSERT INTO Racuni(KupacID, BrojRacuna, DatumKupovine) VALUES(?, ?, ?);";
	int rc1 = sqlite3_prepare_v2(db, sqlRacun.c_str(), -1, &stmtRacun, nullptr);
	if (rc1 != SQLITE_OK) {
		cout << "GRESKA PRI PRIPREMI DODAVANJA RACUNA:" << sqlite3_errmsg(db) << endl;
		IzvrsiUpit("ROLLBACK;", "GRESKA PRI ROLLBACK-u");
		return;
	}
	sqlite3_bind_text(stmtRacun, 1, to_string(KupacID).c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(stmtRacun, 2, BrojRacun.c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(stmtRacun, 3, DatumKupovine.c_str(), -1, SQLITE_TRANSIENT);
	rc1 = sqlite3_step(stmtRacun);
	sqlite3_finalize(stmtRacun);
	if (rc1 != SQLITE_DONE) {
		cout << "GRESKA PRI DODAVANJU RACUNA:" << sqlite3_errmsg(db) << endl;
		IzvrsiUpit("ROLLBACK;", "GRESKA PRI ROLLBACK-u");
		return;
	}

	int RacunID = (int)sqlite3_last_insert_rowid(db);
	for (auto& x : StavkeRacuna) {
		string SerijskiBroj = x.first;
		double Kolicina = x.second;

		sqlite3_stmt* stmtArtikal;
		int ArtikalID = 0;
		string sqlArtikal =
			"SELECT ID FROM Artikli WHERE SerijskiBroj=?;";
		int rc2 = sqlite3_prepare_v2(db, sqlArtikal.c_str(), -1, &stmtArtikal, nullptr);
		if (rc2 != SQLITE_OK) {
			cout << "GRESKA PRI PRIPREMI PROVJERE ARTIKLA:" << sqlite3_errmsg(db) << endl;
			IzvrsiUpit("ROLLBACK;", "GRESKA PRI ROLLBACK-u");
			return;
		}
		sqlite3_bind_text(stmtArtikal, 1, SerijskiBroj.c_str(), -1, SQLITE_TRANSIENT);
		if (sqlite3_step(stmtArtikal) == SQLITE_ROW) {
			ArtikalID = sqlite3_column_int(stmtArtikal, 0);
		}
		sqlite3_finalize(stmtArtikal);
		if (ArtikalID == 0) {
			cout << "GRESKA PRI DODAVANJU STAVKI RACUNA-NE POSTOJI TRAZENI ARTIKAL" << endl;
			IzvrsiUpit("ROLLBACK;", "GRESKA PRI ROLLBACK-u");
			return;
		}
		sqlite3_stmt* stmtStavke;
		string sqlStavke =
			"INSERT INTO Stavke(RacunID, ArtikalID, Kolicina) VALUES(?, ?, ?);";
		int rc3 = sqlite3_prepare_v2(db, sqlStavke.c_str(), -1, &stmtStavke, nullptr);
		if (rc3 != SQLITE_OK) {
			cout << "GRESKA PRI PRIPREMI DODAVANJA STAVKI RACUNA:" << sqlite3_errmsg(db) << endl;
			IzvrsiUpit("ROLLBACK;", "GRESKA PRI ROLLBACK-u");
			return;
		}
		sqlite3_bind_text(stmtStavke, 1, to_string(RacunID).c_str(), -1, SQLITE_TRANSIENT);
		sqlite3_bind_text(stmtStavke, 2, to_string(ArtikalID).c_str(), -1, SQLITE_TRANSIENT);
		sqlite3_bind_text(stmtStavke, 3, to_string(Kolicina).c_str(), -1, SQLITE_TRANSIENT);
		rc3 = sqlite3_step(stmtStavke);
		sqlite3_finalize(stmtStavke);
		if (rc3 != SQLITE_DONE) {
			cout << "GRESKA PRI DODAVANJU STAVKI RACUNA:" << sqlite3_errmsg(db) << endl;
			IzvrsiUpit("ROLLBACK;", "GRESKA PRI ROLLBACK-u");
			return;
		}
	}
	IzvrsiUpit("COMMIT;", "GRESKA PRI COMMIT-u");
	cout << "TRANSAKCIJA ZAVRSENA-RACUN I STAVKE USPJESNO DODANI" << endl;
}
void Baza::PrikaziSveRacune() {
	sqlite3_stmt* stmt;
	string sql =
		"SELECT Kupci.ImePrezime, Racuni.BrojRacuna, Racuni.DatumKupovine, Artikli.Naziv, Artikli.SerijskiBroj, Artikli.Cijena, Stavke.Kolicina "
		"FROM Racuni "
		"JOIN Kupci ON Racuni.KupacID=Kupci.ID "
		"JOIN Stavke ON Racuni.ID=Stavke.RacunID "
		"JOIN Artikli ON Stavke.ArtikalID=Artikli.ID "
		"ORDER BY Racuni.ID;";
	int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
	if (rc != SQLITE_OK) {
		cout << "GRESKA PRI PRIPREMI PRIKAZA:" << sqlite3_errmsg(db) << endl;
		return;
	}
	string trenutniRacun = "";
	double UkupanRacun = 0;
	while (sqlite3_step(stmt)==SQLITE_ROW) {
		string ImePrezime = (const char*)sqlite3_column_text(stmt, 0);
		string BrojRacuna = (const char*)sqlite3_column_text(stmt, 1);
		string Datum = (const char*)sqlite3_column_text(stmt, 2);
		string Naziv = (const char*)sqlite3_column_text(stmt, 3);
		string SerijskiBroj = (const char*)sqlite3_column_text(stmt, 4);
		double Cijena = sqlite3_column_double(stmt, 5);
		double Kolicina = sqlite3_column_double(stmt, 6);

		
		if (trenutniRacun != BrojRacuna) {
			if (trenutniRacun != "") {
				cout << "Ukupan Iznos Racuna:" << UkupanRacun << endl;
				cout << "---" << endl;
			}
			cout << "Ime i Prezime Kupca:" << ImePrezime << endl;
			cout << "Broj Racuna:" << BrojRacuna << endl;
			cout << "Datum Kreiranja Racuna:" << Datum << endl;
			trenutniRacun = BrojRacuna;
			UkupanRacun = 0;
			cout << "Kupljeni Artikli:" << endl;
		}
		
		cout << Naziv << " -Serijski Broj:" << SerijskiBroj << " -Cijena:" << Cijena << " KM -Kolicina:" << Kolicina << endl;
		UkupanRacun += Cijena * Kolicina;
	}
	if (trenutniRacun != "") {
		cout << "Ukupan Iznos Racuna:" << UkupanRacun << endl;
		cout << "---" << endl;
	}
	sqlite3_finalize(stmt);
}
void Baza::PrikaziKupcaSaNajvecimTroskom() {
	sqlite3_stmt* stmt;
	string sql =
		"SELECT Kupci.ImePrezime, Racuni.BrojRacuna, Racuni.DatumKupovine, "
		"SUM(Artikli.Cijena * Stavke.Kolicina) AS UkupanRacun "
		"FROM Racuni "
		"JOIN Kupci ON Racuni.KupacID=Kupci.ID "
		"JOIN Stavke ON Racuni.ID=Stavke.RacunID "
		"JOIN Artikli ON Stavke.ArtikalID=Artikli.ID "
		"GROUP BY Racuni.ID "
		"ORDER BY UkupanRacun DESC "
		"LIMIT 1;";
	int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
	if (rc != SQLITE_OK) {
		cout << "GRESKA PRI PRIPREMI PRIKAZA:" << sqlite3_errmsg(db) << endl;
		return;
	}
	if (sqlite3_step(stmt) == SQLITE_ROW) {
		string ImePrezime = (const char*)sqlite3_column_text(stmt, 0);
		string BrojRacuna = (const char*)sqlite3_column_text(stmt, 1);
		string Datum = (const char*)sqlite3_column_text(stmt, 2);
		double UkupanIznos = sqlite3_column_double(stmt, 3);

		cout << "Ime i Prezime Kupca:" << ImePrezime << endl;
		cout << "Broj Racuna:" << BrojRacuna << endl;
		cout << "Datum Kreiranja Racuna:" << Datum << endl;
		cout << "Ukupan Iznos Racuna:" << UkupanIznos << endl;
	}
	else {
		cout << "NEMA PODATAKA" << endl;
	}
	sqlite3_finalize(stmt);
}