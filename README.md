# json-to-sqlite
Parses JSON files and stores the results in a SQLite database.

Written with the SQLite 3.38.0 amalgamation [(direct download)](https://www.sqlite.org/2022/sqlite-amalgamation-3380000.zip)  - though I do not see any reason why the newest version wouldn't work.

## to use

Simply add your JSON files to `data/` and record them in `manifest.txt`. Binary files go in 'blob/'.

The example files are derived from a game, for which I made this program. If you're me, or you exist in an unknown future, it is accessible at [surfactants/asteroids](https://github.com/surfactants/asteroids)

## notes

- Each table must be defined in its own JSON file.
- Each JSON file must begin with a dummy entry, formatted as `"KEY":"TYPE"` (i.e. `"COUNT":"INT"`)
- Primary key must be identified in the type string (i.e. `"NAME" : "TEXT PRIMARY KEY NOT NULL"`)
<<<<<<< Updated upstream

#### Files describing tables with binary data have additional requirements:
- Only one blob per table
- **the dummy entry** requires a `"DATA":"BLOB"` field!
- **all regular entries** require a `"FILENAME":"dir/EXAMPLE_NAME"` field!
- These files are to be stored in `binary/`.
- The data to which they point are to be stored in `binary/blob/`
  - (subdirectories are always acceptable, just make sure it is recorded in the manifest.)
- The filenames defined in the JSON files *must exist*, or else the program will undergo rapid unplanned disassembly.
=======
- BLOB fields require a filename for the value
	- include subdirectories in the value!
	- *the filename must exist*, or the program will undergo rapid unplanned disassembly!
>>>>>>> Stashed changes
