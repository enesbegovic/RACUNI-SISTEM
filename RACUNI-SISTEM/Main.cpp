#include<iostream>
#include<string>
#include<vector>
#include"Baza.h"
using namespace std;
int main() {
	Baza baza;
	baza.KreirajTabele();
	baza.KreirajIndex();
	cout << "GOTOVO" << endl;
}