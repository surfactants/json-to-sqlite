////////////////////////////////////////////////////////////
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
// you must not claim that you wrote the original software.
// If you use this software in a product, an acknowledgment
// in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
// and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
////////////////////////////////////////////////////////////

#pragma once

#include "sqlite/sqlite3.h"
#include <string>
#include <vector>

std::vector<std::string> extractFilenames(std::string manifest_name, std::string prefix);

std::string wrap(std::string str);

void littleBobbyTables(sqlite3* db);

void insertBlob(sqlite3* db, std::string file_name, std::string table_name, std::string entry_name, std::string primary_key, std::string column_name);

void addTables(sqlite3* db, std::vector<std::string> file_names); //adds text tables

std::vector<std::pair<std::string, std::string>> convertBlock(std::vector<std::string> block);
