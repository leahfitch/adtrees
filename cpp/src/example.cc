#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <map>
#include <ctime>
#include "adtree.h"

using namespace std;
using namespace adtree;

void run_count(ADTree &tree, map<string,string> conditions)
{
    auto c = tree.count(conditions);
    
    for (auto it = conditions.begin(); it != conditions.end(); it++)
    {
        cout << it->first << "=" << it->second << endl;
    }
}

void run_table(ADTree &tree, columns_t columns)
{
    auto rows = tree.table(columns);
    
    for (auto it = columns.begin(); it != columns.end(); it++)
    {
        cout << *it << " ";
    }
    
    cout << endl;
    
    for (auto it = rows.begin(); it != rows.end(); it++)
    {
        for (dimension_t i=0; i<columns.size(); i++)
        {
            cout << (*it)->columns[i] << "\t";
        }
        
        cout << (*it)->count << endl;
    }
    
    tree.delete_rows(rows);
}

int main(int argc, char *argv[])
{
    int num_dimensions = 3;
    string dimensions[] = {
        "A",
        "B",
        "C"
    };
    
    string arr_records[][3] = {
        {"foo", "bar", "baz"},
        {"goo", "gar", "gaz"},
        {"hoo", "har", "haz"},
        {"moo", "mar", "maz"},
        {"foo", "zar", "baz"},
        {"goo", "zar", "gaz"},
        {"hoo", "zar", "haz"},
        {"hoo", "zar", "paz"},
        {"coo", "zar", "maz"}
    };
    
    vector<string*> records (&arr_records[0], &arr_records[8]);
    
    string s;
    
    cout.precision(2);
    ADTree tree(dimensions, num_dimensions, records);
    
    map<string,string> conditions;
    conditions["A"] = "foo";
    run_count(tree, conditions);
    conditions.clear();
    
    conditions["B"] = "zar";
    conditions["C"] = "gaz";
    run_count(tree, conditions);
    conditions.clear();
    
    vector<string> columns;
    columns.push_back("A");
    columns.push_back("B");
    run_table(tree, columns);
    columns.clear();
    
    columns.push_back("A");
    columns.push_back("C");
    run_table(tree, columns);
    columns.clear();
    
    return 0;
}