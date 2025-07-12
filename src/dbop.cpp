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

#include <dbop.hpp>

#include <experimental/filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <utility>
#include <iostream>

namespace filesystem = std::experimental::filesystem::v1;

std::function<void(Error)> Export::getError;

void err(const std::string& table, const std::string& operation, const std::string& reason)
{

    Export::getError({ table, operation, reason });
}

std::string extractArg(std::string arg)
{
    std::cout << "\nparsing arg: " << arg << '\n';
    arg = arg.substr(arg.find(":") + 1);
    std::cout << "\nparsed arg: " << arg << '\n';
    return arg;
}

std::vector<std::string> extractFilenames(std::string prefix)
{
    std::vector<std::string> filenames;

    std::string path = filesystem::current_path().string() + "/" + prefix;

    for (const auto& entry : filesystem::directory_iterator(path)) {
        auto path = entry.path();
        if (path.extension() == ".json") {
            filenames.push_back(path.string());
        }
    }

    return filenames;
}

Blocks getBlocks(std::string filename, std::string& table_name)
{
    Blocks blocks;

    std::ifstream file;
    file.open(filename, std::ios::in);
    if(!file.is_open()) {
        std::cout << "\nfailed to open " << filename << "!\n";
        return blocks;
    }

    std::cout << "opened " << filename << "\n";
    std::string line;
    std::getline(file, line); //opening brace

//extract table name
    std::getline(file, line);
    table_name = line.substr(line.find('"') + 1);
    table_name = table_name.substr(0, table_name.find('"'));

    std::cout << "creating table " << table_name << ", with fields: \n";

//read the blocks. each represents one table.
    while(file.good()) {
        std::getline(file, line);
        if(line.find(']') != std::string::npos) {
            break;
        }
        else if(line.find('{') != std::string::npos) {
            blocks.push_back(std::vector<std::string>());
        }
        else if(line.find('}') == std::string::npos) {
            blocks.back().push_back(line.substr(line.find('"')));
                //the find operation truncates the prepending whitespace
        }
    }

    file.close();

    std::cout << "\tblocks read!\n\n";

    return blocks;
}

Entry_Data convertBlock(std::vector<std::string> block)
{
    Entry_Data entries;

//define lambdas for extraction operations
    auto trunc = [] (std::string& s) {
        s = s.substr(s.find('"') + 1);
    };

    auto extract = [] (std::string& s) {
        return s.substr(0, s.find('"'));
    };


//split each line into key/value pairs and add them to the vector
    for(auto line : block) {
        std::string key,
                    val;

    //remove the first quote and extract the key
        trunc(line);
        key = extract(line);

    //remove the key and extract the value
        trunc(line);
        trunc(line);
        val = extract(line);

        entries.push_back(std::make_pair(key, val));
    }

    return entries;
}

std::string wrap(std::string str)
{
    return std::string ("'" + str + "'");
}

void littleBobbyTables(sqlite3* db)
{
    int rc;
    rc = sqlite3_db_config(db, SQLITE_DBCONFIG_RESET_DATABASE, 1, 0);
    if(rc != SQLITE_OK) {
        err("null", "CONFIG STEP 1", std::string("ERROR CODE " + std::to_string(rc)));
        std::abort();
    }
    rc = sqlite3_exec(db, "VACUUM", 0, 0, 0);
    if(rc != SQLITE_OK) {
        err("null", "CONFIG STEP 2", std::string("ERROR CODE " + std::to_string(rc)));
        std::abort();
    }
    rc = sqlite3_db_config(db, SQLITE_DBCONFIG_RESET_DATABASE, 0, 0);
    if(rc != SQLITE_OK) {
        err("null", "CONFIG STEP 3", std::string("ERROR CODE " + std::to_string(rc)));
        std::abort();
    }
}

