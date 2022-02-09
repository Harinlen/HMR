#ifndef ARGUMENTS_H
#define ARGUMENTS_H

typedef struct Argument {
    char *group = nullptr;
    char *group_output = nullptr;
    char *edge = nullptr;
    char *nodes = nullptr;
    char *output = nullptr;
} Argument;

void parse_arguments(int argc, char *argv[]);

#endif // ARGUMENTS_H
