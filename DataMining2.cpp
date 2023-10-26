#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <algorithm>
#include <sstream>


using std::cout; using std::set; using std::string;
using std::vector; using std::ifstream; using std::stringstream;
using std::map; using std::cerr; using std::endl; using std::stod;

// Define a class for itemsets and their counts
class Itemset {
public:
    set<string> items;
    int count;

    bool operator<(const Itemset& other) const {
        return items < other.items;
    }
};

// Function to read transactions from a file
vector<set<string>> readTransactions(const string& filename) {
    vector<set<string>> transactions;
    ifstream file(filename);
    string line;

    while (getline(file, line)) {
        set<string> items;
        string item;
        stringstream ss(line);

        while (ss >> item) {
            items.insert(item);
        }

        transactions.push_back(items);
    }

    return transactions;
}


// Function to generate pairs of amount 1
map<Itemset, int> generateFrequent1Itemsets(const vector<set<string>>& transactions, double minSupport) {
    map<Itemset, int> frequentItemsets;

    // Count item occurrences in transactions
    map<string, int> itemCounts;
    for (const set<string>& transaction : transactions) {
        for (const string& item : transaction) {
            itemCounts[item]++;
        }
    }

    // Determine frequent 1-itemsets
    for (const auto& pair : itemCounts) {
        if (static_cast<double>(pair.second) / transactions.size() >= minSupport) {
            Itemset itemset;
            itemset.items.insert(pair.first);
            itemset.count = pair.second;
            frequentItemsets[itemset] = itemset.count;
        }
    }

    return frequentItemsets;
}

// Function to generate pairs of amount k
map<Itemset, int> generateCandidateItemsets(const map<Itemset, int>& prevFreqItemsets, double minSupport) {
    map<Itemset, int> candidateItemsets;

    // Iterate through pairs of frequent itemsets to generate candidates
    for (auto it1 = prevFreqItemsets.begin(); it1 != prevFreqItemsets.end(); ++it1) {
        for (auto it2 = next(it1); it2 != prevFreqItemsets.end(); ++it2) {
            const Itemset& itemset1 = it1->first;
            const Itemset& itemset2 = it2->first;

            // Merge itemset1 and itemset2
            Itemset candidate = itemset1;
            candidate.items.insert(itemset2.items.begin(), itemset2.items.end());

            // Prune candidate itemsets
            bool shouldRemove = false;

            set<Itemset> subsets;
            for (const string& item : candidate.items) {
                Itemset subset = candidate;
                subset.items.erase(item);
                subsets.insert(subset);
            }

            for (const Itemset& subset : subsets) {
                if (prevFreqItemsets.find(subset) == prevFreqItemsets.end()) {
                    shouldRemove = true;
                    break;
                }
            }

            if (!shouldRemove) {
                candidateItemsets[candidate] = 0;
            }
        }
    }

    return candidateItemsets;
}

// Function to count candidate occurrences in transactions
void countCandidateOccurrences(const vector<set<string>>& transactions, map<Itemset, int>& candidates) {
    for (const set<string>& transaction : transactions) {
        for (auto& pair : candidates) {
            const Itemset& candidate = pair.first;
            bool allItemsFound = true;

            // Check if all items in the candidate are present in the transaction
            for (const string& item : candidate.items) {
                if (transaction.find(item) == transaction.end()) {
                    allItemsFound = false;
                    break;
                }
            }
            if (allItemsFound) {
                pair.second++;
            }
        }
    }
}

// Determine frequent k-itemsets
map<Itemset, int> getFrequentKItemsets(const vector<set<string>>& transactions, const map<Itemset, int>& candidateItemsets, double minSupport) {
    map<Itemset, int> frequentItemsets;

    for (const auto& pair : candidateItemsets) {
        double support = static_cast<double>(pair.second) / transactions.size();
        if (support >= minSupport) {
            frequentItemsets[pair.first] = pair.second;
        }
    }

    return frequentItemsets;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        return 1;
    }

    string filename = argv[1];
    double minSupport = stod(argv[2]) / 100.0; // Convert minSupport to a percentage

    // Read transactions from the input file
    vector<set<string>> transactions = readTransactions(filename);

    int k = 1;
    map<Itemset, int> prevFreqItemsets;
    map<Itemset, int> frequentItemsets;

    do {
        // Generate candidates and count their occurrences
        map<Itemset, int> candidateItemsets;
        if (k == 1) {
            // For k = 1, generate and count 1-itemsets differently
            candidateItemsets = generateFrequent1Itemsets(transactions, minSupport);
        }
        else {
            candidateItemsets = generateCandidateItemsets(prevFreqItemsets, minSupport);
            countCandidateOccurrences(transactions, candidateItemsets);
        }

        // Get frequent k-itemsets
        frequentItemsets = getFrequentKItemsets(transactions, candidateItemsets, minSupport);


        // Output frequent k-itemsets
        cout << "Frequent " << k << "-Itemsets:" << endl;
        for (const auto& pair : frequentItemsets) {
            const Itemset& itemset = pair.first;
            const int count = pair.second;
            for (const string& item : itemset.items) {
                cout << item << " ";
            }
            cout << "(Count: " << count << ")" << endl;
        }

        prevFreqItemsets = frequentItemsets;
        k++;

    } while (!frequentItemsets.empty());
    
    
    return 0;
}
