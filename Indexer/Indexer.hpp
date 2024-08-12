#pragma once
#ifndef INDEXER_HPP
#define INDEXER_HPP
#define TOTAL_DOCS (30)
#define ALPHA (0.05) // The value of 0.05 given in assignment yields too few results!
                       // while values as low as 0.01 give too many.
                       // After trial and error, I found 0.0275 to be a nice balance.
#include "Tries/Trie.hpp"
#include <cmath>
#include <fstream>
#include <algorithm>
#include <string>
#include <vector>
#include <map>
#include <iostream>

class Indexer
{
    std::vector<std::string> stopwords;

private:
    char c1;
    unsigned pos{0};
    HashEntry *target{0};

    Trie dictionary;
    std::map<std::string, long double> doc_vectors[TOTAL_DOCS];

    bool binary_search(std::vector<std::string>::iterator it, const int size, const std::string& s)
    {
        if (it[size/2] == s)
            return true;
        else if (size == 1)
            return false;
        else if (it[size/2] > s)
            return binary_search(it, size/2, s);
        else
            return binary_search(it + size/2, (size/2) + (size % 2), s);
    }

    // Returns true if the word is a stopword; else returns false
    bool is_stopword(const std::string& word)
    {
        return binary_search(stopwords.begin(), stopwords.size(), word);
    }

    // Reads stopwords from "Stopword-List.txt" into the vector
    void load_stopwords()
    {
        std::ifstream file;
        file.open("../Stopword-List.txt", std::ios::in);
        std::string s;
        while (getline(file, s))
            stopwords.push_back(s);
        file.close();
        sort(stopwords.begin(), stopwords.end());
    }

