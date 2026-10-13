#ifndef UTILS_H
#define UTILS_H

#include <iosfwd>
#include <string>
#include <vector>

struct Candidate
{
    std::string name;
    int votes = 0;
    bool eliminated = false;
};

struct Ballot
{
    int voters = 1;
    std::vector<int> preferences; // Candidate indices, in preference order.
};

struct Election
{
    std::vector<Candidate> candidates;
    std::vector<Ballot> ballots;
};

bool read_input(std::istream& input, Election& election);
void run_election(Election& election, std::ostream& output);

#endif
