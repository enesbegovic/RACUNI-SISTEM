#include <mutex>
#include <chrono>
#include <map>
#include <sodium.h>
#include<httplib.h>
#include<nlohmann/json.hpp>
#include "Baza.h"
using json = nlohmann::json;
int main() {
	if (sodium_init() < 0) {
		std::cout << "Libsodium nije inicijalizovan" << std::endl;
		return 1;
	}
	Baza baza("RACUNI-SISTEM.db");
	struct Sesija {
		string UserName;
		string Uloga;
		std::chrono::steady_clock::time_point vrijeme;
	};
	std::map<string, Sesija>sesije;
	std::mutex sesijeMutex;
	baza.KreirajTabele();
	baza.KreirajIndex();
	httplib::Server server;
	server.set_pre_routing_handler([&sesije, &sesijeMutex](const httplib::Request& req, httplib::Response& res) {
		if (req.path == "/" || req.path == "/register" || req.path == "/login") {
			return httplib::Server::HandlerResponse::Unhandled;
		}
		string auth = req.get_header_value("Authorization");
		string Prefiks = "Bearer ";
		if (auth.rfind(Prefiks, 0) == 0) {
			string token = auth.substr(Prefiks.size());
			std::lock_guard<mutex>lock(sesijeMutex);
			auto it = sesije.find(token);
			if (it !=sesije.end() && it->second.vrijeme > std::chrono::steady_clock::now()) {
				bool Mijenja = (req.method == "POST" || req.method == "PUT" || req.method == "DELETE")
					&& req.path != "/logout";
				if (Mijenja && it->second.Uloga != "admin") {
					res.status = 403;
					res.set_content("{\"Greska\":\"Nemate dozvolu za ovu akciju\"}", "application/json");
					return httplib::Server::HandlerResponse::Handled;
				}
				return httplib::Server::HandlerResponse::Unhandled;
			}
		}
		res.status = 401;
		res.set_content("{\"Greska\":\"Nevazeci ili istekao token\"}", "application/json");
		return httplib::Server::HandlerResponse::Handled;
		});
	server.Get("/", [](const httplib::Request& req, httplib::Response& res) {
		res.set_content("Server radi!", "text/plain");
		});
	server.Post("/register", [&baza](const httplib::Request& req, httplib::Response& res) {
		try {
			json tijelo = json::parse(req.body);
			string UserName = tijelo.at("username");
			string PassWord = tijelo.at("password");
			char hash[crypto_pwhash_STRBYTES];
			if (crypto_pwhash_str(hash, PassWord.c_str(), PassWord.size(), crypto_pwhash_OPSLIMIT_INTERACTIVE, crypto_pwhash_MEMLIMIT_INTERACTIVE) != 0) {
				res.status = 500;
				res.set_content("{\"Greska\":\"Hashiranje nije uspijelo\"}", "application/json");
				return;
			}
			baza.DodajKorisnika(UserName, hash);
			json Poruka;
			Poruka["Poruka"] = "Korsinik uspijesno registrovan";
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
	server.Post("/login", [&baza, &sesije, &sesijeMutex](const httplib::Request& req, httplib::Response& res) {
		try {
			json tijelo = json::parse(req.body);
			string UserName = tijelo.at("username");
			string PassWord = tijelo.at("password");
			string hash = baza.UzmiHash(UserName);
			if (hash.empty() || crypto_pwhash_str_verify(hash.c_str(), PassWord.c_str(), PassWord.size()) != 0) {
				res.status = 401;
				res.set_content("{\"Greska\":\"Pogresno korisnicko ime ili lozinka\"}", "application/json");
				return;
			}
			unsigned char bajtovi[32];
			randombytes_buf(bajtovi,sizeof bajtovi);
			char token[sodium_base64_ENCODED_LEN(sizeof bajtovi, sodium_base64_VARIANT_URLSAFE_NO_PADDING)];
			sodium_bin2base64(token, sizeof token, bajtovi, sizeof bajtovi, sodium_base64_VARIANT_URLSAFE_NO_PADDING);
			string Uloga = baza.UzmiUlogu(UserName);
			{
				std::lock_guard<mutex>lock(sesijeMutex);
				for (auto it = sesije.begin(); it != sesije.end();) {
					if (it->second.vrijeme < std::chrono::steady_clock::now()) {
						it = sesije.erase(it);
					}
					else {
						++it;
					}
				}
				sesije[token] = { UserName,Uloga,std::chrono::steady_clock::now() + std::chrono::hours(2) };
			}
			json Poruka;
			Poruka["Token"] = token;
			res.status = 200;
			res.set_content(Poruka.dump(4), "application/json");
		}
		catch (const std::exception& e) {
			json Greska;
			Greska["Greska"] = e.what();
			res.status = 400;
			res.set_content(Greska.dump(4), "application/json");
		}
		});
	server.Post("/logout", [&sesije, &sesijeMutex](const httplib::Request& req, httplib::Response& res) {
		string auth = req.get_header_value("Authorization");
		string Prefiks = "Bearer ";
		string token = auth.substr(Prefiks.size());
		{
			std::lock_guard<mutex>lock(sesijeMutex);
			sesije.erase(token);
		}
		res.status = 200;
		res.set_content("{\"Poruka\":\"Odjava uspijesna\"}", "application/json");
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
	server.listen("0.0.0.0", 8080);
	return 0;
}