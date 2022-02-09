#include <cstdio>
#include <cstdlib>

#include "arguments.h"
#include "ui_utils.h"

Argument opts;

bool stris(const char *sl, const char *s2)
{
    //Simple and fast compare.
    for(; *sl==*s2; ++sl, ++s2)
    {
        if(*sl=='\0')
        {
            return true;
        }
    }
    return false;
}

void time_print_int(const char *log, int data)
{
    char buf[1024];
    sprintf(buf, log, data);
    time_print(buf);
}

void help_exit(char *prog_name)
{
    printf(
        "usage: %s [-h] -n NODES -u GROUP_BIN -g GROUP -e EDGES -o OUTPUT\n\n"
        "optional arguments:\n"
        "  -h, --help            show this help message and exit\n"
        "  -n NODES, --node NODES\n"
        "                        Contig enzyme count file (.enz_count).\n"
        "  -u GROUP_BIN, --unpack GROUP_BIN\n"
        "                        Group binary file (.txt.bin). Mutually exclusive with -e and -o.\n"
        "  -g GROUP, --group GROUP\n"
        "                        Group text file. Mutually exclusive with -e and -o.\n"
        "  -e EDGES, --edge EDGES\n"
        "                        Contig HiC edge file (.edge). Mutually exclusive with -u and -g.\n"
        "  -o OUTPUT, --output OUTPUT\n"
        "                        Pajek NET graph file (.net). Mutually exclusive with -u and -g.\n", prog_name);
    exit(1);
}

void parse_arguments(int argc, char *argv[])
{
    //Parse the argc.
    for(int i=1; i<argc; i+=2)
    {
        // Check the help argument.
        if(stris("-h", argv[i])) { help_exit(argv[0]); }
        // Check the arguments.
        if(stris("-n", argv[i]) || stris("--node", argv[i])) opts.nodes = argv[i+1];
        else if(stris("-e", argv[i]) || stris("--edge", argv[i])) opts.edge = argv[i+1];
        else if(stris("-u", argv[i]) || stris("--unpack", argv[i])) opts.group = argv[i+1];
        else if(stris("-g", argv[i]) || stris("--group", argv[i])) opts.group_output = argv[i+1];
        else if(stris("-o", argv[i]) || stris("--output", argv[i])) opts.output = argv[i+1];
    }
    //Check the group or nodes & edges are nullptr.
    if(opts.nodes == nullptr) { printf("No enzyme count file found.\n"); help_exit(argv[0]); }
    if(opts.group == nullptr)
    {
        //Check the nodes and edges.
        if(opts.nodes == nullptr && opts.edge == nullptr && opts.output == nullptr)
        {
            //All the settings are empty, show the message.
            printf("Group file or nodes/edges files are required.\n"); help_exit(argv[0]);
        }
        //Check whether the nodes and edges are nullptr.
        if(opts.edge == nullptr) { printf("No HiC edge file found.\n"); help_exit(argv[0]); }
        if(opts.output == nullptr) { printf("No output file assigned.\n"); help_exit(argv[0]); }
    }
    else
    {
        //Has group file, check group extract files.
        if(opts.group_output == nullptr) { printf("No group output file specified.\n"); help_exit(argv[0]); }
    }
}
