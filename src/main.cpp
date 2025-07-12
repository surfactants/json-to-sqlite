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
#include <fstream>
#include <iostream>

void outputErrors(const std::vector<Error>& errors)
{
    const size_t size = errors.size();
    std::cout << "\n" << size << " errors were encountered";
    switch (size) {
        case 0:
            std::cout << "!";
            break;
        default:
            std::cout << ":";
    }

    for (const auto& error : errors) {
        std::cout << "\n" << error.table;
        std::cout << "\n\t" << error.operation;
        std::cout << "\n\t" << error.reason;
    }
}

int main(int argc, char* argv[])
{
    // assign error tracking callback
    std::vector<Error> errors;
    Export::getError = [&](Error e) {
        errors.push_back(e);
    };

    std::string prefix = "data/";
    std::string database_name = "data.db";

    // parse args
    for(int c = 1; c < argc; c++) {
        std::string arg(argv[c]);

        if(arg.length() < 2) continue;

        if(arg[1] == 'p') { // prefix
            prefix = extractArg(arg);
            if(prefix.back() != '/') prefix += '/';
        }
        else if(arg[1] == 'd') {
            database_name = extractArg(arg); // database name
            if(database_name.find(".db") == std::string::npos) database_name += ".db";
        }
    }

    //initialize and open the database
    sqlite3* db;
    int rc = sqlite3_open(database_name.c_str(), &db);

    //if the database fails to open for whatever reason,
        //attempt to close it and return error code 1
    if(rc != SQLITE_OK) {
        std::cout << "failed to open database, error " << sqlite3_errmsg(db) << '\n';
        sqlite3_close(db);
        return 1;
    }
    std::cout << database_name << " successfully opened!\n";

    std::cout << "\nclearing database...\n";
    littleBobbyTables(db);

    std::vector<std::string> filenames = extractFilenames(prefix);

    if(filenames.size() > 0) {
        std::cout << "\nadding tables...\n";
        addTables(db, filenames);
        std::cout << "\ntables added!\n";
    }
    else{
        std::cout << "\nno filenames were found! double check that your subdirectory's prefix is correct, and that it contains files!";
    }

    outputErrors(errors);

    std::cout << "\n\nEnter any character to continue...\n";
    char in;
    std::cin >> in;

    return 0;
}

