#ifndef ARGUMENTS_H
#define ARGUMENTS_H

typedef struct Argument {
    char *mapping = nullptr;
    char *reference = nullptr;
    char *output = nullptr;
    char *enzyme = nullptr;
    int mapq = 40;
    int threads = 1;
} Argument;

void parse_arguments(int argc, char *argv[]);

#endif // ARGUMENTS_H
