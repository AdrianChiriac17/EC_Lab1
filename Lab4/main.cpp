#include <algorithm>
#include <charconv>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <numeric>
#include <stdexcept>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

using namespace std;

// Every strategy receives read-only histories ending at the previous round.
class Player
{
public:
    virtual ~Player() = default;
    virtual string getName() const = 0;
    virtual char move(const vector<char>& myHistory,
                      const vector<char>& opponentHistory) const = 0;
};

class TitForTat : public Player
{
public:
    string getName() const override { return "TitForTat"; }

    char move(const vector<char>&, const vector<char>& opponentHistory) const override
    {
        return opponentHistory.empty() ? 'C' : opponentHistory.back();
    }
};

class Grudger : public Player
{
public:
    string getName() const override { return "Grudger"; }

    char move(const vector<char>&, const vector<char>& opponentHistory) const override
    {
        return find(opponentHistory.begin(), opponentHistory.end(), 'D')
                       == opponentHistory.end() ? 'C' : 'D';
    }
};

class AlwaysDefect : public Player
{
public:
    string getName() const override { return "AlwaysDefect"; }

    char move(const vector<char>&, const vector<char>&) const override
    {
        return 'D';
    }
};

class TitForTwoTats : public Player
{
public:
    string getName() const override { return "TitForTwoTats"; }

    char move(const vector<char>&, const vector<char>& opponentHistory) const override
    {
        const size_t size = opponentHistory.size();
        return size >= 2 && opponentHistory[size - 2] == 'D'
                         && opponentHistory[size - 1] == 'D' ? 'D' : 'C';
    }
};

struct Configuration
{
    int rounds = 500;
    vector<string> playerNames;
};

string trim(const string& text)
{
    const auto first = text.find_first_not_of(" \t\r\n\f\v");
    if (first == string::npos)
        return "";

    const auto last = text.find_last_not_of(" \t\r\n\f\v");
    return text.substr(first, last - first + 1);
}

bool isStrategyName(const string& name)
{
    return name == "TitForTat" || name == "Grudger"
        || name == "AlwaysDefect" || name == "TitForTwoTats";
}

int parseRounds(const string& value)
{
    int rounds = 0;
    const auto parsed = from_chars(value.data(), value.data() + value.size(), rounds);
    if (parsed.ec != errc{} || parsed.ptr != value.data() + value.size() || rounds <= 0)
        throw runtime_error("ROUNDS must be a positive integer that fits in int.");

    return rounds;
}

vector<string> parsePlayers(const string& value)
{
    vector<string> names;
    size_t start = 0;
    while (true)
    {
        const auto comma = value.find(',', start);
        const string name = trim(value.substr(start, comma == string::npos
                                                       ? string::npos : comma - start));
        if (name.empty())
            throw runtime_error("PLAYERS contains an empty strategy name.");
        if (!isStrategyName(name))
            throw runtime_error("Unknown strategy '" + name + "'.");
        if (find(names.begin(), names.end(), name) != names.end())
            throw runtime_error("Duplicate strategy '" + name + "'.");

        names.push_back(name);
        if (comma == string::npos)
            break;
        start = comma + 1;
    }

    if (names.size() < 2)
        throw runtime_error("PLAYERS must contain at least two distinct strategies.");

    return names;
}

Configuration readConfiguration(const string& filename)
{
    ifstream input(filename);
    if (!input)
        throw runtime_error("Cannot open configuration file '" + filename + "'.");

    Configuration configuration;
    bool hasRounds = false;
    bool hasPlayers = false;
    string line;
    size_t lineNumber = 0;
    while (getline(input, line))
    {
        ++lineNumber;
        line = trim(line);
        if (line.empty() || line.front() == '#')
            continue;

        try
        {
            const auto equals = line.find('=');
            if (equals == string::npos || line.find('=', equals + 1) != string::npos)
                throw runtime_error("Expected one KEY=VALUE pair.");

            const string key = trim(line.substr(0, equals));
            const string value = trim(line.substr(equals + 1));
            if (key == "ROUNDS")
            {
                if (hasRounds)
                    throw runtime_error("Duplicate key 'ROUNDS'.");
                configuration.rounds = parseRounds(value);
                hasRounds = true;
            }
            else if (key == "PLAYERS")
            {
                if (hasPlayers)
                    throw runtime_error("Duplicate key 'PLAYERS'.");
                configuration.playerNames = parsePlayers(value);
                hasPlayers = true;
            }
            else
            {
                throw runtime_error("Unknown configuration key '" + key + "'.");
            }
        }
        catch (const exception& error)
        {
            throw runtime_error(filename + ":" + to_string(lineNumber) + ": " + error.what());
        }
    }

    if (input.bad() || (input.fail() && !input.eof()))
        throw runtime_error("Error reading configuration file '" + filename + "'.");
    if (!hasPlayers)
        throw runtime_error("Configuration file '" + filename + "' is missing PLAYERS.");

    return configuration;
}

unique_ptr<Player> createPlayer(const string& name)
{
    if (name == "TitForTat") return make_unique<TitForTat>();
    if (name == "Grudger") return make_unique<Grudger>();
    if (name == "AlwaysDefect") return make_unique<AlwaysDefect>();
    if (name == "TitForTwoTats") return make_unique<TitForTwoTats>();
    throw invalid_argument("Unknown strategy '" + name + "'.");
}

pair<int, int> payoff(char a, char b)
{
    if ((a != 'C' && a != 'D') || (b != 'C' && b != 'D'))
        throw invalid_argument("Payoff requires moves C or D.");
    if (a == 'C' && b == 'C') return {3, 3};
    if (a == 'C' && b == 'D') return {0, 5};
    if (a == 'D' && b == 'C') return {5, 0};
    return {1, 1};
}

