#include "dbop.hpp"
#include <fstream>
#include <iostream>

int main(){
    //initialize and open the database
    sqlite3* db;
    int rc = sqlite3_open("data.db", &db);

    //if the database fails to open for whatever reason,
        //attempt to close it and return error code 2
    if(rc){
        std::cout << "failed to open database, error " << sqlite3_errmsg(db) << '\n';
        sqlite3_close(db);
        return 1;
    }

    std::cout << "data.db successfully opened!\n";

    //extract the filenames from manifest.txt


    std::ifstream manifest;
    manifest.open("manifest.txt", std::ios::in);

    //if the file fails to open, return error code 2
    if(!manifest.is_open()){
        std::cout << "failed to open manifest.txt!";
        return 2;
    }

    std::vector<std::string> filenames;

    std::cout << "extracting filenames from manifest file:\n";

    const std::string prefix = "data/";

    while(manifest.good()){
        std::string filename;
        std::getline(manifest, filename);
        filenames.push_back(prefix + filename);
        std::cout << '\t' << filenames.back() << '\n';
    }

    std::cout << '\n';

    littleBobbyTables(db);

    addTables(db, filenames);

    sqlite3_close(db);

    std::cout << "data.db closed!\n";

    return 0;
}

