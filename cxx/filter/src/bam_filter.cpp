#include <algorithm>
#include <cstring>

#include "ui_utils.h"
#include "hp_fasta_types.h"

#include "bam_filter.h"

inline int index_of(char **names, size_t n_ref, const char *name, size_t name_size)
{
    for(size_t i=0; i<n_ref; ++i)
    {
        if(strncmp(name, names[i], name_size) == 0)
        {
            return i;
        }
    }
    return -1;
}

inline int in_range(const ENZYME_RANGE &range, int32_t pos)
{
    if(pos < range.first) { return -1; }
    if(pos > range.second) { return 1; }
    return 0;
}

inline bool in_enzyme_range(const ENZYME_RANGES &range, int32_t pos)
{
    int s = 0, e = range.size(), m;
    while(s < e)
    {
        m = (s + e) >> 1;
        int ret = in_range(range.at(m), pos);
        if(ret == 0) { return true; }
        if(ret == -1) { e = m; }
        else {s = m + 1;}
    }
    return false;
}

void bam_filter_n_ref(uint32_t n_ref, void *user)
{
    BAM_FILTER_USER *filter_data = static_cast<BAM_FILTER_USER *>(user);
    //Prepare the mapper array.
    filter_data->enzyme_idx = static_cast<int32_t *>(malloc(n_ref * sizeof(int32_t)));
    filter_data->total = 0;
}

void bam_filter_ref_info(uint32_t ref_idx, BAM_REF_NAME *ref_name, uint32_t l_ref, void *user)
{
    BAM_FILTER_USER *filter_data = static_cast<BAM_FILTER_USER *>(user);
    //Save the name.
    auto enzyme_info = filter_data->enzyme_info;
    //Find the string in the mapping file.
    filter_data->enzyme_idx[ref_idx] = index_of(enzyme_info->ref_name, enzyme_info->n_ref, ref_name->name, ref_name->l_name);
}

void bam_filter_align(size_t offset, BAM_ALIGN *align, void *user)
{
    BAM_FILTER_USER *filter_data = static_cast<BAM_FILTER_USER *>(user);
    ++filter_data->total;
    auto enzyme_info = filter_data->enzyme_info;
    //Filter the align.
    if(align->mapq < filter_data->mapq)
    {
        return;
    }
    // Check whether the align is in the map or not.
    int filterIdx = filter_data->enzyme_idx[align->refID],
            nextFilterIdx = filter_data->enzyme_idx[align->next_refID];
    if(filterIdx == -1 || nextFilterIdx == -1 ||
            !in_enzyme_range(enzyme_info->ranges[filterIdx], align->pos))
    {
        return;
    }
    //Check whether the record appears in the pending map.
    std::string align_name(align->read_name, align->l_read_name);
    auto pending_iter = filter_data->pending_map.find(align_name);
    if(pending_iter == filter_data->pending_map.end())
    {
        //It does not appear in the map, put it in the map.
        filter_data->pending_map.insert(std::make_pair(align_name, BAM_ALIGN_INFO {offset, align->refID, align->pos, align->next_refID, align->next_pos}));
    }
    else
    {
        //Check whether it is cross reference.
        auto record_info = pending_iter->second;
        if(record_info.refID != -1 &&  // This record is already used.
                record_info.refID == align->next_refID && record_info.next_refID == align->refID && //Cross reference.
                record_info.pos == align->next_pos && record_info.next_pos == align->pos)
        {
            //It is a pair, we pop it out, and send to writing list.
            filter_data->writing_queue.push_back(record_info.offset);
            filter_data->writing_queue.push_back(offset);
            //Assign the reference ID to be -1.
            pending_iter->second.refID = -1;
        }
    }
}
