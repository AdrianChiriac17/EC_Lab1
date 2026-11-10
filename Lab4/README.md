# E-Commerce Lab 4: Iterated Prisoner's Dilemma

Single-file C++17 tournament with four strategies: TitForTat, Grudger,
AlwaysDefect, and TitForTwoTats. All classes and game logic are in `main.cpp`.

## Build and run

Run from the `Lab4` directory. Windows PowerShell:

```powershell
g++ -std=c++17 -Wall -Wextra -pedantic -O2 main.cpp -o lab4.exe
.\lab4.exe input.txt
.\lab4.exe tests\two_players.txt
```

Linux:

```bash
g++ -std=c++17 -Wall -Wextra -pedantic -O2 main.cpp -o lab4
./lab4 input.txt
```

## Input and rules

Supply exactly one configuration filename; there is no interactive input.

```ini
ROUNDS=500
PLAYERS=TitForTat,Grudger,AlwaysDefect,TitForTwoTats
```

`ROUNDS` must be a positive integer fitting in `int`; it defaults to 500 when
omitted. `PLAYERS` is required and must contain at least two distinct supported
strategies. Whitespace, blank lines, and full-line `#` comments are supported.
Invalid configuration or usage produces an error on stderr and exit code 1.

Both players choose simultaneously from past histories. Payoffs are `C/C: 3/3`,
`C/D: 0/5`, `D/C: 5/0`, and `D/D: 1/1`. Every ordered pair plays separately,
with fresh histories: four players produce **12 matches**, excluding self-matches.

TitForTat copies the opponent's last move after cooperating first. Grudger
cooperates until the opponent defects, then always defects. AlwaysDefect always
defects. The selected fourth strategy, TitForTwoTats, defects only after two
consecutive opponent defections and resumes cooperation when that condition clears.

Output includes a score matrix and ranking by total points. Equal totals share
a rank, with tied names listed alphabetically. With `input.txt`, Grudger and
TitForTat tie for first at **6998**, followed by TitForTwoTats at **6996** and
AlwaysDefect at **3032**.

## Test inputs

| File in `tests/` | Expected result |
| --- | --- |
| `one_round.txt` | AlwaysDefect: 30; all others tie at 12 |
| `two_players.txt` | Two ordered matches; TitForTat/AlwaysDefect: 499/504 per match |
| `two_tats_three_rounds.txt` | TitForTwoTats first retaliates in round 3; totals: 2 and 22 |
| `reordered_players.txt` | Same standard totals and ranking; table follows input order |
| `invalid_duplicate_player.txt` | Duplicate strategy error, exit code 1 |
| `invalid_zero_rounds.txt` | Invalid round count error, exit code 1 |
