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

using Blocks = std::vector<std::vector<std::string>>;
using Entry_Data = std::vector<std::pair<std::string, std::string>>;

////////////////////////////////////////////////////////////
/// \brief extracts arg parameter
///
/// \param \b arg full arg string
///
/// \return \b arg parsed arg
///
std::string extractArg(std::string arg);

////////////////////////////////////////////////////////////
/// \brief gets filenames from the manifest file
///
/// \param \b manifest_name filename of the manifest
/// \param \b prefix the directory containing the JSON files
///
/// \return \b filenames locations of JSON files to be reserialized in the database
///
std::vector<std::string> extractFilenames(std::string manifest_name, std::string prefix);

////////////////////////////////////////////////////////////
/// \brief converts JSON files to strings
///
/// \param \b filename target JSON file
/// \param \b table_name reference to the table name for consolidation of file I/O
///
/// \return \b blocks, with each vector representing one entry, and each subvector representing each line in that entry
///
Blocks getBlocks(std::string filename, std::string& table_name);

////////////////////////////////////////////////////////////
/// \brief splits a block's lines into their key/value pairs
///
/// \param \b block the entry block to be split
///
/// \return \b entries the set of key/value pairs defining an entry
///
Entry_Data convertBlock(std::vector<std::string> block);

////////////////////////////////////////////////////////////
/// \brief wraps a string in single quotes for queries
///
/// \param \b str the string to be wrapped
///
/// \return \b str ... the wrapped string
///
std::string wrap(std::string str);

////////////////////////////////////////////////////////////
/// \brief entirely clears a database
///
/// \param \b db the database to be cleared
///
void littleBobbyTables(sqlite3* db);

////////////////////////////////////////////////////////////
/// \brief adds a Binary Large OBject to the database
///
/// \param \b filename the file to be converted to binary
/// \param \b table_name ... the name of the table
/// \param \b primary_key the table's primary key
/// \param \b entry_name the target primary key value
/// \param \b column_name the blob's target column
///
void insertBlob(sqlite3* db, std::string filename, std::string table_name, std::string primary_key, std::string entry_name, std::string column_name);

////////////////////////////////////////////////////////////
/// \brief adds the tables to the database
///
/// \param \b db sqlite database object
/// \param \b file_names JSON files holding the data to be reserialized
///
void addTables(sqlite3* db, std::vector<std::string> file_names); //adds text tables
