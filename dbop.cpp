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

void insertBlob(sqlite3* db, std::string fname, std::string tname, std::string bname, std::string pkey){
    std::cout << "inserting " << bname << " to " << tname << '\n';
    sqlite3_stmt* stmt;
    //std::string sql = "UPDATE " + tname " SET "
    std::string sql = "INSERT INTO " + tname + "('" + pkey + "', DATA) VALUES(?,?)";
    int rc;

    std::ifstream data(fname, std::ios::in | std::ios::binary);
    if(!data) std::cout << "failed to open " + fname + "...\n";

//determines file size
    data.seekg(0, std::ifstream::end);
    int size_img = data.tellg();
    data.seekg(0);

    char* buffer = new char[size_img];
    data.read(buffer, size_img);

    rc = sqlite3_prepare(db, sql.c_str(), -1, &stmt, 0);
    if(rc != SQLITE_OK){
        std::cout << "unable to prepare " << tname << " statement: " << sqlite3_errmsg(db) << '\n';
    }

    sqlite3_bind_text(stmt, 1, bname.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_blob(stmt, 2, buffer, size_img, SQLITE_STATIC);

    rc = sqlite3_step(stmt);

    if(rc != SQLITE_DONE){
        std::cout << "execution of " << bname << " failed: " << sqlite3_errmsg(db) << '\n';
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

            //first we extract the name of the table

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

            entries = convertBlock(lines[0]);

            std::string sql = "CREATE TABLE " + table_name + "(";

            for(unsigned int e = 0; e < entries.size(); ++e){
                std::string diff = entries[e].first + " " + entries[e].second;
                sql += diff;
                if(e < entries.size() - 1) sql += ",";
                else sql += ")";

                std::cout << "\t\t" << diff << "\n";
            }

            rc = sqlite3_exec(db, sql.c_str(), 0, 0, 0);
            if(rc != SQLITE_OK){
                std::cout << "FAILED TO CREATE TABLE " << table_name << " WITH ERROR CODE " << rc;
                continue;
            }

            //the table has been created
                //we are now ready to begin iterating through and adding the actual entries
            for(unsigned int block = 1; block < lines.size(); ++block){
                std::cout << "\tcreating " << lines[block][0] << '\n';
                entries.clear();

                entries = convertBlock(lines[block]);

                std::string insert = "INSERT INTO " + table_name + "(";
                std::string values = "VALUES(";


                for(unsigned int e = 0; e < entries.size(); ++e){

                    //the keys are added to insert
                    insert += wrap(entries[e].first);

                    //the values are added to... values
                    values += wrap(entries[e].second);

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
                    std::cout << "\n\nQUERY FAILED WITH ERROR CODE " << rc << "!\n\t" << sql << '\n';
                    continue;
                }

            } //end block loop
        } //end file opened successfully
    } //end filename loop
}

void addBinaryTables(sqlite3* db, std::vector<std::string> filenames){
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

            //first we extract the name of the table

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

            entries = convertBlock(lines[0]);

            std::string pkey;

            std::string sql = "CREATE TABLE " + table_name + "(";

            for(unsigned int e = 0; e < entries.size(); ++e){
                if(entries[e].second.find("PRIMARY KEY") != std::string::npos){
                    pkey = entries[e].first;
                }

                std::string diff = entries[e].first + " " + entries[e].second;
                sql += diff;
                if(e < entries.size() - 1) sql += ",";
                else sql += ")";

                std::cout << "\t\t" << diff << "\n";
            }

            rc = sqlite3_exec(db, sql.c_str(), 0, 0, 0);
            if(rc != SQLITE_OK){
                std::cout << "FAILED TO CREATE TABLE " << table_name << " WITH ERROR CODE " << rc;
                continue;
            }

            //the table has been created
                //we are now ready to begin iterating through and adding the actual entries
            for(unsigned int block = 1; block < lines.size(); ++block){
                std::cout << "\tcreating " << lines[block][0] << '\n';
                entries.clear();

                entries = convertBlock(lines[block]);

                std::string filename;
                std::string entry_name;

                //find the entry and file names
                for(const auto& e : entries){
                    if(e.first == pkey){
                        entry_name = e.second;
                    }
                    else if(e.first == "FILENAME"){
                        filename = e.second;
                    }
                }

                std::vector<std::string> update;

                for(unsigned int e = 0; e < entries.size(); ++e){
                    if(entries[e].first == "FILENAME"
                    || entries[e].first == pkey){
                        continue;
                    }

                    update.push_back(std::string("UPDATE " + table_name + " SET " + entries[e].first + " = " + wrap(entries[e].second) + " WHERE " + pkey + " = " + wrap(entry_name) + ";"));
                }

                std::cout << "\ninserting blob from binary/blob/" << filename << " to " << entry_name;

                insertBlob(db, "binary/blob/" + filename, table_name, entry_name, pkey);

                for(const auto& u : update){
                    rc = sqlite3_exec(db, u.c_str(), 0, 0, 0);
                    if(rc != SQLITE_OK){
                        std::cout << "\n\nQUERY FAILED WITH ERROR CODE " << rc << "!\n\t" << u << '\n';
                        continue;
                    }
                }

            } //end block loop
        } //end file opened successfully
    } //end filename loop
}
