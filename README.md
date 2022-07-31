# json-to-sqlite
Parses JSON files and stores the results in a SQLite database.

Written with SQLite [3.38.0](https://github.com/sqlite/sqlite/releases/tag/version-3.38.0) amalgamation file. Instructions for building the file can be found [on the SQLite website](https://www.sqlite.org/draft/amalgamation.html)..

## to use

Simply add your JSON files to `data/` and record them in `manifest.txt`

The example files are derived from a game, for which I made this program. If you're me, or you exist in an unknown future, it is accessible at [surfactants/asteroids](https://github.com/surfactants/asteroids)

## caveats

- The JSON file must begin with a dummy entry, formatted as "KEY" : "TYPE"
- The primary key must be identified in the type string (i.e. "NAME" : "TEXT PRIMARY KEY NOT NULL")
- One table per json file
- Cannot handle binary data
