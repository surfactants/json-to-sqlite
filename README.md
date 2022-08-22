# json-to-sqlite
Parses JSON files and stores the results in a SQLite database.

Written with the SQLite 3.38.0 amalgamation [(direct download)](https://www.sqlite.org/2022/sqlite-amalgamation-3380000.zip)  - though I do not see any reason why the newest version wouldn't work.

## to use

Simply add your JSON files to `data/` and record them in `manifest.txt`.

## notes

- Each table must be defined in its own JSON file.
- Each JSON file must begin with a dummy entry, formatted as `"KEY":"TYPE"` (i.e. `"COUNT":"INT"`)
- Primary key must be identified in the type string (i.e. `"NAME" : "TEXT PRIMARY KEY NOT NULL"`)
- BLOB fields require a filename, including any subdirectories, for the value
	- *the reffered file must exist in `blob/` or the program will undergo rapid unplanned disassembly!*
