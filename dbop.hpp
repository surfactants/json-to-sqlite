#include "sqlite/sqlite3.h"
#include <string>
#include <vector>

std::string wrap(std::string str);

void littleBobbyTables(sqlite3* db);

void addTables(sqlite3* db, std::vector<std::string> filenames); //adds text tables

std::vector<std::pair<std::string, std::string>> convertBlock(std::vector<std::string> block);
