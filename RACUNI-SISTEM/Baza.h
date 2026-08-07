#pragma once
#include<iostream>
#include<string>
#include<vector>
#include"sqlite3.h"
using namespace std;
class Baza {
	sqlite3* db;
public:
	Baza(string putanjaBaze="RACUNI-SISTEM.db");
	~Baza();
	void IzvrsiUpit(string sql, string PORUKAGRESKE);
	void KreirajTabele();
	void KreirajIndex();
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
};