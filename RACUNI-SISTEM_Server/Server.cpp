#include<httplib.h>
#include<nlohmann/json.hpp>
#include "Baza.h"
using json = nlohmann::json;
int main() {
	Baza baza("C:\\Users\\enesb\\source\\repos\\RACUNI-SISTEM\\RACUNI-SISTEM\\RACUNI-SISTEM.db");
	baza.KreirajTabele();
	baza.KreirajIndex();
	httplib::Server server;
	server.Get("/", [](const httplib::Request& req, httplib::Response& res) {
		res.set_content("Server radi!", "text/plain");
		});
	server.Get("/artikli", [&baza](const httplib::Request& req, httplib::Response& res) {
		std::vector<Artikal>artikli = baza.UzmiSveArtikle();
		json rezultat = json::array();
		for (const auto& a : artikli) {
			json ja;
			ja["ID"] = a.ID;
			ja["Naziv"] = a.Naziv;
			ja["Serijski Broj"] = a.SerijskiBroj;
			ja["Cijena"] = a.Cijena;
			rezultat.push_back(ja);
		}
		res.set_content(rezultat.dump(4), "application/json");
		});
	server.Post("/artikli", [&baza](const httplib::Request& req, httplib::Response& res) {
		try {
			json tijelo = json::parse(req.body);
			string naziv = tijelo.at("naziv");
			string serijskibroj = tijelo.at("serijskibroj");
			double cijena = tijelo.at("cijena");
			baza.DodajArtikal(naziv, serijskibroj, cijena);
			json Poruka;
			Poruka["Poruka"] = "Artikal uspjesno dodan";
			res.status = 201;
			res.set_content(Poruka.dump(4), "application/json");
		}
		catch (const std::exception& e) {
			json Greska;
			Greska["Greska"] = e.what();
			res.status = 400;
			res.set_content(Greska.dump(4), "application/json");
		}
		});
	server.Get("/kupci", [&baza](const httplib::Request& req, httplib::Response& res) {
		std::vector<Kupac>kupci = baza.UzmiSveKupce();
		json rezultat = json::array();
		for (const auto& k : kupci) {
			json jk;
			jk["ID"] = k.ID;
			jk["Ime i Prezime"] = k.ImePrezime;
			jk["JMBG"] = k.JMBG;
			rezultat.push_back(jk);
		}
		res.set_content(rezultat.dump(4), "application/json");
		});
	server.Post("/kupci", [&baza](const httplib::Request& req, httplib::Response& res) {
		try {
			json Tijelo = json::parse(req.body);
			string ImePrezime = Tijelo.at("imeprezime");
			string JMBG = Tijelo.at("JMBG");
			baza.DodajKupca(ImePrezime, JMBG);
			json Poruka;
			Poruka["Poruka"] = "Kupac uspijesno dodan";
			res.status = 201;
			res.set_content(Poruka.dump(4), "application/json");
		}
		catch (const std::exception& e) {
			json Greska;
			Greska["Greska"] = e.what();
			res.status = 400;
			res.set_content(Greska.dump(4), "application/json");
		}
		});
	server.Get("/racuni", [&baza](const httplib::Request& req, httplib::Response& res) {
		std::vector<StavkaRacuna>stavke = baza.UzmiSveStavke();
		json rezultat = json::array();
		string trenutniRacun = "";
		json trenutniObjekat;
		for (const auto& s : stavke) {
			if (s.BrojRacuna != trenutniRacun) {
				if (!trenutniRacun.empty()) {
					rezultat.push_back(trenutniObjekat);
				}
				trenutniObjekat = json();
				trenutniObjekat["Ime i Prezime Kupca"] = s.ImePrezime;
				trenutniObjekat["Broj Racuna"] = s.BrojRacuna;
				trenutniObjekat["Datum Kupovine"] = s.DatumKupovine;
				trenutniObjekat["Stavke"] = json::array();
				trenutniObjekat["Ukupno"] = 0.0;
				trenutniRacun = s.BrojRacuna;
			}
			json stavka;
			stavka["Naziv Artikla"] = s.NazivArtikla;
			stavka["Serijski Broj Artikla"] = s.SerijskiBroj;
			stavka["Cijena"] = s.Cijena;
			stavka["Kolicina"] = s.Kolicina;
			trenutniObjekat["Stavke"].push_back(stavka);
			trenutniObjekat["Ukupno"] = trenutniObjekat["Ukupno"].get<double>() + s.Cijena * s.Kolicina;
		}
		if (!trenutniRacun.empty()) {
			rezultat.push_back(trenutniObjekat);
		}
		res.set_content(rezultat.dump(4), "application/json");
		});
	server.Post("/racuni", [&baza](const httplib::Request& req, httplib::Response& res) {
		try {
			json tijelo = json::parse(req.body);
			string KupacJMBG = tijelo.at("kupac");
			string BrojRacuna = tijelo.at("brojracuna");
			string datumKupovine = tijelo.at("datumkupovine");
			vector < pair<string, double>>stavke;
			for (auto& s : tijelo.at("stavke")) {
				string serijskibroj = s.at("serijskibroj").get<string>();
				double Kolicina = s.at("kolicina").get<double>();
				stavke.push_back({ serijskibroj,Kolicina });
			}
			baza.DodajRacunIStavkeRacuna(KupacJMBG, BrojRacuna, datumKupovine, stavke);
			json Poruka;
			Poruka["Poruka"] = "Racun uspijesno kreiran";
			res.status = 201;
			res.set_content(Poruka.dump(4), "application/json");
		}
		catch (const std::exception& e) {
			json Greska;
			Greska["Greska"] = e.what();
			res.status = 400;
			res.set_content(Greska.dump(4), "application/json");
		}
		});
	server.Get("/racuni/max", [&baza](const httplib::Request& req, httplib::Response& res) {
		NajveciRacun najveci = baza.UzmiNajveciRacun();
	    json rezultat;
	    rezultat["Ime i Prezime Kupca"] = najveci.ImePrezimeKupca;
		rezultat["Broj Racuna"] = najveci.BrojRacuna;
		rezultat["Datum Kupovine"] = najveci.DatumKupovine;
		rezultat["Ukupan Iznos Racuna"] = najveci.UkupanIznosRacuna;
		
		res.set_content(rezultat.dump(4), "application/json");
		});
	std::cout << "Server pokrenut na http:://localhost:8080" << std::endl;
	server.listen("localhost", 8080);
	return 0;
}