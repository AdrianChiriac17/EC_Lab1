#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

using namespace std;

// Election data.
struct Candidate
{
    string name;
    int votes = 0;
    bool eliminated = false;
};

struct Ballot
{
    int voters = 1;
    vector<int> preferences; // Candidate indices, in preference order.
};

struct Election
{
    vector<Candidate> candidates;
    vector<Ballot> ballots;
};

namespace
{
struct RoundTotals
{
    int valid = 0;
    int exhausted = 0;
};

using VoteHistory = vector<vector<int>>;

// Read candidate names in declaration order and add them to the election.
void read_candidates(istringstream& words, Election& election)
{
    string name;
    while (words >> name)
    {
        if (name.back() == ',')
        {
            name.pop_back();
        }
        election.candidates.push_back({name});
    }
}

// Read one ballot's voter count and ranked candidate indices; reject invalid input.
bool read_ballot(istringstream& words, string word, Election& election)
{
    Ballot ballot;
    if (word.back() == ':')
    {
        const string count = word.substr(0, word.size() - 1);
        try
        {
            size_t parsed = 0;
            ballot.voters = stoi(count, &parsed);
            if (parsed != count.size() || ballot.voters <= 0)
            {
                throw invalid_argument("voter count");
            }
        }
        catch (const exception&)
        {
            cerr << "Invalid voter count: " << count << '\n';
            return false;
        }
        if (!(words >> word))
        {
            // An empty ranking is immediately exhausted.
            election.ballots.push_back(move(ballot));
            return true;
        }
    }

    do
    {
        if (word == ">")
        {
            continue;
        }
        const auto candidate = find_if(
            election.candidates.begin(), election.candidates.end(),
            [&word](const Candidate& entry) { return entry.name == word; });
        if (candidate == election.candidates.end())
        {
            cerr << "Invalid vote: unknown candidate " << word << '\n';
            return false;
        }
        ballot.preferences.push_back(
            static_cast<int>(candidate - election.candidates.begin()));
    }
    while (words >> word);

    election.ballots.push_back(move(ballot));
    return true;
}

// Count votes for each ballot's first remaining candidate; track exhausted votes.
RoundTotals tabulate(Election& election)
{
    for (auto& candidate : election.candidates)
    {
        candidate.votes = 0;
    }

    RoundTotals totals;
    for (const auto& ballot : election.ballots)
    {
        const auto preference = find_if(
            ballot.preferences.begin(), ballot.preferences.end(),
            [&election](int index) { return !election.candidates[index].eliminated; });
        if (preference == ballot.preferences.end())
        {
            totals.exhausted += ballot.voters;
        }
        else
        {
            election.candidates[*preference].votes += ballot.voters;
            totals.valid += ballot.voters;
        }
    }
    return totals;
}

// Display this round's vote counts and valid/exhausted totals.
void print_round(const Election& election, const RoundTotals& totals,
                 int round, ostream& output)
{
    output << "Round " << round << ":\n";
    for (const auto& candidate : election.candidates)
    {
        if (!candidate.eliminated)
        {
            output << "  " << candidate.name << ": " << candidate.votes << '\n';
        }
    }
    output << "  exhausted: " << totals.exhausted << ", valid: " << totals.valid << '\n';
}

// Decide whether anyone has more than half the valid votes; print the winner if so.
bool print_winner(const Election& election, int valid, ostream& output)
{
    for (const auto& candidate : election.candidates)
    {
        if (!candidate.eliminated && candidate.votes > valid / 2)
        {
            output << "\nWinner: " << candidate.name << " with "
                   << candidate.votes << " of " << valid << " valid votes\n";
            return true;
        }
    }
    return false;
}

// Decide whether a loses a tie to b using earlier rounds, then the larger name.
bool loses_tie(int a, int b, const Election& election, const VoteHistory& history)
{
    // Compare previous rounds, starting with the most recent.
    for (auto round = history.rbegin(); round != history.rend(); ++round)
    {
        if ((*round)[a] != (*round)[b])
        {
            return (*round)[a] < (*round)[b];
        }
    }
    // Always tied: eliminate the lexicographically larger name.
    return election.candidates[a].name > election.candidates[b].name;
}

// Choose the candidate to eliminate: fewest votes, with ties resolved by loses_tie().
int find_loser(const Election& election, const VoteHistory& history)
{
    int loser = -1;
    for (size_t i = 0; i < election.candidates.size(); ++i)
    {
        const auto& candidate = election.candidates[i];
        if (candidate.eliminated)
        {
            continue;
        }
        if (loser == -1 || candidate.votes < election.candidates[loser].votes ||
            (candidate.votes == election.candidates[loser].votes &&
             loses_tie(static_cast<int>(i), loser, election, history)))
        {
            loser = static_cast<int>(i);
        }
    }
    return loser;
}
} // namespace

// Read the election line by line, skipping comments and blanks; parse names and ballots.
bool read_input(istream& input, Election& election)
{
    string line;
    while (getline(input, line))
    {
        istringstream words(line);
        string word;
        if (!(words >> word) || word.front() == '#')
        {
            continue;
        }
        if (word == "candidates:")
        {
            read_candidates(words, election);
        }
        else if (!read_ballot(words, word, election))
        {
            return false;
        }
    }
    return true;
}

// Repeat counting and winner checks; save history and eliminate the chosen loser.
void run_election(Election& election, ostream& output)
{
    VoteHistory history;
    for (int round = 1; ; ++round)
    {
        const auto totals = tabulate(election);
        print_round(election, totals, round, output);
        if (totals.valid == 0)
        {
            output << "\nNo winner - all ballots exhausted.\n";
            return;
        }
        if (print_winner(election, totals.valid, output))
        {
            return;
        }

        const int loser = find_loser(election, history);
        vector<int> votes;
        for (const auto& candidate : election.candidates)
        {
            votes.push_back(candidate.votes);
        }
        history.push_back(move(votes));

        election.candidates[loser].eliminated = true;
        output << "  -> eliminated: " << election.candidates[loser].name << "\n\n";
    }
}

// Open the input file (or use standard input), load the election, and start the runoffs.
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
