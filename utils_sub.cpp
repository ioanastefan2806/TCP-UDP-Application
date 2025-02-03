#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cstring>
#include <arpa/inet.h>

using namespace std;

void print_integer(char* payload) {
    // afiseaza un intreg
    char semn = payload[0];
    unsigned val = ntohl(*(unsigned*)(payload + 1));

    if (semn == 0 || val == 0) {
        fprintf(stdout, "%u\n", val);
    } else {
        fprintf(stdout, "-%u\n", val);
    }
}

void print_short_real(char* payload) {
    uint16_t val = ntohs(*(uint16_t*)payload);
    unsigned int int_part = val / 100;
    unsigned int frac_part = val % 100;

    if (frac_part < 10) {
        fprintf(stdout, "%u.0%u\n", int_part, frac_part);
    } else {
        fprintf(stdout, "%u.%u\n", int_part, frac_part);
    }
}

void print_float(char* payload) {
    u_int8_t semn = payload[0];
    u_int32_t val = ntohl(*(u_int32_t*)(payload + 1));
    u_int8_t putere = payload[5];

    int multiplu = 1;
    for (int i = 0; i < putere; i++) {
        multiplu *= 10;
    }

    float rez = (float)val / multiplu;
    rez = semn == 0 ? rez : -rez;

    fprintf(stdout, "%.4f\n", rez);
}

void print_string(char* payload) {
    fprintf(stdout, "%s\n", payload);
}

string get_type(u_int8_t data_type) {
    // returneaza tipul de date
    if (data_type == 0) {
        return "INT";
    } else if (data_type == 1) {
        return "SHORT_REAL";
    } else if (data_type == 2) {
        return "FLOAT";
    } else if (data_type == 3) {
        return "STRING";
    } else {
        return "INVALID";
    }
}