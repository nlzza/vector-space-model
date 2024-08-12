#include <iostream>
#include <vector>
#include "Indexer/Indexer.hpp"
using namespace std;

int main()
{
    Indexer indexer;
    indexer.read("doc_vectors.txt");

    cout << "Enter a query: ";
    string query;
    getline(cin, query);
    
    auto result = indexer.query_eval(query);
    cout << "\nResult(s): ";
    for (const auto& i: result)
        cout << i.first << " ";
    cout << endl;
    return 0;
}
