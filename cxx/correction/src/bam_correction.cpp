#include <algorithm>
#include <cstring>
#include <cmath>
#include <mutex>

#include "arguments.h"
#include "hp_thread_pool.h"

#include "bam_correction.h"

void bam_initial_correct(BAM_CORRECT *correct, uint8_t mapq)
{
    correct->ref_names = NULL;
    correct->ref_name_size = 0;
    correct->mapq = mapq;
}

void bam_correct_n_ref(uint32_t n_ref, void *bam_correct)
{
    BAM_CORRECT *correct = reinterpret_cast<BAM_CORRECT *>(bam_correct);
    correct->ref_name_size = n_ref;
    //Allocate memory for names.
    correct->ref_names = static_cast<char **>(malloc(sizeof(char *) * n_ref));
    for(uint32_t i=0; i<n_ref; ++i)
    {
        correct->ref_names[i] = NULL;
    }
    correct->ref_aligns.resize(n_ref);
}

void bam_correct_ref_info(uint32_t ref_idx, BAM_REF_NAME *ref_name, uint32_t, void *bam_correct)
{
    BAM_CORRECT *correct = reinterpret_cast<BAM_CORRECT *>(bam_correct);
    //Copy the name.
    if(!correct->ref_names[ref_idx])
    {
        correct->ref_names[ref_idx] = static_cast<char *>(malloc(ref_name->l_name));
        strncpy(correct->ref_names[ref_idx], ref_name->name, ref_name->l_name);
    }
}

void bam_correct_align(size_t offset, BAM_ALIGN *align, void *bam_correct)
{
    (void)offset;
    //Check the validation.
    BAM_CORRECT *correct = reinterpret_cast<BAM_CORRECT *>(bam_correct);
    if(align->refID != -1 && align->refID == align->next_refID
            && align->pos != -1 && align->next_pos != -1 && align->mapq >= correct->mapq)
    {
        correct->ref_aligns[align->refID].push_back(
                    BAM_CORRECT_ALIGN{align->pos, align->next_pos});
    }
}

typedef struct POS_RANGE
{
    int32_t s;
    int32_t e;
} POS_RANGE;

bool operator== (const struct POS_RANGE &lhs, const struct POS_RANGE &rhs)
{
    return (lhs.s == rhs.s) && (lhs.e == rhs.e);
}

struct POS_RANGE_HASH
{
    size_t operator() (const POS_RANGE &range) const
    {
        return (static_cast<int64_t>(range.s) << 32) | (range.e & 0xFFFFFFFF);
    }
};

typedef struct POS
{
    int32_t first;
    int32_t second;
} POS;

struct POS_HASH
{
    size_t operator() (const POS &pos) const
    {
        return (static_cast<uint64_t>(pos.first) << 32) | (static_cast<uint64_t>(pos.second));
    }
};

bool operator== (const POS &lhs, const POS &rhs)
{
    return (lhs.first == rhs.first) && (lhs.second == rhs.second);
}

typedef std::unordered_map<POS_RANGE, uint32_t, POS_RANGE_HASH> CONTIG_RANGE_DB;
typedef std::unordered_map<int, long double> POS_DB;

inline void range_count(CONTIG_RANGE_DB &db, const POS_RANGE &range)
{
    auto range_iter = db.find(range);
    if(db.end() == range_iter)
    {
        //Insert a new record.
        db.insert(std::make_pair(range, 1));
    }
    else
    {
        //Increase the counter.
        ++range_iter->second;
    }
}

long double roundoff(long double value, uint8_t prec)
{
    long double pow_10 = powf(10.0, static_cast<long double>(prec));
    return roundf(value * pow_10) / pow_10;
}

long double calc_sat_level(const CONTIG_RANGE_DB &hic_db, long double pct)
{
    if(hic_db.empty())
        return -1;
    //Prepare the counter vector.
    std::vector<long double> tmp_list;
    tmp_list.reserve(hic_db.size());
    //Check in the database.
    for(auto hic_iter : hic_db)
    {
        //Check the range is equal.
        const POS_RANGE &poses = hic_iter.first;
        if(poses.s != poses.e)
        {
            tmp_list.push_back(hic_iter.second);
        }
    }
    if(tmp_list.empty())
        return -1;
    if(tmp_list.size() == 1)
        return tmp_list[0];
    //Sort the list.
    std::sort(tmp_list.begin(), tmp_list.end());
    //Reset the first several values as NaN.
    long double pos = static_cast<long double>(pct) * (tmp_list.size() + 1);
    if(pos < 1)
        return tmp_list[0];
    if(pos >= tmp_list.size())
        return tmp_list[tmp_list.size() - 1];
    // Calculate the d.
    long double d = pos - int(pos);
    return tmp_list[int(pos) - 1] + d * (tmp_list[int(pos)] - tmp_list[int(pos) - 1]);
}

