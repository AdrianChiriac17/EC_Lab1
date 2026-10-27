#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <string>

using namespace std;

struct Bid
{
    string name;
    long long cents = 0;
};

string trim(const string& text);
bool parse_cents(const string& value, long long& cents);
bool read_bid(const string& line, Bid& bid);

// Process bids once, keeping only the winner and the two highest values.
int run_auction(istream& input, ostream& output)
{
    string winner;
    long long first_bid = 0;
    long long second_bid = 0;
    string line;
    size_t line_number = 0;

    while (getline(input, line))
    {
        ++line_number;
        if (trim(line).empty())
        {
            continue;
        }

        Bid current_bid;
        if (!read_bid(line, current_bid))
        {
            cerr << "Invalid input on line " << line_number
                 << ": expected id,value with non-negative dollars and "
                    "optionally one or two decimal digits (e.g. 120, 120.0, 120.00); "
                    "amount in cents must fit in a long long.\n";
            return 1;
        }

        if (winner.empty())
        {
            //set the winner and first bid to the first valid read.
            winner = current_bid.name;
            first_bid = current_bid.cents;
        }
        else if (current_bid.cents > first_bid)
        {
            // A new valid highest bid is found.
            second_bid = first_bid; // Preserve the old highest bid as number 2, so we can tell number 1 what to pay.
            first_bid = current_bid.cents;
            winner = current_bid.name;
        }
        else if (current_bid.cents > second_bid)
        {
            // New valid bid found placing between No1 and No2, so now No2 becomes No3.
            second_bid = current_bid.cents;
        }
    }

    if (input.bad())
    {
        cerr << "Error reading input.\n";
        return 2;
    }
    if (winner.empty())
    {
        cerr << "Invalid input: no bidders.\n";
        return 3;
    }

    output << "Winner: " << winner << ", price: " << second_bid / 100
           << '.' << setfill('0') << setw(2) << second_bid % 100 << '\n';

    return 0;
}

// Read a file supplied as an argument, or use standard input like Lab 1.
int main(int argc, char* argv[])
{
    //if called with more than one argument
    if (argc > 2)
    {
        cerr << "Usage: " << argv[0] << " [input-file]\n";
        return 1;
    }

    //if called with one argument, file read.
    ifstream file;
    if (argc == 2)
    {
        file.open(argv[1]);
        if (!file)
        {
            cerr << "Cannot open " << argv[1] << '\n';
            return 1;
        }
        return run_auction(file, cout);
    }
    else
    {
        //called with no arguments, read from standard input.
        cout<<"Enter bids in the format id,value (e.g. 120, 120.0, 120.00).";
        return run_auction(cin, cout);
    }
}

string trim(const string& text)
{
    const auto first = text.find_first_not_of(" \t\r\n");
    if (first == string::npos)
    {
        return "";
    }
    const auto last = text.find_last_not_of(" \t\r\n");
    return text.substr(first, last - first + 1);
}

// Read dollars and optional one/two-digit cents directly as an integer.
bool parse_cents(const string& value, long long& cents)
{
    const auto dot = value.find('.');
    if (value.empty() || dot == 0)
    {
        return false;
    }

    const size_t decimals = dot == string::npos ? 0 : value.size() - dot - 1;
    if (dot != string::npos && (decimals == 0 || decimals > 2))
    {
        return false;
    }

    cents = 0;
    const auto maximum = numeric_limits<long long>::max();
    for (size_t i = 0; i < value.size(); ++i)
    {
        if (i == dot)
        {
            continue;
        }
        if (value[i] < '0' || value[i] > '9')
        {
            return false;
        }

        const int digit = value[i] - '0';
        if (cents > (maximum - digit) / 10)
        {
            return false;
        }
        cents = cents * 10 + digit;
    }

    // 120 -> 12000; 115.6 -> 11560; 120.00 already has both cents digits.
    for (size_t i = decimals; i < 2; ++i)
    {
        if (cents > maximum / 10)
        {
            return false;
        }
        cents *= 10;
    }
    return true;
}

// Require exactly two fields and a non-negative amount in dollars and cents.
bool read_bid(const string& line, Bid& bid)
{
    const auto comma = line.find(',');
    if (comma == string::npos || line.find(',', comma + 1) != string::npos)
    {
        return false;
    }

    bid.name = trim(line.substr(0, comma));
    const string value = trim(line.substr(comma + 1));
    if (bid.name.empty() || value.empty())
    {
        return false;
    }

    return parse_cents(value, bid.cents);
}

