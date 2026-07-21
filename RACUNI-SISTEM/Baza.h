#pragma once
#include<iostream>
#include<string>
#include<vector>
#include"sqlite3.h"
using namespace std;
class Baza {
	sqlite3* db;
public:
	Baza();
	~Baza();
	void IzvrsiUpit(string sql, string PORUKAGRESKE);
	void KreirajTabele();
	void KreirajIndex();
};