void get_score_db(const CONTIG_RANGE_DB &hic_db, int bin_size, int dep_size, float pct, float sens,
                  POS_DB &score_db, float &thr)
{
    if(hic_db.empty())
        return;
    //Calculate the SAT level.
    long double sat_level = roundoff(calc_sat_level(hic_db, pct), 5);
    if(sat_level < 0)
        return;
    //Calculate the threshold.
    thr = sens*sat_level*0.5*dep_size/bin_size*(dep_size/bin_size-1);
    //Compute the dep score.
    for(auto iter : hic_db)
    {
        auto s = iter.first.s, e = iter.first.e;
        long double val = iter.second;
        if(e - s > dep_size)
            continue;
        if(val >= sat_level)
            val = sat_level;
        for(int i=s+bin_size; i<e; i+=bin_size)
        {
            auto score_db_iter = score_db.find(i);
            if(score_db_iter == score_db.end())
            {
                // Insert a new record.
                score_db.insert(std::make_pair(i, val));
            }
            else
            {
                score_db_iter->second += val;
            }
        }
    }
    int min_pos = 0, max_pos = 0;
    if(score_db.size() != 0)
    {
        //Find the max and min value of the database.
        int pmax = -1, pmin = -1;
        for(auto score_iter : score_db)
        {
            if(score_iter.first > pmax)
                pmax = score_iter.first;
            if(pmin == -1 || score_iter.first < pmin)
                pmin = score_iter.first;
        }
        min_pos = pmin;
        max_pos = pmax;
        //Replace the database with the sub score database.
        POS_DB sub_score_db;
        for(int i=min_pos+dep_size-2*bin_size; i<max_pos-dep_size+3*bin_size; i+=bin_size)
        {
            auto score_iter = score_db.find(i);
            if(score_iter == score_db.end())
            {
                sub_score_db.insert(std::make_pair(i, 0));
            }
            else
            {
                sub_score_db.insert(std::make_pair(i, score_iter->second));
            }
        }
        score_db = sub_score_db;
    }
}

std::vector<int> sorted_keys(const POS_DB &db)
{
    std::vector<int> keys;
    keys.reserve(db.size());
    for(auto iter : db)
        keys.push_back(iter.first);
    std::sort(keys.begin(), keys.end());
    return keys;
}

extern Argument opts;

