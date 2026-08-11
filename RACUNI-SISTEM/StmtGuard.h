#pragma once
#include "sqlite3.h"

class StmtGuard {
private:
	sqlite3_stmt* stmt;
public:
	explicit StmtGuard(sqlite3_stmt* s) :stmt(s) {}
	~StmtGuard() {
		sqlite3_finalize(stmt);
	}
	StmtGuard(const StmtGuard&) = delete;
	StmtGuard& operator=(const StmtGuard&) = delete;
	sqlite3_stmt* get()const {
		return stmt;
	}
	operator sqlite3_stmt* ()const {
		return stmt;
	}
};