    std::string& stem(std::string& word)
    {
        // Prefix removal (tested on "ultraaggressive")
        std::string prefixes[] = {"kilo", "micro", "milli", "intra", "ultra",
                                  "mega", "nano", "pico", "pseudo"}; // Only those given in slides are included
        int wordlen = word.length();
        for (short i = 0; i < 9; i++)
        {
            const int len = prefixes[i].length();
            if (wordlen <= len)
                continue;
            if (prefixes[i] == word.substr(0, len))
            {
                word.erase(0, len);
                break;
            }
        }
        // Suffix removal step 1
        // Remove "es" from words ending in "sses" or "ies" (tested on "dismisses")
        wordlen = word.length();
        if ((wordlen > 4 && word.substr(wordlen - 4, 4) == "sses") ||
            (wordlen > 3 && word.substr(wordlen - 3, 3) == "ies"))
            word.erase(wordlen - 2, 2);
        
        // Remove last "s" from words ending in s whose second last letter is not s (tested on "runs")
        wordlen = word.length();
        if (word[wordlen - 1] == 's' && word[wordlen - 2] != 's')
            word.erase(wordlen - 1, 1);
        
        // If word has a vowel and ends in "eed", remove "ed" (tested on "agreed")
        wordlen = word.length();
        if (wordlen > 3)
        {
            bool has_vowel = false;
            for (short i = 0; i < wordlen - 3; i++)
            {
                if (word[i] == 'a' || word[i] == 'e' || word[i] == 'i' || word[i] == 'o' || word[i] == 'u')
                {
                    has_vowel = true;
                    break;
                }
            }
            if (has_vowel && word.substr(wordlen - 3, 3) == "eed")
                word.erase(wordlen - 2, 2);
        }

        // If word has a vowel and ends in "ed", remove "d" (tested on "chased")
        wordlen = word.length();
        if (wordlen > 2)
        {
            bool has_vowel = false;
            for (short i = 0; i < wordlen - 2; i++)
            {
                if (word[i] == 'a' || word[i] == 'e' || word[i] == 'i' || word[i] == 'o' || word[i] == 'u')
                {
                    has_vowel = true;
                    break;
                }
            }
            if (has_vowel && word.substr(wordlen - 2, 2) == "ed")
                word.erase(wordlen - 1, 1);
        }

        // Replace trailing y with an i (tested on "briefly")
        wordlen = word.length();
        if (wordlen > 1 && word[wordlen - 1] == 'y')
            word[wordlen - 1] = 'i';

        // Suffix removal step 2
        if (wordlen > 6 && word.substr(wordlen - 6, 6) == "tional")
            word.erase(wordlen - 2, 2);
        else if (wordlen > 7 && word.substr(wordlen - 7, 7) == "ization")
        {
            word.erase(wordlen - 5, 5);
            word.push_back('e');
        }
        else if (wordlen > 4 && word.substr(wordlen - 4, 4) == "ness")
            word.erase(wordlen - 4, 4);
        else if (wordlen > 5 && word.substr(wordlen - 5, 5) == "ousli")
            word.erase(wordlen - 2, 2);
        else if (wordlen > 5 && word.substr(wordlen - 5, 5) == "entli")
            word.erase(wordlen - 2, 2);
        else if (wordlen > 6 && word.substr(wordlen - 6, 6) == "biliti")
        {
            word.erase(wordlen - 5, 5);
            word.append("le");
        }

        // Suffix removal step 3
        wordlen = word.length();
        if (wordlen > 5 && word.substr(wordlen - 5, 5) == "icate")
            word.erase(wordlen - 3, 3);
        else if (wordlen > 5 && word.substr(wordlen - 5, 5) == "ative")
            word.erase(wordlen - 5, 5);
        else if (wordlen > 5 && word.substr(wordlen - 5, 5) == "alize")
            word.erase(wordlen - 3, 3);
        else if (wordlen > 5 && word.substr(wordlen - 5, 5) == "iciti")
            word.erase(wordlen - 3, 3);
        else if (wordlen > 4 && word.substr(wordlen - 4, 4) == "ical")
            word.erase(wordlen - 2, 2);
        else if (wordlen > 3 && word.substr(wordlen - 3, 3) == "ful")
            word.erase(wordlen - 3, 3);
        
        // Suffix removal step 4 (removing standard suffixes)
        std::string suffixes[] = {"al", "ance", "ence", "er", "ic", "able",
                                  "ible", "ant", "ement", "ment", "ent",
                                  "sion", "tion", "ou", "ism", "ate", "iti",
                                  "ous", "ive", "ize", "ise"}; // Only those given in slides are included
        wordlen = word.length();
        for (short i = 0; i < 21; i++)
        {
            const int len = suffixes[i].length();
            if (wordlen <= len)
                continue;
            if (suffixes[i] == word.substr(wordlen - len, len))
            {
                word.erase(wordlen - len, len);
                break;
            }
        }
        // Suffix removel step 5 (remove trailing e)
        wordlen = word.length();
        if (word[wordlen - 1] == 'e')
            word.erase(wordlen - 1, 1);

        return word;
    }

public:

    // Constructor
    Indexer()
    {
        load_stopwords();
    }

    void index(const char *filename, const unsigned &doc_ID = 0)
    {
        std::string word;
        pos = 0;
        FILE *file = fopen(filename, "r");
        while ((c1 = fgetc(file)) != EOF)
        {
            if ((c1 >= 'a' && c1 <= 'z') || (c1 >= '0' && c1 <= '9'))
                word.push_back(c1);
            else if (c1 >= 'A' && c1 <= 'Z')
            {
                c1 |= 32; // case folding
                word.push_back(c1);
            }
            else if (c1 == ' ' || c1 == '\n')
            {
                if (word.length() && !is_stopword(word))
                {
                    std::string w = stem(word);
                    doc_vectors[doc_ID - 1][w] = 1; // 1 is a placeholder. The TF-IDF will be calaulated later
                    target = dictionary.insert(w);
                    if (target->posting)
                        target->posting->push_directly(doc_ID, pos);
                    else
                        target->posting = new Posting(doc_ID, pos);
                }
                word.clear();
                pos++;
            }
        }
        // Dont forget to index the last word!
        if (word.length() && !is_stopword(word))
        {
            std::string w = stem(word);
            doc_vectors[doc_ID - 1][w] = 1;  // 1 is a placeholder. The TF-IDF will be calaulated later
            target = dictionary.insert(w);
            if (target->posting)
                target->posting->push_directly(doc_ID, pos);
            else
                target->posting = new Posting(doc_ID, pos);
        }
        fclose(file);
    }

