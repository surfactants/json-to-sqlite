# json-to-sqlite
## About
Written by [surfactants](github.com/surfactants), with the SQLite 3.38.0 amalgamation [(direct download)](https://www.sqlite.org/2022/sqlite-amalgamation-3380000.zip)

Parses JSON files and stores the results in a SQLite database.

## Usage
- Simply add your JSON files to `data/`. You can store them in a separate subdirectory, but that title must be passed as an arg.
- Binary files go in `blob/`.

Results will be saved to `data.db`

The JSON subdirectory, and output filename can  be specified with passed args:
- `-p:json_directory/` (the trailing slash will be appended if missing)
- `-d:database_name.db` (the file extension will be appended if missing)

## Notes
- Each table must be defined in its own JSON file.
- Each JSON file must begin with a dummy entry, formatted as `"KEY":"TYPE"` (i.e. `"COUNT":"INT"`).
- Primary key must be identified in the type string (i.e. `"NAME":"TEXT PRIMARY KEY NOT NULL"`)
- BLOB fields require a filename, including any subdirectories, for the value (i.e. `"DATA":"images/img.jpg"`)
	- *the referred file must exist in `blob/` or the program will undergo rapid unplanned disassembly!*

Here is the packed example data, as seen in [DB Browser for SQLite](sqlitebrowser/sqlitebrowser):

![a screenshot from DB Browser, showing a single entry in a BOOKS table which matches the data found in example.json](https://github.com/surfactants/json-to-sqlite/blob/main/example.png)
