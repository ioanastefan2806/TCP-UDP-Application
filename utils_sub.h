#ifndef UTILS_SUB_H
#define UTILS_SUB_H

#include <iostream>
#include <string>
#include <vector>

using namespace std;

void print_integer(char* payload);
void print_short_real(char* payload);
void print_float(char* payload);
void print_string(char* payload);
string get_type(u_int8_t data_type);

#endif