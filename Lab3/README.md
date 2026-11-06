# E-Commerce Lab 3: Dinner VCG

Single-file C++17 implementation with CSV input and command-line / stdin options.

## Compile

```bash
g++ -std=c++17 -Wall -Wextra -pedantic -O2 main.cpp -o dinner
```

## Run

```bash
./dinner tests/example.csv
./dinner < tests/example.csv
```

On Windows (MinGW), compile with `-o dinner.exe`, then run `./dinner.exe tests/example.csv` from Bash or `dinner.exe tests/example.csv` from CMD.

Input format: first column must be `friend`, followed by at least one restaurant. One participant per following row. Numbers may be negative or decimal. The first restaurant wins exact ties.

## VCG equations

- Choose restaurant `o*` maximizing total reported value.
- Payment for player i = `max_o(sum_{j != i} v_j(o)) - sum_{j != i} v_j(o*)`.
- Gain = `v_i(o*) - payment_i`.

CSV tests: `example.csv`, `tie.csv`, `single_friend.csv`, `negative_values.csv`, `empty.csv`, `missing_value.csv`, `non_numeric.csv`. Extra: `only_header.csv`.

Error cases exit with nonzero status.
