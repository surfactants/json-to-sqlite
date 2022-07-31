# json-to-sqlite
Parses JSON files and stores the results in a SQLite database.

Written with the SQLite 3.38.0 amalgamation[(direct download)](https://www.sqlite.org/2022/sqlite-amalgamation-3380000.zip)  - though I do not see any reason why the newest version wouldn't work.

## to use

Simply add your JSON files to `data/` and record them in `manifest.txt`. Binary files go in `binary/` and are recorded in `manifest_binary.txt`

The example files are derived from a game, for which I made this program. If you're me, or you exist in an unknown future, it is accessible at [surfactants/asteroids](https://github.com/surfactants/asteroids)

## caveats

- Each JSON file must begin with a dummy entry, formatted as `"KEY":"TYPE"`
- Primary key must be identified in the type string (i.e. `"NAME" : "TEXT PRIMARY KEY NOT NULL"`)
- Binary JSON files only need one value per entry (`"NAME":"FILENAME"`),
  - **EXCEPT for the dummy entry**, which also requires `"DATA":"BLOB"`
- If the filenames from the binary files do not exist
- One table per json file
