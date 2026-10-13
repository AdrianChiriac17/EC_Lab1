# EC Lab 1: Instant-runoff voting

A small C++ program that counts ranked ballots, eliminates the candidate with
the fewest votes each round, and elects a candidate with more than half of the
remaining valid votes. Ties use earlier rounds, then candidate declaration order.

## Windows setup

Install GCC/G++ using MSYS2 UCRT64 and open this folder in VS Code with Microsoft's
C/C++ extension. The checked-in editor configuration expects the compiler at
`C:/msys64/ucrt64/bin/g++.exe`; update the paths in `.vscode/` if MSYS2 is installed
elsewhere. Standard-library include directories are discovered from the compiler.

After opening the project or changing settings, run **Developer: Reload Window**
and open a new terminal to pick up the UCRT64 and Git paths. If include errors
remain, run **C/C++: Reset IntelliSense Database** and select **MSYS2 UCRT64** via
**C/C++: Select a Configuration**.

Press **Ctrl+Shift+B** to build, or use **Terminal: Run Task** and select
**Run lab1 example** to build and run the example.

In a new VS Code PowerShell terminal:

```powershell
g++ -std=c++17 -Wall -Wextra -Wpedantic -g main.cpp -o lab1.exe
.\lab1.exe example.txt
.\lab1.exe test_tie_first_round.txt
.\lab1.exe test_tie_backward.txt
.\lab1.exe test_exhausted.txt
```

The expected winners are B, A, A, and C respectively. The executable needs the
UCRT64 `bin` directory on `PATH` for its runtime DLLs; the VS Code tasks and new
integrated terminals include this automatically. These settings apply to this
project and do not change the system-wide Windows `PATH`.

## Input format

Pass a file path as the first argument, or omit it to read from standard input.
Blank lines and lines beginning with `#` are ignored.

```text
candidates: A, B, C
4: A > B > C
3: B > C > A
2: C > B > A
```

Counts are optional and default to one voter. Partial rankings are allowed;
ballots become exhausted when none of their ranked candidates remains.
The program supports up to 20 candidates and 1,000 ballot lines.
