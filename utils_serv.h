#ifndef UTILS_SERV_H
#define UTILS_SERV_H

#include <string>
#include <vector>
#include <algorithm>
#include <unordered_map>

using namespace std;

bool topic_inclusion(string topic_specific, string topic_template);
bool user_exists(int id, unordered_map<int, int> users);

#endif