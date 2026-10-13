// Ranked voting - Instant-Runoff Voting (IRV).
//
// Input (file given as argument, or standard input):
//     # comment
//     candidates: A, B, C
//     4: A > B > C        <- 4 voters with this ballot (count is optional, default 1)
//     B > C               <- partial rankings are allowed
//
// Tie-breaking rule for last place:
//     1. look back at previous rounds (most recent first) and eliminate the tied
//        candidate with fewer votes in the first round where they differ;
//     2. if they were always tied, eliminate the one listed last in 'candidates:'.
//
// Build: g++ -std=c++17 -Wall -Wextra -Wpedantic -o lab1 main.cpp
// Run: ./lab1 example.txt (Windows: ./lab1.exe example.txt)

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
using namespace std;

// Max ballots and candidates
#define MAX_BALLOTS 1000
#define MAX_CANDIDATES 20

// preferences[i][j] is jth preference for ballot i
int preferences[MAX_BALLOTS][MAX_CANDIDATES];

// voters[i] is how many people cast ballot i, ranks[i] how many candidates it ranks
int voters[MAX_BALLOTS];
int ranks[MAX_BALLOTS];

// history[r][i] is the number of votes of candidate i in round r
int history[MAX_CANDIDATES + 1][MAX_CANDIDATES];

// Candidates have name, vote count, eliminated status
struct Candidate
{
    string name;
    int votes;
    bool eliminated;
};

// Array of candidates
Candidate candidates[MAX_CANDIDATES];

// Numbers of ballots and candidates
int ballot_count = 0;
int candidate_count = 0;

// Ballots that count for someone / ballots with no remaining candidate
int valid;
int exhausted;

// Function prototypes
bool read_input(istream& in);
bool read_candidates(stringstream& words);
bool read_ballot(stringstream& words, string word);
bool vote(int ballot, int rank, string name);
void tabulate(void);
void print_round(int round);
void save_history(int round);
bool print_winner(void);
int find_min(void);
int find_loser(int min, int round);
bool loses_tie(int a, int b, int round);
void eliminate(int loser);

int main(int argc, char* argv[])
{
    // Read from the file given as argument, or from standard input
    ifstream file;
    if (argc > 1)
    {
        file.open(argv[1]);
        if (!file)
        {
            cout << "Cannot open " << argv[1] << "\n";
            return 1;
        }
    }
    if (!read_input(argc > 1 ? file : cin))
    {
        return 2;
    }

    // Keep holding runoffs until winner exists
    for (int round = 1; ; round++)
    {
        // Calculate votes given remaining candidates
        tabulate();
        print_round(round);
        save_history(round);

        // Stop if nobody has any votes left
        if (valid == 0)
        {
            cout << "\nNo winner - all ballots exhausted.\n";
            break;
        }

        // Check if election has been won
        bool won = print_winner();
        if (won)
        {
            break;
        }

        // Eliminate the last-place candidate (ties broken by the tie rule)
        int min = find_min();
        int loser = find_loser(min, round);
        eliminate(loser);
    }
    return 0;
}

// Read the candidates line and the ballot lines
bool read_input(istream& in)
{
    string line;
    while (getline(in, line))
    {
        stringstream words(line);
        string word;

        // Skip blank lines and comments
        if (!(words >> word) || word[0] == '#')
        {
            continue;
        }

        bool ok;
        if (word == "candidates:")
        {
            ok = read_candidates(words);
        }
        else
        {
            ok = read_ballot(words, word);
        }
        if (!ok)
        {
            return false;
        }
    }
    return true;
}

// Read "candidates: A, B, C" (the word "candidates:" was already read)
bool read_candidates(stringstream& words)
{
    string name;
    while (words >> name)
    {
        if (name.back() == ',')
        {
            name.pop_back();
        }
        if (candidate_count == MAX_CANDIDATES)
        {
            cout << "Maximum number of candidates is " << MAX_CANDIDATES << "\n";
            return false;
        }
        candidates[candidate_count].name = name;
        candidates[candidate_count].votes = 0;
        candidates[candidate_count].eliminated = false;
        candidate_count++;
    }
    return true;
}

