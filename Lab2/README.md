# EC Lab 2: Vickrey auction

The highest bidder wins and pays the second-highest bid. If the highest bids
are tied, the bidder appearing first in the input wins and pays the tied value.
A single bidder wins and pays zero.

## Input and output

Pass a text file as the first argument, or omit it to read from standard input.
Each non-blank line must contain exactly two comma-separated fields: `id,value`.
Surrounding whitespace is ignored. Bids must contain digits for the whole-dollar
part and may have a decimal point followed by exactly one or two digits.
Zero is valid. Signs, scientific notation, missing whole-dollar digits, and more
than two decimal places are rejected, as is a decimal point without cents (`12.`).

All amounts are parsed and stored as `long long` integer cents: $1 = 100 cents.
No floating-point parsing, storage, or arithmetic is used.

| Input amount | Stored cents |
| --- | ---: |
| `120` | 12000 |
| `120.0` | 12000 |
| `120.00` | 12000 |
| `115.6` | 11560 |
| `0.01` | 1 |

With a 64-bit `long long`, the maximum amount is `92233720368547758.07` dollars.
The parser uses `numeric_limits<long long>::max()` to reject amounts that would
overflow on the current platform. Prices are printed in dollars
with exactly two decimal places, using integer division and remainder.

```text
alice,120
bob,95
carol,150
dave,110
```

Output:

```text
Winner: carol, price: 120.00
```

Empty input, missing fields, and invalid bids produce a clear error on standard
error and exit code 1. Malformed input stops the auction; no winner is printed.
An unreadable file or incorrect command-line usage produces exit code 1.

## Algorithm

`main.cpp` is self-contained. It parses one line at a time, keeping the winning
identifier, highest bid (`first_cents`), and second-highest bid (`second_cents`). There is no
need to store every bidder in a vector or sort the bids.

The first bidder initializes the winner and highest bid; `second_cents` starts at zero.
For each remaining bidder:

1. If the bid is greater than `first_cents`, assign `second_cents = first_cents`
   before replacing `first_cents` and the winner. The old highest bid becomes the runner-up.
2. Otherwise, if the bid is greater than `second_cents`, update `second_cents`.

A bid equal to `first_cents` keeps the earlier winner but sets `second_cents` to the tied
value. The algorithm makes one pass: O(n) time for n bidders, with a constant
number of stored bid records (plus the strings for input and identifiers).

## Build and run

Run these commands from the `Lab2` directory with a C++17 compiler on `PATH`.

Linux:

```sh
g++ -std=c++17 -Wall -Wextra -Wpedantic -g main.cpp -o lab2
./lab2 test_example.txt
```

Windows PowerShell:

```powershell
g++ -std=c++17 -Wall -Wextra -Wpedantic -g main.cpp -o lab2.exe
.\lab2.exe test_example.txt
```

In VS Code, use **Tasks: Run Task** and select **Build lab2** or
**Run lab2 example**. The existing default build task still builds Lab 1.

## Required test files

| File | Expected result |
| --- | --- |
| `test_example.txt` | `Winner: carol, price: 120.00` |
| `test_tie.txt` | `Winner: bob, price: 150.00` |
| `test_single_bidder.txt` | `Winner: alice, price: 0.00` |
| `test_empty.txt` | Error: no bidders |
| `test_missing_field.txt` | Error on line 2 |
| `test_negative_bid.txt` | Error on line 2 |
| `test_non_numeric_bid.txt` | Error on line 2 |

Additional money parsing tests:

| File | Expected result |
| --- | --- |
| `test_cents.txt` | `Winner: alice, price: 115.60` |
| `test_equivalent_amounts.txt` | `Winner: alice, price: 120.00` |
| `test_trailing_decimal.txt` | Error on line 1, exit code 1 |
| `test_excess_decimals.txt` | Error on line 1, exit code 1 |

## Discussion

In a second-price auction, bidding your true valuation is a weakly dominant
strategy: underbidding may lose a profitable opportunity, while overbidding may
win at a price above your valuation. If the winner instead paid their own bid
(a first-price auction), bidders would have an incentive to bid below their
valuation to leave a gain when they win.
