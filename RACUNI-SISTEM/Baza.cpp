#include<iostream>
#include <stdexcept>
#include<string>
#include<vector>
#include"Baza.h"
using namespace std;
Baza::Baza(string putanjaBaze) {
	sqlite3_open(putanjaBaze.c_str(), &db);
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

	string sqlKorisnici =
		"CREATE TABLE IF NOT EXISTS Korisnici("
		"ID INTEGER PRIMARY KEY AUTOINCREMENT,"
		"UserName TEXT NOT NULL UNIQUE,"
		"PassWordHash TEXT NOT NULL,"
		"Uloga TEXT NOT NULL DEFAULT 'korisnik');";
	IzvrsiUpit(sqlKorisnici, "GRESKA PRI KREIRANJU TABELE KORISNICI");
}
void Baza::KreirajIndex() {
	IzvrsiUpit("CREATE INDEX IF NOT EXISTS idx_racuni_kupacid ON Kupci(ID);", "GRESKA PRI KREIRANJU INDEXA");
	IzvrsiUpit("CREATE INDEX IF NOT EXISTS idx_stavke_racunid ON Racuni(ID);", "GRESKA PRI KREIRANJU INDEXA");
	IzvrsiUpit("CREATE INDEX IF NOT EXISTS idx_stavke_artikalid ON Artikli(ID);", "GRESKA PRI KREIRANJU INDEXA");
}
void Baza::DodajArtikal(string Naziv, string SerijskiBroj, double Cijena) {
	sqlite3_stmt* rawStmtProvjera;
	int count = 0;
	string sqlProvjera =
		"SELECT COUNT (*) FROM Artikli WHERE SerijskiBroj=?;";
	int rc = sqlite3_prepare_v2(db, sqlProvjera.c_str(), -1, &rawStmtProvjera, nullptr);
	if (rc != SQLITE_OK) {
		cout << "GRESKA PRI PRIPREMI PROVJERE:" << sqlite3_errmsg(db) << endl;
		return;
	}
	StmtPtr stmtProvjera(rawStmtProvjera);
	sqlite3_bind_text(stmtProvjera.get(), 1, SerijskiBroj.c_str(), -1, SQLITE_TRANSIENT);
	if (sqlite3_step(stmtProvjera.get()) == SQLITE_ROW) {
		count = sqlite3_column_int(stmtProvjera.get(), 0);
	}

	if (count > 0) {
		cout << "GRESKA PRI DODAVANJU ARTIKLA-ARTIKAL VEC POSTOJI" << endl;
		return;
	}
	sqlite3_stmt* rawStmt;
	string sql =
		"INSERT INTO Artikli(Naziv, SerijskiBroj, Cijena) VALUES(?, ?, ?);";
	int rc1 = sqlite3_prepare_v2(db, sql.c_str(), -1, &rawStmt, nullptr);
	if (rc1 != SQLITE_OK) {
		cout << "GRESKA PRI PRIPREMI DODAVANJA ARTIKLA:" << sqlite3_errmsg(db) << endl;
		return;
	}
	StmtPtr stmt(rawStmt);
	sqlite3_bind_text(stmt.get(), 1, Naziv.c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(stmt.get(), 2, SerijskiBroj.c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(stmt.get(), 3, to_string(Cijena).c_str(), -1, SQLITE_TRANSIENT);

	rc1 = sqlite3_step(stmt.get());
	if (rc1 != SQLITE_DONE) {
		cout << "GRESKA PRI DODAVANJU ARTIKLA:" << sqlite3_errmsg(db) << endl;
		return;
	}
	cout << "ARTIKAL USPJESNO DODAN" << endl;
}
void Baza::PrikaziSveArtikle() {
	sqlite3_stmt* rawStmt;
	string sql =
		"SELECT * FROM Artikli;";
	int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &rawStmt, nullptr);
	if (rc != SQLITE_OK) {
		cout << "GRESKA PRI PRIPREMI PRIKAZA:" << sqlite3_errmsg(db) << endl;
		return;
	}
	StmtPtr stmt(rawStmt);
	while (sqlite3_step(stmt.get()) == SQLITE_ROW) {
		int ID = sqlite3_column_int(stmt.get(), 0);
		string Naziv = (const char*)sqlite3_column_text(stmt.get(), 1);
		string SerijskiBroj = (const char*)sqlite3_column_text(stmt.get(), 2);
		double Cijena = sqlite3_column_double(stmt.get(), 3);

		cout << "ID:" << ID << endl;
		cout << "Naziv Artikla:" << Naziv << endl;
		cout << "Serijski broj:" << SerijskiBroj << endl;
		cout << "Cijena:" << Cijena << " KM" << endl;
		cout << "---" << endl;
	}
}
int Baza::brojArtikala() {
	sqlite3_stmt* rawStmt;
	string sql =
		"SELECT COUNT (*) FROM Artikli;";
	int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &rawStmt, nullptr);
	if (rc != SQLITE_OK) {
		cout << "GRESKA PRI PRIPREMI BROJANJA ARTIKALA:" << sqlite3_errmsg(db) << endl;
		return -1;
	}
	StmtPtr stmt(rawStmt);
	int count = 0;
	if (sqlite3_step(stmt.get()) == SQLITE_ROW) {
		count = sqlite3_column_int(stmt.get(), 0);
	}
	return count;
}
void Baza::DodajKupca(string ImePrezime, string JMBG) {
	sqlite3_stmt* rawStmtProvjera;
	int count = 0;
	string sqlProvjera =
		"SELECT COUNT (*) FROM Kupci WHERE JMBG=?;";
	int rc = sqlite3_prepare_v2(db, sqlProvjera.c_str(), -1, &rawStmtProvjera, nullptr);
	if (rc != SQLITE_OK) {
		cout << "GRESKA PRI PRIPREMI PROVJERE:" << sqlite3_errmsg(db) << endl;
		return;
	}
	StmtPtr stmtProvjera(rawStmtProvjera);
	sqlite3_bind_text(stmtProvjera.get(), 1, JMBG.c_str(), -1, SQLITE_TRANSIENT);
	if (sqlite3_step(stmtProvjera.get()) == SQLITE_ROW) {
		count = sqlite3_column_int(stmtProvjera.get(), 0);
	}
	if (count > 0) {
		cout << "GRESKA PRI DODAVANJU KUPCA-KUPAC VEC POSTOJI" << endl;
		return;
	}
	sqlite3_stmt* rawStmt;
	string sql =
		"INSERT INTO Kupci(ImePrezime, JMBG) VALUES(?, ?);";
	int rc1 = sqlite3_prepare_v2(db, sql.c_str(), -1, &rawStmt, nullptr);
	if (rc1 != SQLITE_OK) {
		cout << "GRESKA PRI PRIPREMI PROVJERE:" << sqlite3_errmsg(db) << endl;
		return;
	}
	StmtPtr stmt(rawStmt);
	sqlite3_bind_text(stmt.get(), 1, ImePrezime.c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(stmt.get(), 2, JMBG.c_str(), -1, SQLITE_TRANSIENT);

	rc1 = sqlite3_step(stmt.get());
	if (rc1 != SQLITE_DONE) {
		cout << "GRESKA PRI DODAVANJU KUPCA:" << sqlite3_errmsg(db) << endl;
		return;
	}
	cout << "KUPAC USPIJESNO DODAN" << endl;
}
void Baza::PrikaziSveKupce() {
	sqlite3_stmt* rawStmt;
	string sql =
		"SELECT * FROM Kupci;";
	int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &rawStmt, nullptr);
	if (rc != SQLITE_OK) {
		cout << "GRESKA PRI PRIPREMI PRIKAZA:" << sqlite3_errmsg(db) << endl;
		return;
	}
	StmtPtr stmt(rawStmt);
	while (sqlite3_step(stmt.get()) == SQLITE_ROW) {
		int ID = sqlite3_column_int(stmt.get(), 0);
		string ImePrezime = (const char*)sqlite3_column_text(stmt.get(), 1);
		string JMBG = (const char*)sqlite3_column_text(stmt.get(), 2);

		cout << "ID:" << ID << endl;
		cout << "Ime i Prezime Kupca:" << ImePrezime << endl;
		cout << "JMBG:" << JMBG << endl;
		cout << "---" << endl;
	}
}
int Baza::brojKupaca() {
	sqlite3_stmt* rawStmt;
	string sql =
		"SELECT COUNT (*) FROM Kupci;";
	int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &rawStmt, nullptr);
	if (rc != SQLITE_OK) {
		cout << "GRESKA PRI PRIPREMI BROJANJA KUPACA:" << sqlite3_errmsg(db) << endl;
		return -1;
	}
	StmtPtr stmt(rawStmt);
	int count = 0;
	if (sqlite3_step(stmt.get()) == SQLITE_ROW) {
		count = sqlite3_column_int(stmt.get(), 0);
	}
	return count;
}
void Baza::DodajRacunIStavkeRacuna(string KupacJMBG, string BrojRacun, string DatumKupovine, vector<pair<string, double>>StavkeRacuna) {
	IzvrsiUpit("BEGIN TRANSACTION;", "GRESKA PRI POKRETANJU TRANSAKNCIJE");
	sqlite3_stmt* rawStmtProvjeraKupca;
	int KupacID = 0;
	string sqlProvjerKupca =
		"SELECT ID FROM Kupci WHERE JMBG=?;";
	int rc = sqlite3_prepare_v2(db, sqlProvjerKupca.c_str(), -1, &rawStmtProvjeraKupca, nullptr);
	if (rc != SQLITE_OK) {
		cout << "GRESKA PRI PRIPREMI PROVJERE KUPCA:" << sqlite3_errmsg(db) << endl;
		IzvrsiUpit("ROLLBACK;", "GRESKA PRI ROLLBACK-u");
		return;
	}
	StmtPtr stmtProvjeraKupca(rawStmtProvjeraKupca);
	sqlite3_bind_text(stmtProvjeraKupca.get(), 1, KupacJMBG.c_str(), -1, SQLITE_TRANSIENT);
	if (sqlite3_step(stmtProvjeraKupca.get()) == SQLITE_ROW) {
		KupacID = sqlite3_column_int(stmtProvjeraKupca.get(), 0);
	}
	if (KupacID == 0) {
		cout << "GRESKA PRI DODAVANJU RACUNA-TRAZENI KUPAC NE POSTOJI" << endl;
		IzvrsiUpit("ROLLBACK;", "GRESKA PRI ROLLBACK-u");
		return;
	}
	sqlite3_stmt* rawStmtRacun;
	string sqlRacun =
		"INSERT INTO Racuni(KupacID, BrojRacuna, DatumKupovine) VALUES(?, ?, ?);";
	int rc1 = sqlite3_prepare_v2(db, sqlRacun.c_str(), -1, &rawStmtRacun, nullptr);
	if (rc1 != SQLITE_OK) {
		cout << "GRESKA PRI PRIPREMI DODAVANJA RACUNA:" << sqlite3_errmsg(db) << endl;
		IzvrsiUpit("ROLLBACK;", "GRESKA PRI ROLLBACK-u");
		return;
	}
	StmtPtr stmtRacun(rawStmtRacun);
	sqlite3_bind_text(stmtRacun.get(), 1, to_string(KupacID).c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(stmtRacun.get(), 2, BrojRacun.c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(stmtRacun.get(), 3, DatumKupovine.c_str(), -1, SQLITE_TRANSIENT);
	rc1 = sqlite3_step(stmtRacun.get());
	if (rc1 != SQLITE_DONE) {
		cout << "GRESKA PRI DODAVANJU RACUNA:" << sqlite3_errmsg(db) << endl;
		IzvrsiUpit("ROLLBACK;", "GRESKA PRI ROLLBACK-u");
		return;
	}

	int RacunID = (int)sqlite3_last_insert_rowid(db);
	for (auto& x : StavkeRacuna) {
		string SerijskiBroj = x.first;
		double Kolicina = x.second;

		sqlite3_stmt* rawStmtArtikal;
		int ArtikalID = 0;
		string sqlArtikal =
			"SELECT ID FROM Artikli WHERE SerijskiBroj=?;";
		int rc2 = sqlite3_prepare_v2(db, sqlArtikal.c_str(), -1, &rawStmtArtikal, nullptr);
		if (rc2 != SQLITE_OK) {
			cout << "GRESKA PRI PRIPREMI PROVJERE ARTIKLA:" << sqlite3_errmsg(db) << endl;
			IzvrsiUpit("ROLLBACK;", "GRESKA PRI ROLLBACK-u");
			return;
		}
		StmtPtr stmtArtikal(rawStmtArtikal);
		sqlite3_bind_text(stmtArtikal.get(), 1, SerijskiBroj.c_str(), -1, SQLITE_TRANSIENT);
		if (sqlite3_step(stmtArtikal.get()) == SQLITE_ROW) {
			ArtikalID = sqlite3_column_int(stmtArtikal.get(), 0);
		}
		if (ArtikalID == 0) {
			cout << "GRESKA PRI DODAVANJU STAVKI RACUNA-NE POSTOJI TRAZENI ARTIKAL" << endl;
			IzvrsiUpit("ROLLBACK;", "GRESKA PRI ROLLBACK-u");
			return;
		}
		sqlite3_stmt* rawStmtStavke;
		string sqlStavke =
			"INSERT INTO Stavke(RacunID, ArtikalID, Kolicina) VALUES(?, ?, ?);";
		int rc3 = sqlite3_prepare_v2(db, sqlStavke.c_str(), -1, &rawStmtStavke, nullptr);
		if (rc3 != SQLITE_OK) {
			cout << "GRESKA PRI PRIPREMI DODAVANJA STAVKI RACUNA:" << sqlite3_errmsg(db) << endl;
			IzvrsiUpit("ROLLBACK;", "GRESKA PRI ROLLBACK-u");
			return;
		}
		StmtPtr stmtStavke(rawStmtStavke);
		sqlite3_bind_text(stmtStavke.get(), 1, to_string(RacunID).c_str(), -1, SQLITE_TRANSIENT);
		sqlite3_bind_text(stmtStavke.get(), 2, to_string(ArtikalID).c_str(), -1, SQLITE_TRANSIENT);
		sqlite3_bind_text(stmtStavke.get(), 3, to_string(Kolicina).c_str(), -1, SQLITE_TRANSIENT);
		rc3 = sqlite3_step(stmtStavke.get());
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
	sqlite3_stmt* rawStmt;
	string sql =
		"SELECT Kupci.ImePrezime, Racuni.BrojRacuna, Racuni.DatumKupovine, Artikli.Naziv, Artikli.SerijskiBroj, Artikli.Cijena, Stavke.Kolicina "
		"FROM Racuni "
		"JOIN Kupci ON Racuni.KupacID=Kupci.ID "
		"JOIN Stavke ON Racuni.ID=Stavke.RacunID "
		"JOIN Artikli ON Stavke.ArtikalID=Artikli.ID "
		"ORDER BY Racuni.ID;";
	int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &rawStmt, nullptr);
	if (rc != SQLITE_OK) {
		cout << "GRESKA PRI PRIPREMI PRIKAZA:" << sqlite3_errmsg(db) << endl;
		return;
	}
	StmtPtr stmt(rawStmt);
	string trenutniRacun = "";
	double UkupanRacun = 0;
	while (sqlite3_step(stmt.get())==SQLITE_ROW) {
		string ImePrezime = (const char*)sqlite3_column_text(stmt.get(), 0);
		string BrojRacuna = (const char*)sqlite3_column_text(stmt.get(), 1);
		string Datum = (const char*)sqlite3_column_text(stmt.get(), 2);
		string Naziv = (const char*)sqlite3_column_text(stmt.get(), 3);
		string SerijskiBroj = (const char*)sqlite3_column_text(stmt.get(), 4);
		double Cijena = sqlite3_column_double(stmt.get(), 5);
		double Kolicina = sqlite3_column_double(stmt.get(), 6);

		
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
}
int Baza::BrojRacuna() {
	sqlite3_stmt* rawStmt;
	string sql =
		"SELECT COUNT (*) FROM Racuni;";
	int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &rawStmt, nullptr);
	if (rc != SQLITE_OK) {
		cout << "GRESKA PRI BROJANJU RACUNA:" << sqlite3_errmsg(db) << endl;
		return -1;
	}
	StmtPtr stmt(rawStmt);
	int count = 0;
	if (sqlite3_step(stmt.get()) == SQLITE_ROW) {
		count = sqlite3_column_int(stmt.get(), 0);
	}
	return count;
}
int Baza::BrojStavkiRacuna() {
	sqlite3_stmt*rawStmt;
	string sql =
		"SELECT COUNT (*) FROM Stavke;";
	int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &rawStmt, nullptr);
	if (rc != SQLITE_OK) {
		cout << "GRESKA PRI BROJANJU STAVKI RACUNA:" << sqlite3_errmsg(db) << endl;
		return -1;
	}
	StmtPtr stmt(rawStmt);
	int count = 0;
	if (sqlite3_step(stmt.get()) == SQLITE_ROW) {
		count = sqlite3_column_int(stmt.get(), 0);
	}
	return count;
}
void Baza::PrikaziKupcaSaNajvecimTroskom() {
	sqlite3_stmt* rawStmt;
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
	int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &rawStmt, nullptr);
	if (rc != SQLITE_OK) {
		cout << "GRESKA PRI PRIPREMI PRIKAZA:" << sqlite3_errmsg(db) << endl;
		return;
	}
	StmtPtr stmt(rawStmt);
	if (sqlite3_step(stmt.get()) == SQLITE_ROW) {
		string ImePrezime = (const char*)sqlite3_column_text(stmt.get(), 0);
		string BrojRacuna = (const char*)sqlite3_column_text(stmt.get(), 1);
		string Datum = (const char*)sqlite3_column_text(stmt.get(), 2);
		double UkupanIznos = sqlite3_column_double(stmt.get(), 3);

		cout << "Ime i Prezime Kupca:" << ImePrezime << endl;
		cout << "Broj Racuna:" << BrojRacuna << endl;
		cout << "Datum Kreiranja Racuna:" << Datum << endl;
		cout << "Ukupan Iznos Racuna:" << UkupanIznos << endl;
	}
	else {
		cout << "NEMA PODATAKA" << endl;
	}
}
vector<Artikal>Baza::UzmiSveArtikle() {
	vector<Artikal>rezultat;
	sqlite3_stmt* rawStmt;
	string sql =
		"SELECT * FROM Artikli;";
	int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &rawStmt, nullptr);
	if (rc != SQLITE_OK) {
		cout << "GRESKA PRI PRIPREMI UZIMANJA ARTIKALA:" << sqlite3_errmsg(db) << endl;
		return rezultat;
	}
	StmtPtr stmt(rawStmt);
	while (sqlite3_step(stmt.get()) == SQLITE_ROW) {
		Artikal a;
		a.ID = sqlite3_column_int(stmt.get(), 0);
		a.Naziv = (const char*)sqlite3_column_text(stmt.get(), 1);
		a.SerijskiBroj = (const char*)sqlite3_column_text(stmt.get(), 2);
		a.Cijena = sqlite3_column_double(stmt.get(), 3);
		rezultat.push_back(a);
	}
	return rezultat;
}
vector<Kupac>Baza::UzmiSveKupce() {
	vector<Kupac>rezultat;
	sqlite3_stmt* rawStmt;
	string sql =
		"SELECT * FROM Kupci;";
	int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &rawStmt, nullptr);
	if (rc != SQLITE_OK) {
		cout << "GRESKA PRI PRIPREMI UZIMANJA KUPACA:" << sqlite3_errmsg(db) << endl;
		return rezultat;
	}
	StmtPtr stmt(rawStmt);
	while (sqlite3_step(stmt.get()) == SQLITE_ROW) {
		Kupac k;
		k.ID = sqlite3_column_int(stmt.get(), 0);
		k.ImePrezime = (const char*)sqlite3_column_text(stmt.get(), 1);
		k.JMBG = (const char*)sqlite3_column_text(stmt.get(), 2);
		rezultat.push_back(k);
	}
	return rezultat;
}
vector<StavkaRacuna>Baza::UzmiSveStavke() {
	vector<StavkaRacuna>rezultat;
	sqlite3_stmt* rawStmt;
	string sql =
		"SELECT Kupci.ImePrezime, Racuni.BrojRacuna, Racuni.DatumKupovine, Artikli.Naziv, Artikli.SerijskiBroj, Artikli.Cijena, Stavke.Kolicina "
		"FROM Racuni "
		"JOIN Kupci ON Racuni.KupacID=Kupci.ID "
		"JOIN Stavke ON Racuni.ID=Stavke.RacunID "
		"JOIN Artikli ON Stavke.ArtikalID=Artikli.ID "
		"ORDER BY Racuni.ID;";
	int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &rawStmt, nullptr);
	if (rc != SQLITE_OK) {
		cout << "GRESKA PRI PRIPREMI UZIMANJA STAVKI:" << sqlite3_errmsg(db) << endl;
		return rezultat;
	}
	StmtPtr stmt(rawStmt);
	while (sqlite3_step(stmt.get()) == SQLITE_ROW) {
		StavkaRacuna s;
		s.ImePrezime = (const char*)sqlite3_column_text(stmt.get(), 0);
		s.BrojRacuna = (const char*)sqlite3_column_text(stmt.get(), 1);
		s.DatumKupovine=(const char*)sqlite3_column_text(stmt.get(), 2);
		s.NazivArtikla = (const char*)sqlite3_column_text(stmt.get(), 3);
		s.SerijskiBroj = (const char*)sqlite3_column_text(stmt.get(), 4);
		s.Cijena = sqlite3_column_double(stmt.get(), 5);
		s.Kolicina = sqlite3_column_double(stmt.get(), 6);
		rezultat.push_back(s);
	}
	return rezultat;
}
NajveciRacun Baza::UzmiNajveciRacun() {
	NajveciRacun rezultat;
	rezultat.UkupanIznosRacuna = 0;
	sqlite3_stmt* rawStmt;
	string sql=
		"SELECT Kupci.ImePrezime, Racuni.BrojRacuna, Racuni.DatumKupovine, "
		"SUM(Artikli.Cijena * Stavke.Kolicina) AS UkupanRacun "
		"FROM Racuni "
		"JOIN Kupci ON Racuni.KupacID=Kupci.ID "
		"JOIN Stavke ON Racuni.ID=Stavke.RacunID "
		"JOIN Artikli ON Stavke.ArtikalID=Artikli.ID "
		"GROUP BY Racuni.ID "
		"ORDER BY UkupanRacun DESC "
		"LIMIT 1;";
	int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &rawStmt, nullptr);
	if (rc != SQLITE_OK) {
		cout << "GRESKA PRI PRIPREMI UZIMANJA NJAVECEG RACUNA:" << sqlite3_errmsg(db) << endl;
		return rezultat;
	}
	StmtPtr stmt(rawStmt);
	if (sqlite3_step(stmt.get()) == SQLITE_ROW){
		rezultat.ImePrezimeKupca = (const char*)sqlite3_column_text(stmt.get(), 0);
		rezultat.BrojRacuna = (const char*)sqlite3_column_text(stmt.get(), 1);
		rezultat.DatumKupovine = (const char*)sqlite3_column_text(stmt.get(), 2);
		rezultat.UkupanIznosRacuna = sqlite3_column_double(stmt.get(), 3);
	}
	return rezultat;
}
void Baza::DodajKorisnika(string UserName, string PassWordHash) {
	sqlite3_stmt* rawStmt;
	string sql =
		"INSERT INTO Korisnici(UserName, PassWordHash) VALUES(?, ?);";
	if (sqlite3_prepare_v2(db, sql.c_str(), -1, &rawStmt, nullptr) != SQLITE_OK) {
		throw std::runtime_error(sqlite3_errmsg(db));
	}
	StmtPtr stmt(rawStmt);
	sqlite3_bind_text(stmt.get(), 1, UserName.c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(stmt.get(), 2, PassWordHash.c_str(), -1, SQLITE_TRANSIENT);
	if (sqlite3_step(stmt.get()) != SQLITE_DONE) {
		throw std::runtime_error(sqlite3_errmsg(db));
	}
	cout << "KORISNIK USPIJESNO DODAN" << endl;
}
string Baza::UzmiHash(string UserName) {
	sqlite3_stmt* rawStmt;
	string PassWord = "";
	string sql =
		"SELECT PassWordHash FROM Korisnici WHERE UserName=?;";
	if (sqlite3_prepare_v2(db, sql.c_str(), -1, &rawStmt, nullptr) != SQLITE_OK) {
		cout << "GRESKA PRI PRIPREMI UZIMANJA PASSWORD-a:" << sqlite3_errmsg(db) << endl;
		return PassWord;
	}
	StmtPtr stmt(rawStmt);
	sqlite3_bind_text(stmt.get(), 1, UserName.c_str(), -1, SQLITE_TRANSIENT);
	if (sqlite3_step(stmt.get()) == SQLITE_ROW) {
		PassWord = (const char*)sqlite3_column_text(stmt.get(), 0);
	}
	return PassWord;
}
void Baza::DodijeliUlogu(string Username, string Uloga) {
	sqlite3_stmt* rawStmt;
	string sql =
		"UPDATE Korisnici SET Uloga=? WHERE UserName=?;";
	if (sqlite3_prepare_v2(db, sql.c_str(), -1, &rawStmt, nullptr)!=SQLITE_OK) {
		cout << "GRESKA PRI PRIPREMI DODJELJIVANJA ULOGE:" << sqlite3_errmsg(db) << endl;
		return;
	}
	StmtPtr stmt(rawStmt);
	sqlite3_bind_text(stmt.get(), 1, Uloga.c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(stmt.get(), 2, Username.c_str(), -1, SQLITE_TRANSIENT);
	if (sqlite3_step(stmt.get()) != SQLITE_DONE) {
		cout << "GRESKA PRI DODJELJIVANJU ULOGE:" << sqlite3_errmsg(db) << endl;
		return;
	}
	cout << "ULOGA USPIJESNO POSTAVLJENA" << endl;
}
string Baza::UzmiUlogu(string UserName) {
	sqlite3_stmt* rawStmt;
	string Uloga = "";
	string sql =
		"SELECT Uloga FROM Korisnici WHERE UserName=?;";
	if (sqlite3_prepare_v2(db, sql.c_str(), -1, &rawStmt, nullptr) != SQLITE_OK) {
		cout << "GRESKA PRI PRIPREMI UZIMANJA ULOGE:" << sqlite3_errmsg(db) << endl;
		return Uloga;
	}
	StmtPtr stmt(rawStmt);
	sqlite3_bind_text(stmt.get(), 1, UserName.c_str(), -1, SQLITE_TRANSIENT);
	if (sqlite3_step(stmt.get()) == SQLITE_ROW) {
		Uloga = (const char*)sqlite3_column_text(stmt.get(), 0);
	}
	return Uloga;
}