MISMATCH get_narrow_mismatch(const std::list<BAM_CORRECT_ALIGN> &ailgns)
{
    //Cache the value to local.
    float percent = opts.percent;
    float sensitive = opts.sensitive;
    int wide = opts.wide;
    int narrow = opts.narrow;
    int depletion = opts.depletion;

    CONTIG_RANGE_DB wide_db, narrow_db;
    for(auto i=ailgns.begin(); i!=ailgns.end(); ++i)
    {
        int32_t pos = (*i).pos, next_pos = (*i).next_pos;
        range_count(wide_db, POS_RANGE{pos / wide * wide, next_pos / wide * wide});
        range_count(narrow_db, POS_RANGE{pos / narrow * narrow, next_pos / narrow * narrow});
    }
    POS_DB wide_score_db, narrow_score_db;
    float wide_thr, narrow_thr;
    get_score_db(wide_db, wide, depletion, percent, sensitive, wide_score_db, wide_thr);
    get_score_db(narrow_db, narrow, wide, percent, sensitive, narrow_score_db, narrow_thr);
    //Calculate the wide mismatch.
    // ------------ Wide ------------
    MISMATCH wide_mismatch;
    if(!wide_score_db.empty())
    {
        wide_mismatch.push_back(std::vector<int>());
        //Sort the key of the score db.
        std::vector<int> wide_keys = sorted_keys(wide_score_db);
        for(auto i : wide_keys)
        {
            if(wide_score_db[i] < wide_thr)
            {
                // Check the last row.
                if(wide_mismatch.back().empty())
                {
                    wide_mismatch.back().push_back(i);
                }
            }
            else
            {
                if(!wide_mismatch.back().empty())
                {
                    wide_mismatch.back().push_back(i);
                    wide_mismatch.push_back(std::vector<int>());
                }
            }
        }
        if(wide_mismatch.back().size() == 1)
        {
            wide_mismatch.back().push_back(wide_keys.back() + opts.wide);
        }
        else if(wide_mismatch.back().empty())
        {
            wide_mismatch.pop_back();
        }
    }
    if(wide_mismatch.empty() || narrow_score_db.empty())
    {
        return wide_mismatch;
    }
    //------------ Narrow ------------
    //Calculate the narrow mismatch (merge region).
    MISMATCH narrow_mismatch;
    const auto &wide_list = wide_mismatch;
    int idx_wide = 0;
    long double min_val = 0;
    std::vector<POS> tmp_list;
    tmp_list.reserve(narrow_score_db.size());
    for(auto pos : sorted_keys(narrow_score_db))
    {
        if(idx_wide >= static_cast<int>(wide_list.size()))
            break;
        if(pos <= wide_list[idx_wide][0])
        {
            min_val = narrow_score_db[pos];
        }
        else
        {
            if(narrow_score_db[pos] < min_val)
            {
                min_val = narrow_score_db[pos];
            }
        }
        if(pos+opts.narrow <= wide_list[idx_wide][0])
            continue;
        if(pos >= wide_list[idx_wide][1])
        {
            for(int i=wide_list[idx_wide][0]; i<wide_list[idx_wide][1]; i+=opts.narrow)
            {
                auto narrow_iter = narrow_score_db.find(i);
                if(narrow_iter != narrow_score_db.end() && narrow_iter->second == min_val)
                {
                    tmp_list.push_back(POS{i, i+opts.narrow});
                }
            }
            ++idx_wide;
        }
    }
    if(idx_wide < static_cast<int>(wide_list.size()))
    {
        for(int i=wide_list[idx_wide][0]; i<wide_list[idx_wide][1]; i+=opts.narrow)
        {
            auto narrow_iter = narrow_score_db.find(i);
            if(narrow_iter != narrow_score_db.end() && narrow_iter->second == min_val)
            {
                tmp_list.push_back(POS{i, i+opts.narrow});
            }
        }
    }
    if(tmp_list.empty())
    {
        return wide_mismatch;
    }
    int last_e = 0;
    for(auto iter : tmp_list)
    {
        auto s = iter.first, e = iter.second;
        if(last_e == 0)
        {
            std::vector<int> field;
            field.push_back(s);
            narrow_mismatch.push_back(field);
            last_e = e;
        }
        else
        {
            if(s != last_e)
            {
                narrow_mismatch.back().push_back(last_e);
                std::vector<int> field;
                field.push_back(s);
                narrow_mismatch.push_back(field);
            }
        }
        //Update the last end.
        last_e = e;
    }
    //Append the last e.
    narrow_mismatch.back().push_back(last_e);
    return narrow_mismatch;
}

typedef struct NarrowWork
{
    uint32_t idx;
    BAM_CORRECT *correct;
    NARROW_MAP *narrow_mismatches;
} NarrowWork;

std::mutex mismatch_mutex;

void narrow_map_worker(const NarrowWork &work)
{
    BAM_CORRECT *correct = work.correct;
    NARROW_MAP *narrow_mismatches = work.narrow_mismatches;
    //Fetch the narrow db.
    MISMATCH ref_mismatch = get_narrow_mismatch(correct->ref_aligns[work.idx]);
    if(ref_mismatch.size() == 0)
        return;
    //Insert to data.
    std::unique_lock<std::mutex> mismatch_lock(mismatch_mutex);
    narrow_mismatches->insert(std::make_pair(std::string(correct->ref_names[work.idx]),
                                             ref_mismatch));
}

void bam_correct_narrow_map(BAM_CORRECT *correct, NARROW_MAP *narrow_mismatches)
{
    uint32_t ref_name_size = correct->ref_name_size;
    thread_pool<void (const NarrowWork &), NarrowWork> work_threads(narrow_map_worker, opts.threads);
    for(uint32_t i=0; i<ref_name_size; ++i)
    {
        work_threads.push_task(NarrowWork {i, correct, narrow_mismatches});
    }
    work_threads.wait_for_tasks();
}
