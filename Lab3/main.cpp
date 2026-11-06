#include <algorithm>
#include <climits>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "csv.hpp"

using namespace std;

struct Friend
{
    string name;
    vector<double> values;
};

struct DinnerData
{
    vector<string> restaurants;
    vector<Friend> friends;
};

struct Result
{
    size_t chosenRestaurant;
    double socialWelfare;
    vector<double> payments;
    vector<double> gains;
};

int readCSV(istream& input, DinnerData& data)
{
    csv::CSVFormat format;
    format.delimiter(',')
          .header_row(0)
          .trim({' ', '\t'})
          .variable_columns(csv::VariableColumnPolicy::THROW);

    csv::CSVError error;
    auto reader = csv::CSVReader::try_open(input, error, format);
    if (!reader)
        return 1;

    vector<string> header = reader->get_col_names();
    if (header.size() < 2 || header[0] != "friend")
    {
        //at least one restaurant and the first column must be "friend"
        return 1;
    }

    for (size_t j = 1; j < header.size(); j++)
    {
        if (header[j].empty())
        {
            //no empty headers
            return 1;
        }
            
        if (find(data.restaurants.begin(), data.restaurants.end(), header[j]) != data.restaurants.end())
        {
            //no duplicate headers
            return 1;
        }

        data.restaurants.push_back(header[j]);
    }

    csv::CSVRow row;
    while (reader->try_read_row(row, error))
    {
        if (row.size() != header.size())
        {
            //row length mismatch
            return 1;
        }

        Friend f;
        f.name = row[0].get<string>();
        if (f.name.empty())
        {
            //friend name cannot be empty
            return 1;
        }

        for (size_t j = 1; j < header.size(); j++)
        {
            istringstream value(row[j].get<string>());
            double number;

            if (!(value >> number) || !value.eof() || !isfinite(number))
            {
                //number not double or not finite (NaN or Inf)
                return 1;
            }

            f.values.push_back(number);
        }
        data.friends.push_back(f);
    }

    return error || data.friends.empty();
}

vector<double> calculateTotals(const DinnerData& data)
{
    vector<double> totals(data.restaurants.size(), 0.0);

    for (const Friend& f : data.friends)
    {
        for (size_t j = 0; j < totals.size(); j++)
            totals[j] += f.values[j];
    }
    return totals;
}

size_t chooseRestaurant(const vector<double>& totals) {
    size_t best = 0;
    for (size_t j = 1; j < totals.size(); j++) {
        if (totals[j] > totals[best]) best = j;
    }
    return best;
}

Result calculateVCG(const DinnerData& data)
{
    const vector<double> totals = calculateTotals(data);
    const size_t chosen = chooseRestaurant(totals);

    Result result{chosen, totals[chosen], {}, {}};

    for (const Friend& f : data.friends)
    {
        double bestForOthers = totals[0] - f.values[0];

        for (size_t j = 1; j < totals.size(); j++)
        {
            double othersWelfare = totals[j] - f.values[j];
            bestForOthers = max(bestForOthers, othersWelfare);
        }

        const double othersAtChosen = totals[chosen] - f.values[chosen];

        double payment = bestForOthers - othersAtChosen;
        if (payment < 0 && fabs(payment) < 1e-12) //0.000000000001
            payment = 0;

        result.payments.push_back(payment);
        result.gains.push_back(f.values[chosen] - payment);
    }
    return result;
}

void printResult(const DinnerData& data, const Result& result)
{
    cout << setprecision(12);
    cout << "Chosen restaurant: " << data.restaurants[result.chosenRestaurant] << '\n';
    cout << "Total social welfare: " << result.socialWelfare << '\n';
    cout << "\nFriend | Payment | Gain\n";

    double totalPayments = 0;
    for (size_t i = 0; i < data.friends.size(); i++)
    {
        cout << data.friends[i].name << " | " << result.payments[i] << " | " << result.gains[i] << '\n';
        totalPayments += result.payments[i];
    }
    cout << "Total payments: " << totalPayments << '\n';
}

int main(int argc, char* argv[])
{
    if (argc > 2) return 1;
    
    DinnerData data;
    if (argc == 2)
    {
        ifstream file(argv[1]);
        if (!file || readCSV(file, data) != 0)
            return 1;
    }
    else
    {
        if (readCSV(cin, data) != 0)
            return 1;
    }

    printResult(data, calculateVCG(data));
    
    return 0;
}
