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
        "usage: %s [-h] -m MAPPING -r REFERENCE -o OUTPUT [-p PERCENT] [-s SENSITIVE] [-q MAPQ] [-w WIDE] [-n NARROW] [-d DEPLETION] [-t THREADS]\n\n"
        "optional arguments:\n"
        "  -h, --help            show this help message and exit\n"
        "  -m MAPPING, --mapping MAPPING\n"
        "                        Input mapping file (.bam)\n"
        "  -r REFERENCE, --reference REFERENCE\n"
        "                        Contig fasta file (.fasta)\n"
        "  -o OUTPUT, --output OUTPUT\n"
        "                        Corrected fasta file (.fasta)\n"
        "  -p PERCENT, --percent PERCENT\n"
        "                        Percent of the map to saturate (default: 0.95)\n"
        "  -s SENSITIVE, --sensitive SENSITIVE\n"
        "                        sensitivity to depletion score (default: 0.5)\n"
        "  -q MAPQ, --mapq MAPQ  MAPQ of mapping lower bound (default: 1)\n"
        "  -w WIDE, --wide WIDE  Resolution for first pass search of mismatches (default: 25000)\n"
        "  -n NARROW, --narrow NARROW\n"
        "                        Resolution for the precise mismatch localizaton, n<w (default: 1000)\n"
        "  -d DEPLETION, --depletion DEPLETION\n"
        "                        The size of the region to aggregate the depletion score in the wide path, d >= 2*w, (default: 100000)\n"
        "  -t THREADS, --threads THREADS\n"
        "                        Number of threads (default: 1)\n", prog_name);
    exit(0);
}

void parse_arguments(int argc, char *argv[])
{
    //Parse the argc.
    for(int i=1; i<argc; i+=2)
    {
        // Check the help argument.
        if(stris("-h", argv[i])) { help_exit(argv[0]); }
        // Check the arguments.
        if(stris("-m", argv[i]) || stris("--mapping", argv[i])) opts.mapping = argv[i+1];
        else if(stris("-r", argv[i]) || stris("--reference", argv[i])) opts.reference = argv[i+1];
        else if(stris("-o", argv[i]) || stris("--output", argv[i])) opts.output = argv[i+1];
        else if(stris("-p", argv[i]) || stris("--percent", argv[i])) opts.percent = atof(argv[i+1]);
        else if(stris("-s", argv[i]) || stris("--sensitive", argv[i])) opts.sensitive = atof(argv[i+1]);
        else if(stris("-q", argv[i]) || stris("--mapq", argv[i])) opts.mapq = atoi(argv[i+1]);
        else if(stris("-w", argv[i]) || stris("--wide", argv[i])) opts.wide = atoi(argv[i+1]);
        else if(stris("-n", argv[i]) || stris("--narrow", argv[i])) opts.narrow = atoi(argv[i+1]);
        else if(stris("-d", argv[i]) || stris("--depletion", argv[i])) opts.depletion = atoi(argv[i+1]);
        else if(stris("-t", argv[i]) || stris("--threads", argv[i])) opts.threads = atoi(argv[i+1]);
    }
    //Check the error.
    if(opts.mapping == nullptr) { printf("No mapping file found.\n"); help_exit(argv[0]); }
    if(opts.reference == nullptr) { printf("No reference file found.\n"); help_exit(argv[0]); }
    if(opts.output == nullptr) { printf("No output file assigned.\n"); help_exit(argv[0]); }
    //Print the data.
    time_print("Execution configuration:");
    time_print_int("\tThreads: %d", opts.threads);
    time_print_int("\tNarrow size: %d", opts.narrow);
    time_print_int("\tWide size: %d", opts.wide);
    time_print_int("\tDepletion size: %d", opts.depletion);
}
