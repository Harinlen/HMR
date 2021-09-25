#include <cstring>
#include <string>
#include <unordered_map>
#include <cstdio>

#include "enzyme.h"
#include "ui_utils.h"

std::unordered_map<std::string, const char *> known_enzyme_alias {
    {std::string("HINDIII"), "AAGCTT"},
    {std::string("HIND3"), "AAGCTT"},
    {std::string("NCOI"), "CCATGG"},
    {std::string("DPN1"), "GATC"},
    {std::string("MBOI"), "GATC"},
};

void invalid_enzyme(char invalid_c)
{
    char buf[1024];
    sprintf(buf, "Invalid nucleotide base '%c' found in enzyme.", invalid_c);
    time_print(buf);
    time_error(-1, "Make sure your enzyme should be only composed of 'A', 'C', 'T' and 'G'.");
}

void check_enzyme(char *enzyme, const char **nuc_seq, size_t *nuc_seq_size)
{
    size_t length = strlen(enzyme);
    //Convert the original char in upper case letter.
    for(char *s=enzyme, *e = enzyme + length; s<e; ++s)
    {
        if((*s) >= 'a' && (*s) <= 'z') {
            (*s) = 'A' + ((*s) - 'a');
        }
    }
    //Check whether it is an known name.
    auto known_finder = known_enzyme_alias.find(std::string(enzyme));
    if(known_finder != known_enzyme_alias.end())
    {
        (*nuc_seq) = known_finder->second;
        (*nuc_seq_size) = strlen(*nuc_seq);
        return;
    }
    //Or else we have to check the enzyme.
    time_print_file("\tEnzyme: %s", enzyme);
    for(char *s = enzyme, *e = enzyme + length; s<e; ++s)
    {
        //Check invalid nuc.
        if((*s) != 'A' && (*s) != 'C' && (*s) != 'T' && (*s) != 'G')
        {
            invalid_enzyme(*s);
        }
    }
    return;
}

