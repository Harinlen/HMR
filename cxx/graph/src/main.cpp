#include <cstdlib>
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <list>
#include <vector>
#include <unordered_map>

#include "arguments.h"

#include "ui_utils.h"

extern Argument opts;

typedef struct EDGE_DATA
{
    int32_t start;
    int32_t end;
    uint64_t count;
} EDGE_DATA;

int extract_main()
{
    //Read the node file and build the map.
    FILE *node_file = fopen(opts.nodes, "r");
    size_t node_count=0;
    char node_buf[1024];
    fscanf(node_file, "%zu", &node_count);
    char **node_names;
    size_t *node_length, *node_enzyme;
    node_names = static_cast<char **>(malloc(sizeof(char *) * node_count));
    node_enzyme = static_cast<size_t *>(malloc(sizeof(size_t) * node_count));
    node_length = static_cast<size_t *>(malloc(sizeof(size_t) * node_count));
    for(size_t i=0; i<node_count; ++i)
    {
        //Read the node name.
        fscanf(node_file, "%s %zu %zu", node_buf, &node_enzyme[i], &node_length[i]);
        //Insert the node name to the map.
        size_t node_name_len = strlen(node_buf);
        node_names[i] = static_cast<char *>(malloc(node_name_len + 1));
        strncpy(node_names[i], node_buf, node_name_len);
    }
    fclose(node_file);
    //Read the group bin.
    FILE *group_bin = fopen(opts.group, "rb");
    if(!group_bin)
    {
        time_error_file("Failed to read group binary file %s\n", opts.nodes);
    }
    //Read the binaries.
    uint32_t no_group;
    fread(&no_group, sizeof(uint32_t), 1, group_bin);
    //Ignore the center positions.
    float x, y;
    for(uint32_t i=0; i<no_group; ++i)
    {
        fread(&x, sizeof(float), 1, group_bin);
        fread(&y, sizeof(float), 1, group_bin);
    }
    for(uint32_t i=0; i<no_group; ++i)
    {
        char group_filepath[2048];
        sprintf(group_filepath, "%s.%ug%u.txt", opts.group_output, no_group, i+1);
        FILE *group_file = fopen(group_filepath, "w");
        //Write the header.
        fprintf(group_file, "#Contig\tRECounts\tLength");
        //Read the no of the group.
        uint32_t no_contig, contig_id;
        fread(&no_contig, sizeof(uint32_t), 1, group_bin);
        time_print_counter("Exporting %zu contigs for group %zu", no_contig, i);
        //Read the id, and print the data.
        for(uint32_t j=0; j<no_contig; ++j)
        {
            //Read the contig id.
            fread(&contig_id, sizeof(uint32_t), 1, group_bin);
            //Extract the data from node.
            fprintf(group_file, "\n%s\t%zu\t%zu",
                    node_names[contig_id], node_enzyme[contig_id], node_length[contig_id]);
        }
        fclose(group_file);
    }
    fclose(group_bin);
    //Recover the memory.
    for(size_t i=0; i<node_count; ++i)
    {
        free(node_names[i]);
    }
    free(node_names);
    free(node_enzyme);
    free(node_length);
    return 0;
}

int main(int argc, char *argv[])
{
    parse_arguments(argc, argv);
    //Check whether we are in extract mode.
    if(opts.group != nullptr)
    {
        return extract_main();
    }
    //Read the node file, construct the node map.
    FILE *node_file = fopen(opts.nodes, "r"), *net_file = fopen(opts.output, "w");
    if(!node_file)
    {
        time_error_file("Failed to read node file %s\n", opts.nodes);
    }
    if(!net_file)
    {
        time_error_file("Failed to open network output file %s\n", opts.output);
    }
    //Read the node count.
    size_t node_count;
    fscanf(node_file, "%zu", &node_count);
    //Write the node size.
    fprintf(net_file, "*Vertices %zu\n", node_count);
    //Prepare the node counter buffer.
    size_t *node_counter = static_cast<size_t *>(malloc(node_count * sizeof(size_t))), size_buf;
    char node_name[1024];
    for(size_t i=0; i<node_count; ++i)
    {
        //Read the node name.
        fscanf(node_file, "%s %zu %zu", node_name, &node_counter[i], &size_buf);
        fprintf(net_file, "%zu \"%s\"\n", i+1, node_name);
    }
    fclose(node_file);
    //Write the arcs.
    fprintf(net_file, "*arcs\n");
    //Read the edge.
    FILE *edge_file = fopen(opts.edge, "rb");
    if(!node_file)
    {
        time_error_file("Failed to read edge file %s\n", opts.edge);
    }
    //Read the file bytes while end.
    EDGE_DATA edge_data;
    const auto edge_data_size = sizeof(EDGE_DATA);
    while(fread(&edge_data, edge_data_size, 1, edge_file))
    {
        //Find the edge from the enzyme from the vector.
        double enz_start = static_cast<double>(node_counter[edge_data.start]),
                enz_end = static_cast<double>(node_counter[edge_data.end]),
                hic_count = static_cast<double>(edge_data.count);
        fprintf(net_file, "%d %d %.15lf\n", edge_data.start+1, edge_data.end+1, hic_count / ((enz_start + enz_end) / 2));
    }
    free(node_counter);
    fclose(edge_file);
    fclose(net_file);
    return 0;
}
