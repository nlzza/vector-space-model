#pragma once
#ifndef TRIE_HPP
#define TRIE_HPP
#define TOTAL_DOCS (30)

#include "HashTable.hpp"
#include <queue>
#include <vector>
#include <ostream>
#include <numeric>

class Trie
{
public:

    // Constructor and deconstructor
    Trie()
    {
        root = new HashTable;
    }
    ~Trie()
    {
        deleteTrie();
        delete root;
    }
    void deleteTrie();
    HashEntry *search(const std::string& prefix);

    HashEntry *insert(std::string& prefix)
    {
        HashTable *ptr = root;
        HashEntry *target;
        const auto &length = prefix.length();

        for (unsigned i = 0; i < length; i++)
        {
            // insert in current table
            target = ptr->search(prefix[i]);
            if (target == nullptr) // character not found
                target = ptr->insert(prefix[i]);

            if (i == length - 1)
            {
                target->endOfWord = true;
                break;
            }
            // move on to next table
            if (target->next_table == nullptr)
                target->next_table = new HashTable;
            ptr = target->next_table;
        }
        return target;
    }

private:
    HashTable *root{0};
};

// finds the given std::string
// returns nullptr if not found
HashEntry *Trie::search(const std::string &prefix)
{
    HashTable *ptr = root;
    HashEntry *target;
    const auto &length = prefix.length();

    for (unsigned i = 0; i < length; i++)
    {
        // search in current table
        target = ptr->search(prefix[i]);
        if (target == nullptr)
            break;

        if (i == length - 1)
        {
            if (target->endOfWord == true)
                return target;
            break;
        }
        // move on to next table
        ptr = target->next_table;
        if (ptr == nullptr) // if next table does not exist
            break;
    }
    return nullptr;
}

// Deletes the trie
// The method is similar to BFS
void Trie::deleteTrie()
{
    std::queue<HashTable *> q;
    q.push(root);

    while (!q.empty())
    {
        HashTable *f = q.front();
        q.pop();

        for (HashEntry &beg : f->entries)
        {
            if (beg.next_table)
                q.push(beg.next_table);
        }
        delete f;
    }
    root = new HashTable;
}

#endif