// Read "4: A > B > C" or "A > B" (word is the first word of the line)
bool read_ballot(stringstream& words, string word)
{
    if (ballot_count == MAX_BALLOTS)
    {
        cout << "Maximum number of ballots is " << MAX_BALLOTS << "\n";
        return false;
    }
    int ballot = ballot_count;
    voters[ballot] = 1;
    ranks[ballot] = 0;

    do
    {
        if (word.back() == ':')
        {
            // "4:" is the number of voters with this ballot
            voters[ballot] = stoi(word);
        }
        else if (word != ">")
        {
            // Record vote, unless it's invalid
            if (!vote(ballot, ranks[ballot], word))
            {
                cout << "Invalid vote: unknown candidate " << word << "\n";
                return false;
            }
            ranks[ballot]++;
        }
    }
    while (words >> word);

    ballot_count++;
    return true;
}

// Record preference if vote is valid
bool vote(int ballot, int rank, string name)
{
    for (int i = 0; i < candidate_count; i++)
    {
        if (name == candidates[i].name)
        {
            preferences[ballot][rank] = i;
            return true;
        }
    }
    return false;
}

// Tabulate votes for non-eliminated candidates
void tabulate(void)
{
    for (int i = 0; i < candidate_count; i++)
    {
        candidates[i].votes = 0;
    }
    valid = 0;
    exhausted = 0;

    for (int i = 0; i < ballot_count; i++)
    {
        bool counted = false;
        for (int j = 0; j < ranks[i]; j++)
        {
            int candidat_ales = preferences[i][j];
            if (!candidates[candidat_ales].eliminated)
            {
                candidates[candidat_ales].votes += voters[i];
                counted = true;
                break;
            }
        }

        // A ballot with no remaining candidate is exhausted
        if (counted)
        {
            valid += voters[i];
        }
        else
        {
            exhausted += voters[i];
        }
    }
}

// Print the vote count of each remaining candidate
void print_round(int round)
{
    cout << "Round " << round << ":\n";
    for (int i = 0; i < candidate_count; i++)
    {
        if (!candidates[i].eliminated)
        {
            cout << "  " << candidates[i].name << ": " << candidates[i].votes << "\n";
        }
    }
    cout << "  exhausted: " << exhausted << ", valid: " << valid << "\n";
}

// Remember the votes of this round (used to break ties later)
void save_history(int round)
{
    for (int i = 0; i < candidate_count; i++)
    {
        history[round][i] = candidates[i].votes;
    }
}

// Print the winner of the election, if there is one
bool print_winner(void)
{
    for (int i = 0; i < candidate_count; i++)
    {
        // More than 50% of the valid ballots
        if (!candidates[i].eliminated && candidates[i].votes * 2 > valid)
        {
            cout << "\nWinner: " << candidates[i].name << " with "
                 << candidates[i].votes << " of " << valid << " valid votes\n";
            return true;
        }
    }
    return false;
}

// Return the minimum number of votes any remaining candidate has
int find_min(void)
{
    int min_votes = valid;
    for (int i = 0; i < candidate_count; i++)
    {
        if (!candidates[i].eliminated && candidates[i].votes < min_votes)
        {
            min_votes = candidates[i].votes;
        }
    }
    return min_votes;
}

// Return the candidate to eliminate among those with min votes
int find_loser(int min, int round)
{
    int loser = -1;
    for (int i = 0; i < candidate_count; i++)
    {
        if (!candidates[i].eliminated && candidates[i].votes == min)
        {
            if (loser == -1 || loses_tie(i, loser, round))
            {
                loser = i;
            }
        }
    }
    return loser;
}

// Return true if tied candidate a should be eliminated before tied candidate b
bool loses_tie(int a, int b, int round)
{
    // Rule 1: the first earlier round (going back) where they had different votes
    for (int r = round - 1; r >= 1; r--)
    {
        if (history[r][a] != history[r][b])
        {
            return history[r][a] < history[r][b];
        }
    }

    // Rule 2: always tied, so the one listed last loses
    return a > b;
}

// Eliminate the candidate in last place
void eliminate(int loser)
{
    candidates[loser].eliminated = true;
    cout << "  -> eliminated: " << candidates[loser].name << "\n\n";
}
