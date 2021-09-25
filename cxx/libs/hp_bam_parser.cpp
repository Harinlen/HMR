#include <algorithm>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cmath>
#include <list>
#include <vector>
#include <unordered_map>

#include "hp_bgzf_queue.h"
#include "ui_utils.h"

#include "hp_bam_parser.h"

void bam_initial_fsm(BAM_PARSE_FSM *fsm)
{
    //Clear the reference.
    fsm->n_ref = 0;
    fsm->ref_idx = 0;
    //Clear the residuals.
    fsm->r = NULL;
    fsm->r_size = 0;
    fsm->r_reserve = 0;
    fsm->offset = 0;
    //Reset the state.
    fsm->state = BAM_FSM_FIND_HEADER;
}

void bam_parse_align(char *data, size_t data_size, BAM_PARSE_FSM *fsm,
                     BAM_HEADER_CALLBACK header_callback,
                     BAM_N_REF_CALLBACK n_ref_callback,
                     BAM_REF_INFO_CALLBACK ref_info_callback,
                     BAM_ALIGN_CALLBACK align_callback, void *user)
{
    //Check residual is valid or not.
    char *bam_data = data;
    size_t bam_data_size = data_size;
    if(fsm->r_size > 0)
    {
        //We need to copy the data to residual.
        //Check whether we need to create a larger buffer.
        data_size += fsm->r_size;
        if(fsm->r_reserve < data_size)
        {
            fsm->r = static_cast<char *>(realloc(fsm->r, data_size));
            fsm->r_reserve = data_size;
        }
        //Copy the data to target position.
        memcpy(fsm->r + fsm->r_size, data, data_size);
        //Reassign the data to residual data.
        data = fsm->r;
    }
    //Keep parsing while FSM breaks.
    bool keep_parsing = true;
    while(keep_parsing)
    {
        switch(fsm->state)
        {
        case BAM_FSM_FIND_HEADER:
        {
            //Parse the header.
            if(bam_data_size < sizeof(BAM_HEADER)) {keep_parsing = false; break;}
            BAM_HEADER *header = reinterpret_cast<BAM_HEADER *>(bam_data);
            //Call the callback function.
            if(header_callback) { header_callback(header->text, header->l_text, user); }
            //Move the size.
            size_t header_size = sizeof(BAM_HEADER) + header->l_text;
            bam_data += header_size; fsm->offset += header_size; bam_data_size -= header_size;
            //Changing the state.
            fsm->state = BAM_FSM_FIND_N_REF;
            break;
        }
        case BAM_FSM_FIND_N_REF:
        {
            //Parse the n_ref.
            if(bam_data_size < sizeof(uint32_t)) {keep_parsing = false; break;}
            uint32_t n_ref = *reinterpret_cast<uint32_t *>(bam_data);
            //Call the callback.
            if(n_ref_callback) { n_ref_callback(n_ref, user); }
            fsm->n_ref = n_ref;
            bam_data += sizeof(uint32_t); fsm->offset += sizeof(uint32_t); bam_data_size -= sizeof(uint32_t);
            //Changing the state.
            fsm->state = BAM_FSM_PARSE_REF;
            break;
        }
        case BAM_FSM_PARSE_REF:
        {
            if(fsm->ref_idx < fsm->n_ref)
            {
                //Try to parse one reference.
                if(bam_data_size < sizeof(BAM_REF_NAME)) {keep_parsing = false; break;}
                BAM_REF_NAME *ref_name = reinterpret_cast<BAM_REF_NAME *>(bam_data);
                size_t ref_info_size = sizeof(BAM_REF_NAME) + ref_name->l_name + sizeof(uint32_t);
                if(bam_data_size < ref_info_size) {keep_parsing = false; break;}
                //Call the callback.
                if(ref_info_callback) { ref_info_callback(fsm->ref_idx, ref_name, *(reinterpret_cast<uint32_t *>(bam_data + ref_name->l_name)), user); }
                //To the next reference information.
                bam_data += ref_info_size; fsm->offset += ref_info_size; bam_data_size -= ref_info_size;
                //Increase the index.
                ++fsm->ref_idx;
            }
            else
            {
                //Go to parse align.
                fsm->state = BAM_FSM_PARSE_ALIGN;
            }
            break;
        }
        case BAM_FSM_PARSE_ALIGN:
        {
            if(bam_data_size < sizeof(BAM_ALIGN)) {keep_parsing = false; break;}
            BAM_ALIGN *align = reinterpret_cast<BAM_ALIGN *>(bam_data);
            if(bam_data_size < align->block_size + 4) {keep_parsing = false; break;}
            //Call the callback.
            if(align_callback) { align_callback(fsm->offset, align, user); }
            //To the next BAM information.
            bam_data += align->block_size + 4; fsm->offset += align->block_size + 4; bam_data_size -= align->block_size + 4;
            break;
        }
        default:
        {
            break;
        }
        }
    }
    //When parsing completed, check the data left.
    size_t data_used = bam_data - data, data_remain = data_size - data_used;
    fsm->r_size = data_remain;
    if(fsm->r_size > 0)
    {
        //Check whether the residual could hold or not.
        if(fsm->r_reserve < data_remain)
        {
            fsm->r = static_cast<char *>(realloc(fsm->r, data_remain));
            fsm->r_reserve = data_remain;
        }
        //Copy the data.
        memmove(fsm->r, bam_data, data_remain);
    }
}

void pipeline_bam_parsing(BGZF_QUEUE *bgzf_queue,
                          BAM_HEADER_CALLBACK header_callback,
                          BAM_N_REF_CALLBACK n_ref_callback,
                          BAM_REF_INFO_CALLBACK ref_info_callback,
                          BAM_ALIGN_CALLBACK align_callback, void *user)
{
    //Create a parsing FSM.
    BAM_PARSE_FSM fsm;
    bam_initial_fsm(&fsm);
    time_print("Start BAM parsing work...");
    //Wait for cache queue is finished.
    while(!bgzf_queue->queue.empty() || !bgzf_queue->finish)
    {
        //Try to extract the BGZF info from queue.
        BGZF_DATA_SLICE slice = bgzf_pop_queue(bgzf_queue);
        if(slice.data == NULL) { continue; }
        //Process the data slice.
        bam_parse_align(slice.data, slice.size, &fsm,
                        header_callback, n_ref_callback,ref_info_callback, align_callback,
                        user);
        //Free the slice data.
        free(slice.data);
    }
    time_print("BAM parsing complete.");
    //Clear the FSM residual.
    if(fsm.r)
    {
        free(fsm.r);
    }
}