struct MatchResult
{
    // Even INT_MAX rounds and all six appearances per player fit in long long.
    long long scoreA = 0;
    long long scoreB = 0;
};

void validateMove(char move, const Player& player, int round)
{
    if (move != 'C' && move != 'D')
        throw runtime_error("Invalid move from " + player.getName()
                            + " in round " + to_string(round) + ": expected C or D.");
}

MatchResult playGame(const Player& playerA, const Player& playerB, int rounds)
{
    if (rounds <= 0)
        throw invalid_argument("A match requires a positive round count.");

    vector<char> historyA;
    vector<char> historyB;
    historyA.reserve(static_cast<size_t>(rounds));
    historyB.reserve(static_cast<size_t>(rounds));
    MatchResult result;

    for (int round = 0; round < rounds; ++round)
    {
        // Select both moves before appending either one: decisions are simultaneous.
        const char a = playerA.move(historyA, historyB);
        const char b = playerB.move(historyB, historyA);
        validateMove(a, playerA, round + 1);
        validateMove(b, playerB, round + 1);

        const auto points = payoff(a, b);
        result.scoreA += points.first;
        result.scoreB += points.second;
        historyA.push_back(a);
        historyB.push_back(b);
    }
    return result;
}

struct TournamentResult
{
    vector<vector<MatchResult>> table;
    vector<long long> totals;
    size_t matchCount = 0;
};

TournamentResult runTournament(const vector<unique_ptr<Player>>& players, int rounds)
{
    const size_t size = players.size();
    TournamentResult result{vector<vector<MatchResult>>(size, vector<MatchResult>(size)),
                            vector<long long>(size, 0), 0};

    for (size_t i = 0; i < size; ++i)
    {
        for (size_t j = 0; j < size; ++j)
        {
            if (i == j)
                continue;

            const MatchResult match = playGame(*players[i], *players[j], rounds);
            result.table[i][j] = match;
            result.totals[i] += match.scoreA;
            result.totals[j] += match.scoreB;
            ++result.matchCount;
        }
    }
    return result;
}

string formatScore(const MatchResult& match)
{
    return to_string(match.scoreA) + "/" + to_string(match.scoreB);
}

void printTable(const vector<unique_ptr<Player>>& players, const TournamentResult& result)
{
    size_t width = string("Row / Column").size();
    for (const auto& player : players)
        width = max(width, player->getName().size());
    for (size_t i = 0; i < players.size(); ++i)
        for (size_t j = 0; j < players.size(); ++j)
            if (i != j)
                width = max(width, formatScore(result.table[i][j]).size());

    const int columnWidth = static_cast<int>(width);
    cout << "\nMatch scores (row player / column player; - means not played)\n";
    cout << left << setw(columnWidth) << "Row / Column";
    for (const auto& player : players)
        cout << " | " << setw(columnWidth) << player->getName();
    cout << '\n';

    for (size_t i = 0; i < players.size(); ++i)
    {
        cout << setw(columnWidth) << players[i]->getName();
        for (size_t j = 0; j < players.size(); ++j)
            cout << " | " << setw(columnWidth) << (i == j ? "-" : formatScore(result.table[i][j]));
        cout << '\n';
    }
}

void printRanking(const vector<unique_ptr<Player>>& players, const TournamentResult& result)
{
    vector<size_t> order(players.size());
    iota(order.begin(), order.end(), size_t{0});
    sort(order.begin(), order.end(), [&](size_t a, size_t b)
    {
        if (result.totals[a] != result.totals[b])
            return result.totals[a] > result.totals[b];
        return players[a]->getName() < players[b]->getName();
    });

    cout << "\nFinal ranking (equal totals share a rank; ties listed alphabetically)\n";
    cout << "Rank | Strategy | Total points\n";
    size_t rank = 0;
    for (size_t position = 0; position < order.size(); ++position)
    {
        const size_t index = order[position];
        if (position == 0 || result.totals[index] != result.totals[order[position - 1]])
            rank = position + 1;
        cout << rank << " | " << players[index]->getName() << " | " << result.totals[index] << '\n';
    }
}

void printResult(const vector<unique_ptr<Player>>& players, int rounds,
                 const TournamentResult& result)
{
    cout << "E-Commerce Lab 4: Iterated Prisoner's Dilemma\n"
         << "Players: " << players.size() << "\nRounds per match: " << rounds
         << "\nOrdered matches: " << result.matchCount
         << "\nTotal rounds: " << static_cast<long long>(result.matchCount) * rounds << '\n';

    printTable(players, result);
    printRanking(players, result);
    for (const auto& player : players)
    {
        if (player->getName() == "TitForTwoTats")
        {
            cout << "\nCustom strategy: TitForTwoTats cooperates unless the opponent's two\n"
                 << "immediately preceding moves were both D. It forgives an isolated defection\n"
                 << "and resumes cooperation once that condition clears. This preserves mutual\n"
                 << "cooperation with TitForTat and Grudger, but lets AlwaysDefect exploit it\n"
                 << "for two rounds before retaliation (see the matchup scores above).\n";
            break;
        }
    }
}

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        cerr << "Usage: " << argv[0] << " <configuration-file>\n";
        return 1;
    }

    try
    {
        const Configuration configuration = readConfiguration(argv[1]);
        vector<unique_ptr<Player>> players;
        for (const string& name : configuration.playerNames)
            players.push_back(createPlayer(name));

        const TournamentResult result = runTournament(players, configuration.rounds);
        printResult(players, configuration.rounds, result);
        return 0;
    }
    catch (const exception& error)
    {
        cerr << "Error: " << error.what() << '\n';
        return 1;
    }
}
