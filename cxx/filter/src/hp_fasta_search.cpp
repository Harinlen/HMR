#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <list>

#include "arguments.h"
#include "hp_fasta_types.h"
#include "hp_thread_pool.h"

#include "hp_fasta_search.h"

static std::mutex parse_mutex;
static char *seq_name = NULL, *seq_data = NULL;
static size_t seq_name_len, seq_data_len;
extern Argument opts;

typedef struct ENZYME_PARSE
{
    char *seq_name;
    size_t seq_name_length;
    char *seq;
    size_t seq_size;
    FASTA_ENZYME *pos;
} ENZYME_PARSE;

typedef thread_pool<void (const ENZYME_PARSE &), ENZYME_PARSE> WORK_THREADS;

inline bool is_space(char c)
{
    return c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r' ||  c == ' ';
}

inline void trimmed_right(char *buf, size_t &size)
{
    while(size > 0 && is_space(buf[size-1]))
    {
        buf[size--] = '\0';
    }
}

void fasta_dump_count(const char *file_path, FASTA_ENZYME *enzyme_pos)
{
    FILE *count_file = fopen(file_path, "w");
    //Loop and write the file.
    fprintf(count_file, "%zu\n", enzyme_pos->n_ref);
    for(size_t i=0; i<enzyme_pos->n_ref; ++i)
    {
        //Write the name.
        fprintf(count_file, "%s\t%zu\t%zu\n",
                enzyme_pos->ref_name[i],
                enzyme_pos->ranges[i].size(),
                enzyme_pos->ref_length[i]);
    }
    fclose(count_file);
}

void fasta_search_in_seq(const ENZYME_PARSE &param)
{
    std::list<int32_t> poses;
    int32_t enzyme_length = strlen(opts.enzyme);
    char *enzyme_pos = strstr(param.seq, opts.enzyme);
    while(enzyme_pos != NULL)
    {
        poses.push_back(enzyme_pos - param.seq);
        //Search the next pos.
        enzyme_pos = strstr(enzyme_pos + enzyme_length, opts.enzyme);
    }
    //Lock and push the data.
    if(!poses.empty())
    {
        std::unique_lock<std::mutex> parse_lock(parse_mutex);
        //Push the data back.
        param.pos->ref_name = static_cast<char **>(realloc(param.pos->ref_name, sizeof(char *) * (1+param.pos->n_ref)));
        param.pos->ref_name[param.pos->n_ref] = static_cast<char *>(malloc(param.seq_name_length + 1));
        strncpy(param.pos->ref_name[param.pos->n_ref], param.seq_name, param.seq_name_length + 1);
        //Increase the n_ref.
        ++param.pos->n_ref;
        //Construct range
        ENZYME_RANGES pos_range;
        pos_range.reserve(poses.size());
        for(auto i: poses)
        {
            //Check total length.
            if(param.seq_size < 1000) { pos_range.push_back(std::pair<int32_t, int32_t>(0, param.seq_size)); }
            else if(i < 500) { pos_range.push_back(std::pair<int32_t, int32_t>(0, i + enzyme_length + 500)); }
            else if(i + enzyme_length > static_cast<int32_t>(param.seq_size - 500)) { pos_range.push_back(std::make_pair(i - 500, param.seq_size)); }
            else { pos_range.push_back(std::pair<int32_t, int32_t>(i - 500, i + 500)); }
        }
        param.pos->ranges.push_back(pos_range);
        param.pos->ref_length.push_back(param.seq_size);
    }
    //Clear the sequence.
    free(param.seq_name);
    free(param.seq);
}

void fasta_parse_line(char *line, size_t line_size, FASTA_ENZYME *enzyme_pos, WORK_THREADS *work_threads)
{
    //Check whether the line start is with '>'.
    if(line_size == 0 || line == NULL)
    {
        return;
    }
    if(line[0] == '>')
    {
        //Check the last is empty or not.
        if(seq_name != NULL)
        {
            if(seq_data != NULL)
            {
                //Push the data.
                work_threads->push_task(ENZYME_PARSE {seq_name, seq_name_len, seq_data, seq_data_len, enzyme_pos});
            }
            else
            {
                //Clear the sequence name.
                free(seq_name);
            }
        }
        //Set the sequence name.
        seq_name = static_cast<char *>(malloc(line_size));
        seq_name_len = line_size-1;
        strncpy(seq_name, line+1, seq_name_len);
        seq_name[seq_name_len] = '\0';
        //Reset the sequence.
        seq_data = NULL;
        seq_data_len = 0;
    }
    else
    {
        //Check the sequence.
        size_t seq_extend_len = seq_data_len + line_size;
        seq_data = static_cast<char *>(realloc(seq_data, seq_extend_len + 1));
        //Copy the data.
        strncpy(seq_data+seq_data_len, line, line_size);
        seq_data_len = seq_extend_len;
        seq_data[seq_data_len] = '\0';
    }
}

void fasta_search_enzyme(const char *file_path, FASTA_ENZYME *enzyme_pos)
{
    //Keep reading the file.
    FILE *fasta_file = fopen(file_path, "r");
    //Prepare the process pool.
    WORK_THREADS work_pool(fasta_search_in_seq);
    //Loop and process the data.
    enzyme_pos->n_ref = 0;
    enzyme_pos->ref_name = NULL;
    //Prepare the line parser.
    char *line = NULL;
    size_t len = 0;
    while(getline(&line, &len, fasta_file) != -1)
    {
        //Trimmed line.
        trimmed_right(line, len);
        //Yield the line.
        fasta_parse_line(line, len, enzyme_pos, &work_pool);
    }
    if(seq_name != NULL)
    {
        if(seq_data != NULL)
        {
            work_pool.push_task(ENZYME_PARSE {seq_name, seq_name_len, seq_data, seq_data_len, enzyme_pos});
        }
        else
        {
            free(seq_data);
        }
    }
    work_pool.wait_for_tasks();
    fclose(fasta_file);
}
