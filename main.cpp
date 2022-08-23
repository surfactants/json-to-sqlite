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
#include <iostream>

int main(int argc, char* argv[]){
    //define names for the manifest file and the json subdirectory
    std::string manifest_name = "manifest.txt",
                     prefix = "data/",
                     database_name = "data.db";

    //parse args to change manifest_name and prefix
    for(int c = 1; c < argc; c++){
        std::string arg(argv[c]);

        if(arg.length() < 2) continue;

        if(arg[1] == 'm'){
            manifest_name = extractArg(arg);
        }
        else if(arg[1] == 'p'){
            prefix = extractArg(arg);
            if(prefix.back() != '/') prefix += '/';
        }
        else if(arg[1] == 'd'){
            database_name = extractArg(arg);
            if(database_name.find(".db") == std::string::npos) database_name += ".db";
        }
    }

    //initialize and open the database
    sqlite3* db;
    int rc = sqlite3_open(database_name.c_str(), &db);

    //if the database fails to open for whatever reason,
        //attempt to close it and return error code 1
    if(rc != SQLITE_OK){
        std::cout << "failed to open database, error " << sqlite3_errmsg(db) << '\n';
        sqlite3_close(db);
        return 1;
    }

    std::cout << database_name << " successfully opened!\n";

    //extract the filenames from manifest.txt

    std::cout << "\nclearing database...\n";

    littleBobbyTables(db);

    std::vector<std::string> filenames = extractFilenames(manifest_name, prefix);

    if(filenames.size() > 0){
        std::cout << "\nadding tables...\n";

        addTables(db, filenames);

        std::cout << "\ntables added!\n";
    }
    else{
        std::cout << "\nno filenames were found! double check manifest.txt in the working directory!";
    }

    return 0;
}

