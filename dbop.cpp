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

#include "dbop.hpp"
#include <fstream>
#include <string>
#include <vector>
#include <utility>
#include <iostream>

std::string extractArg(std::string arg){
    std::cout << "\nparsing arg: " << arg << '\n';
    arg = arg.substr(arg.find(":") + 1);
    std::cout << "\nparsed arg: " << arg << '\n';
    return arg;
}

std::vector<std::string> extractFilenames(std::string manifest_name, std::string prefix){
    std::vector<std::string> filenames;

    std::ifstream manifest;
    manifest.open(manifest_name, std::ios::in);

    //if the file fails to open, return empty vector
    if(!manifest.is_open()){
        std::cout << "failed to open " << manifest_name << "!\n";
        return std::vector<std::string>();
    }

    std::cout << "\nextracting filenames from " << prefix << manifest_name << ":\n";

    while(manifest.good()){
        std::string filename;
        std::getline(manifest, filename);
        if(filename != ""){
            filenames.push_back(prefix + filename);
            std::cout << '\t' << filenames.back() << '\n';
        }
    }

    return filenames;
}

std::vector<std::pair<std::string, std::string>> convertBlock(std::vector<std::string> block){
    std::vector<std::pair<std::string, std::string>> entries;

    //define lambdas for extraction operations

    auto trunc = [] (std::string& s){
        s = s.substr(s.find('"') + 1);
    };

    auto extract = [] (std::string& s){
        return s.substr(0, s.find('"'));
    };


    //split each line into key/value pairs and add them to the vector

    for(auto line : block){
        std::string k,
                    v;

        trunc(line);

        k = extract(line);
        trunc(line);
        trunc(line);
        v = extract(line);

        entries.push_back(std::make_pair(k, v));
    }

    return entries;
}

std::string wrap(std::string str){
    return std::string ("'" + str + "'");
}

void littleBobbyTables(sqlite3* db){
    int rc;
    rc = sqlite3_db_config(db, SQLITE_DBCONFIG_RESET_DATABASE, 1, 0);
    if(rc != SQLITE_OK){
        std::cout << "config failed on step 1! aborting...";
        std::abort();
    }
    rc = sqlite3_exec(db, "VACUUM", 0, 0, 0);
    if(rc != SQLITE_OK){
        std::cout << "config failed on step 2! aborting...";
        std::abort();
    }
    rc = sqlite3_db_config(db, SQLITE_DBCONFIG_RESET_DATABASE, 0, 0);
    if(rc != SQLITE_OK){
        std::cout << "config failed on step 3! aborting...";
        std::abort();
    }
}

void insertBlob(sqlite3* db, std::string file_name, std::string table_name, std::string entry_name, std::string primary_key, std::string column_name){
    std::cout << "inserting " << entry_name << "::" << column_name << " to " << table_name << '\n';
    sqlite3_stmt* stmt;
    std::string sql = "UPDATE " + table_name + " SET " + column_name + " = ? WHERE " + primary_key + " = '" + entry_name + "'";
    int rc;

    std::ifstream data(file_name, std::ios::in | std::ios::binary);
    if(!data) std::cout << "failed to open " + file_name + "...\n";

//determines file size
    data.seekg(0, std::ifstream::end);
    int size_img = data.tellg();
    data.seekg(0);

    char* buffer = new char[size_img];
    data.read(buffer, size_img);

    data.close();

    rc = sqlite3_prepare(db, sql.c_str(), -1, &stmt, 0);
    if(rc != SQLITE_OK){
        std::cout << "unable to prepare " << table_name << " statement: " << sqlite3_errmsg(db) << "\n\t(" + sql + ")\n";
    }

    sqlite3_bind_blob(stmt, 1, buffer, size_img, SQLITE_STATIC);

    rc = sqlite3_step(stmt);

    if(rc != SQLITE_DONE){
        std::cout << "execution of " << entry_name << " failed: " << sqlite3_errmsg(db) << "\n\t(" + sql + ")\n";
    }

    sqlite3_finalize(stmt);

    delete[] buffer;
}

void addTables(sqlite3* db, std::vector<std::string> filenames){
    for(const auto& filename : filenames){
        std::cout << '\n';
        int rc;

        std::ifstream file;
        file.open(filename, std::ios::in);
        if(!file.is_open()){
            std::cout << "\nfailed to open " << filename << "!\n";
            continue;
        }
        else{ //file opened successfully
            std::cout << "opened " << filename << "\n";
            std::string line;
            std::getline(file, line); //opening brace

            //extract table name

            std::getline(file, line);
            std::string table_name = line.substr(line.find('"') + 1);
            table_name = table_name.substr(0, table_name.find('"'));

            std::cout << "creating table " << table_name << ", with fields: \n";

            //now read the lines into a vector of string vectors
                //the second dimension holds each distinct definition

            std::vector<std::vector<std::string>> lines;
            while(file.good()){
                std::getline(file, line);
                if(line.find(']') != std::string::npos){
                    break;
                }
                else if(line.find('{') != std::string::npos){
                    lines.push_back(std::vector<std::string>());
                }
                else if(line.find('}') == std::string::npos){
                    lines.back().push_back(line.substr(line.find('"')));
                        //the find operation truncates the prepending whitespace
                }
            }

            //now we create the table, using the first entry (which is formatted as "KEY":"TYPE")
                //make sure you have a primary key included in the type string!

            std::vector<std::pair<std::string, std::string>> entries;

            std::vector<std::string> blob_keys;

            entries = convertBlock(lines[0]);

            std::string sql = "CREATE TABLE " + table_name + "(";

            for(unsigned int e = 0; e < entries.size(); ++e){
                std::string diff = entries[e].first + " " + entries[e].second;
                sql += diff;
                if(e < entries.size() - 1) sql += ",";
                else sql += ")";

                if(entries[e].second == "BLOB"){
                    blob_keys.push_back(std::string(entries[e].first));
                }

                std::cout << "\t\t" << diff << "\n";
            }

            rc = sqlite3_exec(db, sql.c_str(), 0, 0, 0);
            if(rc != SQLITE_OK){
                std::cout << "FAILED TO CREATE TABLE " << table_name << " WITH ERROR CODE " << rc << "\n\t" << sql;
                continue;
            }

            //the table has been created
                //we are now ready to begin iterating through and adding the actual entries
            for(unsigned int block = 1; block < lines.size(); ++block){
                std::vector<std::pair<std::string, std::string>> blobs;

                std::cout << "\tcreating " << lines[block][0] << '\n';
                entries.clear();

                entries = convertBlock(lines[block]);

                std::string insert = "INSERT INTO " + table_name + "(";
                std::string values = "VALUES(";


                for(unsigned int e = 0; e < entries.size(); ++e){


                    //the keys are added to insert
                    insert += wrap(entries[e].first);

                    //the values are added to... values
                    if(std::find(blob_keys.begin(), blob_keys.end(), entries[e].first) != blob_keys.end()){
                        values += "NULL";
                        entries[e].second = "blob/" + entries[e].second;
                        blobs.push_back(entries[e]);
                    }
                    else values += wrap(entries[e].second);

                    //commas or terminators are added as needed
                    if(e < entries.size() - 1){
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
                if(rc != SQLITE_OK){
                    std::cout << "\n\nQUERY FAILED WITH ERROR CODE " << rc << ":\n\t" << sql << '\n';
                    continue;
                }

                for(const auto& blob : blobs){
                    insertBlob(db, blob.second, table_name, entries[0].first, entries[0].second, blob.first);
                }

            } //end block loop
        } //end file opened successfully
    } //end filename loop
}
