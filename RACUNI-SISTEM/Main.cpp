#include<iostream>
#include<string>
#include<vector>
#include"Baza.h"
using namespace std;
int main() {
	Baza baza;
	baza.KreirajTabele();
	baza.KreirajIndex();
	int izbor;
	do {
		cout << "MENI" << endl;
		cout << "1.Dodaj Artikal" << endl;
		cout << "2.Prikazi Sve Artikle" << endl;
		cout << "3.Dodaj Kupca" << endl;
		cout << "4.Prikazi Sve Kupce" << endl;
		cout << "5.Kreiraj Racun" << endl;
		cout << "6.Prikazi Sve Racune" << endl;
		cout << "7.Prikazi Kupca Sa Najvecim Racunom" << endl;
		cout << "8.Izlaz" << endl;
		cin >> izbor;
		cin.ignore();
		switch (izbor) {
		case 1:
		{
			string Naziv;
			string SerijskiBroj;
			double Cijena;
			cout << "Unesite Naziv Artikla: ";
			getline(cin, Naziv);
			cout << "Unesite Serijski Broj: ";
			getline(cin, SerijskiBroj);
			cout << "Unesite Cijenu: ";
			cin >> Cijena;
			cin.ignore();
			baza.DodajArtikal(Naziv, SerijskiBroj, Cijena);
			break;
		}
		case 2:
		{
			baza.PrikaziSveArtikle();
			break;
		}
		case 3:
		{
			string ImePrezime;
			string JMBG;
			cout << "Unesite Ime I Prezime Kupca: ";
			getline(cin, ImePrezime);
			cout << "Unesite JMBG Kupca: ";
			getline(cin, JMBG);
			baza.DodajKupca(ImePrezime, JMBG);
			break;
		}
		case 4:
		{
			baza.PrikaziSveKupce();
			break;
		}
		case 5:
		{
			string JMBG;
			string BrojRacuna;
			int dan, mjesec, godina;
			vector<pair<string, double>>Stavke;
			cout << "Unesite JMBG Kupca: ";
			getline(cin, JMBG);
			cout << "Unesite Broj Racuna: ";
			getline(cin, BrojRacuna);
			cout << "Unesite Dan Kreiranja Racuna: ";
			cin >> dan;
			cout << "Unesite Mjesec Kreiranja Racuna: ";
			cin >> mjesec;
			cout << "Unesite Godinu Kreiranja Racuna: ";
			cin >> godina;
			cin.ignore();
			string DatumISO = to_string(godina) + "-" + (mjesec < 10 ? "0" : "") + to_string(mjesec) + "-" + (dan < 10 ? "0" : "") + to_string(dan);
			string Odgovor;
			do {
				string SerijskiBroj;
				double Kolicina;
				cout << "Unesite Serijski Broj Artikla: ";
				getline(cin, SerijskiBroj);
				cout << "Unesite Kolicinu: ";
				cin >> Kolicina;
				cin.ignore();
				Stavke.push_back({ SerijskiBroj, Kolicina });
				cout << "Zelite Li Dodati Jos Artikala(DA/NE): ";
				cin >> Odgovor;
				cin.ignore();
			} while (Odgovor == "DA" || Odgovor == "Da" || Odgovor == "da");
			baza.DodajRacunIStavkeRacuna(JMBG, BrojRacuna, DatumISO, Stavke);
			break;

		}
		case 6:
		{
			baza.PrikaziSveRacune();
			break;
		}
		case 7:
		{
			baza.PrikaziKupcaSaNajvecimTroskom();
			break;
		}
		case 8:
		{
			cout << "DOVIDJENJA" << endl;
			break;
		}
		default:
		{
			cout << "POGRESAN IZBOR!!!" << endl;
			break;
		}
		}
	} while (izbor != 8);
	
}