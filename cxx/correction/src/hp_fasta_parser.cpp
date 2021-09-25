#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <list>

#include "hp_str_utils.h"
#include "ui_utils.h"

#include "hp_fasta_parser.h"

SEQ_DATA fasta_combine_seq(const std::list<SEQ_SLICE> &seq_list, size_t seq_length)
{
    SEQ_DATA seq_data;
    seq_data.seq_len = seq_length;
    seq_data.seq = static_cast<char *>(malloc(seq_length));
    for(auto iter: seq_list)
    {
        //Copy the data.
        memcpy(seq_data.seq + iter.offset, iter.seq, iter.seq_len);
    }
    return seq_data;
}

void fasta_load_file(const char *file_path, FASTA_MAP *map)
{
    //Keep reading the file.
    FILE *fasta_file = fopen(file_path, "r");
    if(!fasta_file)
    {
        time_error_file("Failed to open FASTA file %s", file_path);
    }
    time_print_file("Loading FASTA file %s", file_path);
    //Prepare the record.
    char *seq_name = NULL;
    size_t seq_name_len = 0, seq_length = 0;
    std::list<SEQ_SLICE> seq_data;
    //Now just read the file line by line.
    char *line = NULL;
    size_t len = 0;
    ssize_t line_size;
    while((line_size = getline(&line, &len, fasta_file)) != -1)
    {
        //Trim the right of the string.
        trimmed_right(line, line_size);
        line_size = strlen(line);
        if(line_size == 0) { continue; }
        if(line[0] == '>')
        {
            //Check the previous data is valid or not.
            if(seq_name != NULL)
            {
                //Combine the sequence data.
                map->names.push_back(SEQ_NAME {std::string(seq_name, seq_name_len), map->seqs.size()});
                map->seqs.push_back(fasta_combine_seq(seq_data, seq_length));
                //Reset the values.
                free(seq_name);
                for(auto iter: seq_data)
                {
                    free(iter.seq);
                }
                //Reset the sequence length for next seq.
                seq_length = 0;
            }
            //Assign the line to be new sequence name.
            --line_size;
            seq_name = static_cast<char *>(malloc(line_size + 1));
            strncpy(seq_name, line+1, line_size);
            seq_name[line_size] = '\0';
            seq_name_len = line_size;
            //Reset the seq data.
            seq_data = std::list<SEQ_SLICE>();
        }
        else
        {
            char *line_copy = static_cast<char *>(malloc(line_size));
            strncpy(line_copy, line, line_size);
            //Push the sequence data.
            seq_data.push_back(SEQ_SLICE {line_copy, static_cast<size_t>(line_size), seq_length});
            //Increase the sequence length.
            seq_length += line_size;
        }
    }
    //Append the last sequence to the map.
    if(seq_name != NULL)
    {
        //Combine the sequence data.
        map->names.push_back(SEQ_NAME {std::string(seq_name, seq_name_len), map->seqs.size()});
        map->seqs.push_back(fasta_combine_seq(seq_data, seq_length));
        free(seq_name);
        //Clear sequence data.
        for(auto iter: seq_data)
        {
            free(iter.seq);
        }
    }
    fclose(fasta_file);
    time_print_file("Complete loading FASTA file %s", file_path);
}
