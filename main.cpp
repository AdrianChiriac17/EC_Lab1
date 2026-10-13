#include "utils.h"

#include <fstream>
#include <iostream>

using namespace std;

int main(int argc, char* argv[])
{
    ifstream file;
    if (argc > 1)
    {
        file.open(argv[1]);
        if (!file)
        {
            cerr << "Cannot open " << argv[1] << '\n';
            return 1;
        }
    }

    Election election;
    if (!read_input(argc > 1 ? file : cin, election))
    {
        return 2;
    }

    run_election(election, cout);
    return 0;
}
