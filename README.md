# EC Lab 1: Instant-runoff voting

A small C++ program that counts ranked ballots, eliminates the candidate with
the fewest votes each round, and elects a candidate with more than half of the
remaining valid votes. Ties use earlier rounds, then candidate declaration order.

## Windows setup

In the MSYS2 **UCRT64** terminal, install GCC/G++:

```sh
pacman -S --needed mingw-w64-ucrt-x86_64-gcc
```

Add your MSYS2 installation's `ucrt64\bin` directory to your Windows user `PATH`
in **Environment Variables** (for example, `C:\msys64\ucrt64\bin`). This lets
Windows find both `g++` and its runtime DLLs. Fully restart VS Code and open a new
terminal after changing `PATH`.

From the repository folder in PowerShell:

```powershell
g++ --version
g++ -std=c++17 -Wall -Wextra -Wpedantic -g main.cpp -o lab1.exe
.\lab1.exe example.txt
.\lab1.exe test_tie_first_round.txt
.\lab1.exe test_tie_backward.txt
.\lab1.exe test_exhausted.txt
```

## Linux Mint / Ubuntu setup

Install the standard compiler tools:

```sh
sudo apt update
sudo apt install build-essential
```

From the repository folder:

```sh
g++ --version
g++ -std=c++17 -Wall -Wextra -Wpedantic -g main.cpp -o lab1
./lab1 example.txt
./lab1 test_tie_first_round.txt
./lab1 test_tie_backward.txt
./lab1 test_exhausted.txt
```

Both platforms use the same C++17 source. The expected winners for these four
inputs are B, A, A, and C respectively. Generated executables are ignored by Git.

## VS Code (Windows and Linux)

Open this repository folder and install Microsoft's **C/C++** extension. The
checked-in configuration finds `g++` on `PATH` and uses `${workspaceFolder}` for
project paths, so it works regardless of where the compiler or repository is
installed. Standard-library include directories are discovered from the compiler.
VS Code automatically selects the `Win32` or `Linux` IntelliSense configuration.

Press **Ctrl+Shift+B** to build, or use **Terminal: Run Task** and select
**Run lab1 example** to build and run the example. The build produces `lab1.exe`
on Windows and `lab1` on Linux.

If `<iostream>` or other standard headers are underlined, first check that
`g++ --version` works in a new terminal, then run **Developer: Reload Window**
and **C/C++: Reset IntelliSense Database**. Use **C/C++: Select a Configuration**
to select your platform if needed.

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
