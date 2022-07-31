# json-to-sqlite
Parses JSON files and stores the results in a SQLite database.

Written with the C amalgamation of SQLite 3.38.0

## to use

Simply add your JSON files to `data/` and record them in `manifest.txt`

The example files are derived from a game, for which I made this program.

## caveats

- The JSON file must begin with a dummy entry, formatted as "KEY" : "TYPE"
- The primary key must be identified in the type string (i.e. "NAME" : "TEXT PRIMARY KEY NOT NULL")
- One table per json file
- Cannot handle binary data