    void make_doc_vectors(void)
    {
        // Replace the placeholder 1s by actual TF-IDF values
        for (int i = 0; i < TOTAL_DOCS; i++)
        {
            long double magnitude = 0;
            for (auto& p: doc_vectors[i])
            {
                std::string word = p.first;
                HashEntry* ptr = dictionary.search(word);
                unsigned doc_count = ptr->posting->doc_count;
                Node<Document>* doc_ptr = ptr->posting->documents.begin();
                unsigned tf;

                while (doc_ptr)
                {
                    if (doc_ptr->data.ID == i + 1)
                    {
                        tf = doc_ptr->data.term_freq;
                        break;
                    }
                    doc_ptr = doc_ptr->next;
                }
                p.second = log10(tf + 1) * log10(TOTAL_DOCS / (double)doc_count);
                magnitude += p.second * p.second;
            }
            // Normalising the vector
            long double divisor = sqrt(magnitude);
            for (auto& p: doc_vectors[i])
                p.second /= divisor;
            std::cout << doc_vectors[0].size() << std::endl;
        }
    }

    // Store the doc vectors in a file
    void write_on(const char *filename)
    {
        std::ofstream file;
        file.open(filename, std::ios::out);

        for (int i = 0; i < TOTAL_DOCS; i++)
        {
            file << i + 1 << " " << doc_vectors[i].size() << " ";
            for (auto& p: doc_vectors[i])
                file << p.first << " " << p.second << " ";
            file << "\n";
        }
        file.close();
    }

    void read(const char *filename)
    {
        int doc, count;
        std::string word;
        long double tf_idf;

        std::ifstream file;
        file.open(filename, std::ios::in);
        while (!file.eof())
        {
            file >> doc >> count;
            for (int i = 0; i < count; i++)
            {
                file >> word >> tf_idf;
                doc_vectors[doc - 1][word] = tf_idf;
            }
        }
        file.close();
    }

    std::vector<std::pair<unsigned, long double>> query_eval(std::string query)
    {
        // Vector is the result containg IDs of all docs that satisfy query
        // Results are ranked on basis of cosine similarity

        long double results[TOTAL_DOCS]; // For all docs, stores similarity b/w doc and query
        std::vector<std::string> query_terms;
        std::string word;

        for (char& c: query)
        {
            if (c >= 'a' && c <= 'z' || c >= '0' && c <= '9')
                word.push_back(c);
            else if (c >= 'A' && c <= 'Z')
            {
                c |= 32;
                word.push_back(c);
            }
            else
            {
                if (word.length() && !is_stopword(word))
                    query_terms.push_back(stem(word));
                word.clear();
            }
        }
        if (word.length() && !is_stopword(word))
            query_terms.push_back(stem(word));
        
        for (int i = 0; i < TOTAL_DOCS; i++)
        {
            results[i] = 0;
            for (const std::string& term: query_terms)
            {
                if (doc_vectors[i].find(term) == doc_vectors[i].end())
                    continue;
                results[i] += doc_vectors[i][term];
            }
        }
        std::vector<std::pair<unsigned, long double>> res;

        for (int i = 0; i < TOTAL_DOCS; i++)
        {
            if (results[i] > ALPHA)
                res.push_back(std::pair<unsigned, long double> (i + 1, results[i]));
        }
        sort(res.begin(), res.end(), [](std::pair<unsigned, long double>& p1,
                                        std::pair<unsigned, long double>& p2)
                                        { return p1.second > p2.second; });
        return res;
    }
};
#endif