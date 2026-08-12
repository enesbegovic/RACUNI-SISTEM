#pragma once
#include "sqlite3.h"
#include <memory>

struct StmtDeleter {
	void operator()(sqlite3_stmt* s)const{
		sqlite3_finalize(s);
	}
};
using StmtPtr = std::unique_ptr<sqlite3_stmt, StmtDeleter>;