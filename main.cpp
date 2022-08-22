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

    std::cout << "\nclearing database...\n";

    littleBobbyTables(db);

    std::vector<std::string> filenames = extractFilenames("manifest.txt", "data/");
    if(filenames.size() > 0){
        std::cout << "\nadding tables...\n";

        addTables(db, filenames);

        std::cout << "\ntables added!\n";

        filenames.clear();
    }
    else{
        std::cout << "\nno filenames were found! double check manifest.txt in the working directory!";
    }

    return 0;
}

