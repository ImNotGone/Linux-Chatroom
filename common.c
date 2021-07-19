//
// Created by MrGonza on 5/27/2021.
//

#include "common.h"

int check4Err(int value, const char * msg) {
    if (value == SOCKETERR) {
        perror(msg);
        exit(1);
    }
    return value;
}

void trim(char * str, int len) {
    for(int i = 0; i < len; i++) {
        if(str[i] == '\r' || str[i] == '\n') {
            str[i] = '\0';
            return;
        }
    }
}

void printAddr(unsigned int addr) {
    printf("%d.%d.%d.%d", 0xff & addr , 0xff & (addr >> 8), 0xff & (addr >> 16), 0xff & (addr >> 24));
}

