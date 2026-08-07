#include "pch.h"
#include "Baza.h"
#include <vector>

TEST(BazaTest, DodajArtikalProvjeriBrojArtikala) {
	Baza testBaza(":memory:");
	testBaza.KreirajTabele();
	testBaza.DodajArtikal("Artikal1", "123456", 10);
    EXPECT_EQ(testBaza.brojArtikala(),1);
}
TEST(BazaTest, DodajDupliArtikalProvjeriBrojArtikala) {
	Baza testBaza(":memory:");
	testBaza.KreirajTabele();
	testBaza.DodajArtikal("Artikal1", "123456", 10);
	EXPECT_EQ(testBaza.brojArtikala(), 1);
	testBaza.DodajArtikal("Artikal122", "123456", 20);
	EXPECT_EQ(testBaza.brojArtikala(), 1);
}
TEST(BazaTest, DodajKupcaProvjeriBrojKupaca) {
	Baza testBaza(":memory:");
	testBaza.KreirajTabele();
	testBaza.DodajKupca("Kupac1", "123456");
	EXPECT_EQ(testBaza.brojKupaca(), 1);
}
TEST(BazaTest, DodajDuplogKupcaProvjeriBrojKupaca) {
	Baza testBaza(":memory:");
	testBaza.KreirajTabele();
	testBaza.DodajKupca("Kupac1", "123456");
	EXPECT_EQ(testBaza.brojKupaca(), 1);
	testBaza.DodajKupca("Kupac2", "123456");
	EXPECT_EQ(testBaza.brojKupaca(), 1);
}
TEST(BazaTest, DodajRacunDodajStavkeRacunaProvjeriBrojRacunaBrojStavki) {
	Baza testBaza(":memory:");
	testBaza.KreirajTabele();
	testBaza.DodajArtikal("Artikal1", "123456", 10);
	testBaza.DodajArtikal("Artikal2", "654321", 20);
	testBaza.DodajKupca("Kupac1", "111222");
	vector<pair<string, double>>stavke = { {"123456",2},{"654321",1} };
	testBaza.DodajRacunIStavkeRacuna("111222", "BR.1", "2026-08-08", stavke);
	EXPECT_EQ(testBaza.BrojRacuna(), 1);
	EXPECT_EQ(testBaza.BrojStavkiRacuna(), 2);
}
TEST(BazaTest, DodajRacunDodajStavkeRacunaNepostojeciKupacProvjeriBrojRacunaBrojStavki) {
	Baza testBaza(":memory:");
	testBaza.KreirajTabele();
	testBaza.DodajArtikal("Artikal1", "123456", 10);
	testBaza.DodajArtikal("Artikal2", "654321", 20);
	testBaza.DodajKupca("Kupac1", "111222");
	vector<pair<string, double>>stavke = { {"123456",2},{"654321",1} };
	testBaza.DodajRacunIStavkeRacuna("000000", "BR.1", "2026-08-08", stavke);
	EXPECT_EQ(testBaza.BrojRacuna(), 0);
	EXPECT_EQ(testBaza.BrojStavkiRacuna(), 0);
}
TEST(BazaTest, DodajRacunDodajStavkeRacunaNepostojeciArtikalProvjeriBrojRacunaBrojStavki) {
	Baza testBaza(":memory:");
	testBaza.KreirajTabele();
	testBaza.DodajArtikal("Artikal1", "123456", 10);
	testBaza.DodajArtikal("Artikal2", "654321", 20);
	testBaza.DodajKupca("Kupac1", "111222");
	vector<pair<string, double>>stavke = { {"000000",2},{"654321",1} };
	testBaza.DodajRacunIStavkeRacuna("111222", "BR.1", "2026-08-08", stavke);
	EXPECT_EQ(testBaza.BrojRacuna(), 0);
	EXPECT_EQ(testBaza.BrojStavkiRacuna(), 0);
}