void insertBlob(sqlite3* db, std::string filename, std::string table_name, std::string entry_name, std::string primary_key, std::string column_name)
{
    std::cout << "\t\tinserting " << table_name << "::" << column_name << " for entry " << primary_key
              << "\n\t\t\t(" << filename << ")\n";

    sqlite3_stmt* stmt;

    std::string sql = "UPDATE " + table_name + " SET " + column_name + " = ? WHERE " + primary_key + " = '" + entry_name + "'";

    int rc;

    std::ifstream data(filename, std::ios::in | std::ios::binary);
    if(!data) {
        std::cout << "\t\t\tfailed to open " + filename + "...\n";
        return;
    }

//get file size
    data.seekg(0, std::ifstream::end);
    int size_img = data.tellg();
    data.seekg(0);

    char* buffer = new char[size_img];
    data.read(buffer, size_img);

    data.close();

    rc = sqlite3_prepare(db, sql.c_str(), -1, &stmt, 0);
    if(rc != SQLITE_OK) {
        err(table_name, "PREPARING TABLE FOR BLOB INSERTION:\n" + sql, sqlite3_errmsg(db));
    }

    rc = sqlite3_bind_blob(stmt, 1, buffer, size_img, SQLITE_STATIC);
    if(rc != SQLITE_OK) {
        err(table_name, "BINDING BLOB FOR INSERTION:\n" + sql, sqlite3_errmsg(db));
    }

    rc = sqlite3_step(stmt);

    if(rc != SQLITE_DONE) {
        err(table_name, "INSERTING BLOB:\n" + sql, sqlite3_errmsg(db));
    }

    sqlite3_finalize(stmt);

    delete[] buffer;

    std::cout << "\t\tblob successfully inserted!\n\n";
}

void addTables(sqlite3* db, std::vector<std::string> filenames) {
    for(const auto& filename : filenames ) {
        std::cout << '\n';
        int rc;

        std::string table_name;

        Blocks blocks = getBlocks(filename, table_name);

    //if the blocks are empty, warn the user and move to the next table
        if(blocks.size() == 0) {
            std::cout << "\nno data was found in " << filename << "! skipping...";
            continue;
        }

//create the table
    //convert the first block to a set of entry strings
        Entry_Data entries;
        entries = convertBlock(blocks[0]);

    //set up an additional vector for blob keys so they may be properly inserted
        std::vector<std::string> blob_keys;

    //create the initial sql query
        std::string sql = "CREATE TABLE " + table_name + "(";

        //iterate through the entries to extract key/value pairs for the table
        for(unsigned int e = 0; e < entries.size(); ++e) {
            std::string diff = entries[e].first + " " + entries[e].second;
            sql += diff;
            if(e < entries.size() - 1) sql += ",";
            else sql += ")";

            if(entries[e].second == "BLOB") {
                blob_keys.push_back(std::string(entries[e].first));
            }

            std::cout << "\t" << diff << "\n";
        }

        std::cout << '\n';

        rc = sqlite3_exec(db, sql.c_str(), 0, 0, 0);
        if(rc != SQLITE_OK) {
            err(table_name, "CREATE TABLE", std::string("ERROR CODE " + std::to_string(rc)));
            continue;
        }

//create the entries
        for(unsigned int block = 1; block < blocks.size(); ++block) {
            Entry_Data blobs;

            std::cout << "\tcreating entry " << blocks[block][0] << '\n';
            entries.clear();

            entries = convertBlock(blocks[block]);

            std::string insert = "INSERT INTO " + table_name + "(";
            std::string values = "VALUES(";


            for(unsigned int e = 0; e < entries.size(); ++e) {

            //add key
                insert += wrap(entries[e].first);

            //add value
                if(std::find(blob_keys.begin(), blob_keys.end(), entries[e].first) != blob_keys.end()) {
                    values += "NULL";
                    entries[e].second = "blob/" + entries[e].second;
                    blobs.push_back(entries[e]);
                }
                else values += wrap(entries[e].second);

            //add commas or terminator as needed
                if(e < entries.size() - 1) {
                    insert += ",";
                    values += ",";
                }
                else{
                    insert += ")";
                    values += ");";
                }
            }

        //combine insert and values for a full sql query
            sql = insert + values;

            rc = sqlite3_exec(db, sql.c_str(), 0, 0, 0);
            if(rc != SQLITE_OK) {
                err(table_name, sql, std::string("ERROR CODE " + std::to_string(rc)));
                continue;
            }

            for(const auto& blob : blobs) {
                insertBlob(db, blob.second, table_name, entries[0].second, entries[0].first, blob.first);
            }

        } //end block loop
    } //end filename loop
}
