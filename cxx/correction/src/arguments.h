#ifndef ARGUMENTS_H
#define ARGUMENTS_H

typedef struct Argument {
    char *mapping = nullptr;
    char *reference = nullptr;
    char *output = nullptr;
    float percent = 0.95;
    float sensitive = 0.5;
    int mapq = 1;
    int wide = 25000;
    int narrow = 1000;
    int depletion = 100000;
    int threads = 1;
} Argument;

void parse_arguments(int argc, char *argv[]);

#endif // ARGUMENTS_H
