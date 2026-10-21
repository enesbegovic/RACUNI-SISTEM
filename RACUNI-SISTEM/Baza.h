#pragma once
#include<iostream>
#include<string>
#include<vector>
#include"sqlite3.h"
#include "StmtPtr.h"
using namespace std;
struct Artikal {
	int ID;
	string Naziv;
	string SerijskiBroj;
	double Cijena;
};
struct Kupac {
	int ID;
	string ImePrezime;
	string JMBG;
};
struct StavkaRacuna {
	string ImePrezime;
	string BrojRacuna;
	string DatumKupovine;
	string NazivArtikla;
	string SerijskiBroj;
	double Cijena;
	double Kolicina;
};
struct NajveciRacun {
	string ImePrezimeKupca;
	string BrojRacuna;
	string DatumKupovine;
	double UkupanIznosRacuna;
};
class Baza {
	sqlite3* db;
public:
	Baza(string putanjaBaze="RACUNI-SISTEM.db");
	~Baza();
	void IzvrsiUpit(string sql, string PORUKAGRESKE);
	void KreirajTabele();
	void KreirajIndex();
	vector<Artikal>UzmiSveArtikle();
	vector<Kupac>UzmiSveKupce();
	vector<StavkaRacuna>UzmiSveStavke();
	NajveciRacun UzmiNajveciRacun();
	void DodajArtikal(string Naziv, string SerijskiBroj, double Cijena);
	void PrikaziSveArtikle();
	int brojArtikala();
	void DodajKupca(string ImePrezime, string JMBG);
	void PrikaziSveKupce();
	int brojKupaca();
	void DodajRacunIStavkeRacuna(string KupacJMBG, string BrojRacuna, string DatumKupovine, vector < pair < string, double >> StavkeRacuna);
	void PrikaziSveRacune();
	int BrojRacuna();
	int BrojStavkiRacuna();
	void PrikaziKupcaSaNajvecimTroskom();
	void DodajKorisnika(string UserName, string PassWordHash);
	string UzmiHash(string UserName);
	void DodijeliUlogu(string UserName, string Uloga);
	string UzmiUlogu(string UserName);
};