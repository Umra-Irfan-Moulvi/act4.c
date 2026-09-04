# act4.c
# Simple Line Editor (C)

A terminal-based line editor built for the "Build a Simple Line Editor in C" studio
competition. See `DESIGN.md` for the paper design (data structure choice + sketched
functions, done before any code was written) and `HELP.md` for full command
documentation with examples.

## Team

- UMRA IRFAN -R25EJ170
- SAKSHAM SRIVASTAVA - R25EJ131
- RISHIT YADAV - R25EJ121

## Features implemented

**Core (4 of 4 — only 2–3 were required):**
- Insert a line (`i`)
- Delete a line (`d`)
- Display the document (`p`)
- Save / load to a `.txt` file (`s` / `o`)

**Bonus:**
- Search (`f`) — reports which line number(s) contain a word/phrase
- Line/word/character count (`w`)

## Data structure

Dynamic array of `char*` (grows via `realloc`), chosen for O(1) line access and good
cache locality over a document of realistic size. Full justification and trade-off
discussion is in `DESIGN.md`.

## How to compile and run

```bash
gcc -Wall -Wextra -o editor editor.c
./editor
```

Then type commands at the `>` prompt (`h` for help, `q` to quit). Example session:

```
> i 1 Hello world
> i 2 Second line
> p
   1| Hello world
   2| Second line
> s mydoc.txt
Saved 2 line(s) to 'mydoc.txt'.
> q
```

## Project structure

```
.
├── editor.c     # all source code
├── DESIGN.md    # paper design deliverable
├── HELP.md      # command reference with examples
└── README.md    # this file
```

## Testing

Manually verified: insert at start/middle/end, delete down to an empty document,
save/reload round-trip, search with and without matches, and error handling for
out-of-range line numbers and missing files — none of these crash the program.
