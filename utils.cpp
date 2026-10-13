#include "utils.h"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <utility>

using namespace std;

namespace
{
struct RoundTotals
{
    int valid = 0;
    int exhausted = 0;
};

using VoteHistory = vector<vector<int>>;

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

bool loses_tie(int a, int b, const VoteHistory& history)
{
    // Compare previous rounds, starting with the most recent.
    for (auto round = history.rbegin(); round != history.rend(); ++round)
    {
        if ((*round)[a] != (*round)[b])
        {
            return (*round)[a] < (*round)[b];
        }
    }
    return a > b; // Always tied: the candidate declared last loses.
}

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
             loses_tie(static_cast<int>(i), loser, history)))
        {
            loser = static_cast<int>(i);
        }
    }
    return loser;
}
} // namespace

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
