#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cstring>
#include <string.h>
#include <unordered_map>
#include <stack>
#include <sstream>

using namespace std;

// functie care imparte un string dupa un delimitator
vector<string> split(string &str, char delimiter) {
    vector<std::string> tokens;
    string token;
    
    // folosim un istringstream pentru a imparti stringul
    istringstream stream_str(str);

    // impartim stringul dupa delimitator
    while (getline(stream_str, token, delimiter)) {
        tokens.push_back(token);
    }

    return tokens;
}

bool topic_inclusion(string topic_specific, string topic_template) {
    // verificam daca un topic specific este inclus in topicul template (daca se potrivesc)
    vector<string> spc = split(topic_specific, '/');
    vector<string> tpl = split(topic_template, '/');
    size_t spc_idx = 0;
    size_t tpl_idx = 0;

    // folosim o stiva pentru a retine pozitiile in care am gasit un *
    stack<size_t> stiva;

    // parcurgem cele doua topicuri
    while (spc_idx < spc.size()) {
        if (tpl_idx < tpl.size() && (tpl[tpl_idx] == "+" || tpl[tpl_idx] == spc[spc_idx])) {
            // daca gasim un + sau daca se potrivesc, continuam
            tpl_idx++;
            spc_idx++;
        } else if (tpl_idx < tpl.size() && tpl[tpl_idx] == "*") {
            // daca gasim un *, retinem pozitia in care am gasit *
            stiva.push(tpl_idx);
            stiva.push(spc_idx);
            tpl_idx++;
        } else if (!stiva.empty()) {
            // daca am gasit un * inainte, incercam sa continuam de acolo
            spc_idx = stiva.top() + 1;
            stiva.pop();
            tpl_idx = stiva.top();
        } else {
            // daca nu am gasit un * inainte, inseamna ca nu se potrivesc
            return false;
        }
    }

    while (tpl_idx < tpl.size() && tpl[tpl_idx] == "*") {
        tpl_idx++;
    }

    return tpl_idx == tpl.size();
}

bool user_exists(int id, unordered_map<int, int> users) {
    // verificam daca un user exista in lista de useri
    for (auto user : users) {
        if (user.second == id) {
            return true;
        }
    }
    return false;
}