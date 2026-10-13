# EC Lab 1: Instant-runoff voting

Count ranked ballots to choose a winner with a majority of the votes still in
play. Unlike plurality voting, which only considers first choices, instant-runoff
voting (IRV) can transfer votes to each voter's next choice.

## How the election works

1. Each ballot counts for its highest-ranked candidate who has not been eliminated.
2. A candidate wins with **more than half** of the valid votes in that round.
3. If nobody wins, eliminate one candidate with the fewest votes and count again.

A partial ballot becomes **exhausted** when every candidate it ranks has been
eliminated. Exhausted votes are excluded from the majority threshold. For example,
7 votes are enough to win when only 13 votes remain valid, even if 15 were cast.
If no valid votes remain, the election ends without a winner.

For a tie in last place, compare the tied candidates' vote totals in previous
rounds, starting with the most recent. The candidate with fewer votes in the first
round where they differ is eliminated. If they have always been tied (including
a tie in the first round), eliminate the candidate listed last in `candidates:`.
Exactly one candidate is eliminated per round.

## Input and example

Pass a file path as the first argument, or omit it to read from standard input.
Blank lines and lines beginning with `#` are ignored. Declare candidates before
the ballots; candidate names are single tokens, separated by spaces, with optional
trailing commas.

```text
candidates: A, B, C
4: A > B > C
3: B > C > A
2: C > B > A
```

Each line describes a ranking shared by a group of voters. The positive integer
before `:` is optional and defaults to one voter, so `B > C` is also a ballot.
Partial rankings are allowed. Preferences store candidate indices in vectors;
there are no fixed limits of 20 candidates or 1,000 ballot lines.

In this example, A leads the first round with 4 votes, B has 3, and C has 2.
Nobody has a majority of the 9 votes. C is eliminated, transferring its 2 votes
to B. B wins the second round with 5 of 9 votes, while plurality would elect A.

The included inputs illustrate the main rules:

| File | What it demonstrates | Winner |
| --- | --- | --- |
| `example.txt` | Vote transfers change the initial leader | B |
| `test_tie_first_round.txt` | Declaration order breaks a first-round tie | A |
| `test_tie_backward.txt` | Earlier rounds break later ties | A |
| `test_exhausted.txt` | Exhausted ballots reduce the majority threshold | C |

## Build and run

With a C++17 compiler available on `PATH`:

```sh
g++ -std=c++17 -Wall -Wextra -Wpedantic -g main.cpp utils.cpp -o lab1
./lab1 example.txt
```

On Windows, use `-o lab1.exe` and run `.\lab1.exe example.txt`.
In VS Code, **Ctrl+Shift+B** builds both source files; the **Run lab1 example**
task builds and runs the example.

`main.cpp` handles file selection and starts the election. `utils.h` declares
the candidate, ballot, and election data types, and `utils.cpp` handles parsing,
counting, tie-breaking, and results. Election data uses `std::vector` rather than
fixed arrays or global state.
