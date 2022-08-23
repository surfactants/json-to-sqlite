# json-to-sqlite
Parses JSON files and stores the results in a SQLite database.

Written with the SQLite 3.38.0 amalgamation [(direct download)](https://www.sqlite.org/2022/sqlite-amalgamation-3380000.zip)  - though I do not see any reason why the newest version wouldn't work.

## to use

Simply add your JSON files to `data/`, and record them in `manifest.txt`. Binary files go in `blob/`.

Results will be saved to `data.db`

The manifest filename, JSON subdirectory, and output filename can all be specified with passed args:
- `-m:manifest.txt`
- `-p:json_directory/` (the trailing slash will be appended if missing)
- `-d:database_name.db` (the file extension will be appended if missing)

## notes

- Each table must be defined in its own JSON file.
- Each JSON file must begin with a dummy entry, formatted as `"KEY":"TYPE"` (i.e. `"COUNT":"INT"`)
- Primary key must be identified in the type string (i.e. `"NAME":"TEXT PRIMARY KEY NOT NULL"`)
- BLOB fields require a filename, including any subdirectories, for the value (i.e. `"DATA":"images/img.jpg"`)
	- *the referred file must exist in `blob/` or the program will undergo rapid unplanned disassembly!*

Here is the packed example data, seen in `DB Browser for SQLite`:

![a screenshot from DB Browser, showing a single entry in a BOOKS table which matches the data found in example.json](https://github.com/surfactants/json-to-sqlite/blob/main/database.png)
