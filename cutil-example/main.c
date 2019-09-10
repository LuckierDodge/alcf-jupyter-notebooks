#include <stdio.h>
#include "darshan-logutils.h"

int write_to_csv (char tmp_string[], struct darshan_job *job, struct darshan_file cp_file, char *filename_out);

int main (int argc, char **argv)
{
    darshan_fd          file;    
    char               *filename_in;
    char               *filename_out;
    int                 ret;
    struct darshan_job  job;
    struct darshan_file cp_file;
    int                 mount_count;
    int64_t*            devs;
    char**              mnt_pts;
    char**              fs_types;
    char                tmp_string[4096];
    int                 record_count;

    filename_in = argv[1];
    filename_out = argv[2];

    file = darshan_log_open(filename_in, "r");
    if(!file)
    {
        fprintf(stderr, "darshan_log_open() failed to open %s\n.", filename_in);
        return(-1);
    }

    ret = darshan_log_getjob(file, &job);
    if(ret < 0)
    {
        fprintf(stderr, "Error: unable to read job information from log file.\n");
        darshan_log_close(file);
        return(-1);
    }

    darshan_log_print_version_warnings(&job);

    // only needed if you want th exe name
    ret = darshan_log_getexe(file, tmp_string);
    if(ret < 0)
    {
        fprintf(stderr, "Error: unable to read trailing job information.\n");
        darshan_log_close(file);
        return(-1);
    }

    // useful is you want to map files to file systems
    ret = darshan_log_getmounts(file, &devs, &mnt_pts, &fs_types, &mount_count);
    if (ret < 0)
    {
        fprintf(stderr, "mount info missing.\n");
    }

    int hret = write_header_to_csv(filename_out);
    if (hret < 0) {
        printf("can't open %s", filename_out);
    }

    int cret = 0;
    for (ret = 1, record_count = 0; ret > 0;)
    {
        ret = darshan_log_getfile(file, &job, &cp_file);
        if(ret < 0)
        {
            fprintf(stderr, "Error: failed to parse log file.\n");
            fflush(stderr);
        }
        else
        {
            //
            // put your processing code here
            cret = write_to_csv(tmp_string, &job, cp_file, filename_out);
            // you get one record per file per rank unless it is fully shared
            record_count++;
        }
    }

    darshan_log_close(file);

    printf("log contains %d records.\n", record_count);

    return 0;
}

int write_header_to_csv(char *filename_out) {
    FILE *f = fopen(filename_out, "w");
    if (f == NULL) {
        printf("Error opening csv %s\n", filename_out);
        return -1;
    }
    fprintf(f, "%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,", "exe_name", "hash", "rank", "name_suffix", 
                                 "version_string", "magic_nr", "uid", "start_time", "end_time", "nprocs", "jobid");
    fprintf(f, "CP_INDEP_OPENS,CP_COLL_OPENS,CP_INDEP_READS,CP_INDEP_WRITES,CP_COLL_READS,CP_COLL_WRITES,CP_SPLIT_READS,CP_SPLIT_WRITES,"
              "CP_NB_READS,CP_NB_WRITES,CP_SYNCS,CP_POSIX_READS,CP_POSIX_WRITES,CP_POSIX_OPENS,CP_POSIX_SEEKS,CP_POSIX_STATS,"
              "CP_POSIX_MMAPS,CP_POSIX_FREADS,CP_POSIX_FWRITES,CP_POSIX_FOPENS,CP_POSIX_FSEEKS,CP_POSIX_FSYNCS,CP_POSIX_FDSYNCS,"
              "CP_INDEP_NC_OPENS,CP_COLL_NC_OPENS,CP_HDF5_OPENS,CP_COMBINER_NAMED,CP_COMBINER_DUP,CP_COMBINER_CONTIGUOUS,CP_COMBINER_VECTOR,"
              "CP_COMBINER_HVECTOR_INTEGER,CP_COMBINER_HVECTOR,CP_COMBINER_INDEXED,CP_COMBINER_HINDEXED_INTEGER,CP_COMBINER_HINDEXED,"
              "CP_COMBINER_INDEXED_BLOCK,CP_COMBINER_STRUCT_INTEGER,CP_COMBINER_STRUCT,CP_COMBINER_SUBARRAY,CP_COMBINER_DARRAY,"
              "CP_COMBINER_F90_REAL,CP_COMBINER_F90_COMPLEX,CP_COMBINER_F90_INTEGER,CP_COMBINER_RESIZED,CP_HINTS,CP_VIEWS,CP_MODE,"
              "CP_BYTES_READ,CP_BYTES_WRITTEN,CP_MAX_BYTE_READ,CP_MAX_BYTE_WRITTEN,CP_CONSEC_READS,CP_CONSEC_WRITES,CP_SEQ_READS,"
              "CP_SEQ_WRITES,CP_RW_SWITCHES,CP_MEM_NOT_ALIGNED,CP_MEM_ALIGNMENT,CP_FILE_NOT_ALIGNED,CP_FILE_ALIGNMENT,CP_MAX_READ_TIME_SIZE,"
              "CP_MAX_WRITE_TIME_SIZE,CP_SIZE_READ_0_100,CP_SIZE_READ_100_1K,CP_SIZE_READ_1K_10K,CP_SIZE_READ_10K_100K,CP_SIZE_READ_100K_1M,"
              "CP_SIZE_READ_1M_4M,CP_SIZE_READ_4M_10M,CP_SIZE_READ_10M_100M,CP_SIZE_READ_100M_1G,CP_SIZE_READ_1G_PLUS,CP_SIZE_WRITE_0_100,"
              "CP_SIZE_WRITE_100_1K,CP_SIZE_WRITE_1K_10K,CP_SIZE_WRITE_10K_100K,CP_SIZE_WRITE_100K_1M,CP_SIZE_WRITE_1M_4M,CP_SIZE_WRITE_4M_10M,"
              "CP_SIZE_WRITE_10M_100M,CP_SIZE_WRITE_100M_1G,CP_SIZE_WRITE_1G_PLUS,CP_SIZE_READ_AGG_0_100,CP_SIZE_READ_AGG_100_1K,"
              "CP_SIZE_READ_AGG_1K_10K,CP_SIZE_READ_AGG_10K_100K,CP_SIZE_READ_AGG_100K_1M,CP_SIZE_READ_AGG_1M_4M,CP_SIZE_READ_AGG_4M_10M,"
              "CP_SIZE_READ_AGG_10M_100M,CP_SIZE_READ_AGG_100M_1G,CP_SIZE_READ_AGG_1G_PLUS,CP_SIZE_WRITE_AGG_0_100, CP_SIZE_WRITE_AGG_100_1K,"
              "CP_SIZE_WRITE_AGG_1K_10K,CP_SIZE_WRITE_AGG_10K_100K,CP_SIZE_WRITE_AGG_100K_1M,CP_SIZE_WRITE_AGG_1M_4M,CP_SIZE_WRITE_AGG_4M_10M,"
              "CP_SIZE_WRITE_AGG_10M_100M,CP_SIZE_WRITE_AGG_100M_1G,CP_SIZE_WRITE_AGG_1G_PLUS,CP_EXTENT_READ_0_100,CP_EXTENT_READ_100_1K,"
              "CP_EXTENT_READ_1K_10K,CP_EXTENT_READ_10K_100K,CP_EXTENT_READ_100K_1M,CP_EXTENT_READ_1M_4M,CP_EXTENT_READ_4M_10M,"
              "CP_EXTENT_READ_10M_100M,CP_EXTENT_READ_100M_1G,CP_EXTENT_READ_1G_PLUS,CP_EXTENT_WRITE_0_100,CP_EXTENT_WRITE_100_1K,"
              "CP_EXTENT_WRITE_1K_10K,CP_EXTENT_WRITE_10K_100K,CP_EXTENT_WRITE_100K_1M,CP_EXTENT_WRITE_1M_4M,CP_EXTENT_WRITE_4M_10M,"
              "CP_EXTENT_WRITE_10M_100M,CP_EXTENT_WRITE_100M_1G,CP_EXTENT_WRITE_1G_PLUS,CP_STRIDE1_STRIDE,CP_STRIDE2_STRIDE,CP_STRIDE3_STRIDE,"
              "CP_STRIDE4_STRIDE,CP_STRIDE1_COUNT,CP_STRIDE2_COUNT,CP_STRIDE3_COUNT,CP_STRIDE4_COUNT,CP_ACCESS1_ACCESS,CP_ACCESS2_ACCESS,"
              "CP_ACCESS3_ACCESS,CP_ACCESS4_ACCESS,CP_ACCESS1_COUNT,CP_ACCESS2_COUNT,CP_ACCESS3_COUNT,CP_ACCESS4_COUNT,CP_DEVICE,"
              "CP_SIZE_AT_OPEN,CP_FASTEST_RANK,CP_FASTEST_RANK_BYTES,CP_SLOWEST_RANK,CP_SLOWEST_RANK_BYTES,");
    fprintf(f, "CP_F_OPEN_TIMESTAMP ,CP_F_READ_START_TIMESTAMP,CP_F_WRITE_START_TIMESTAMP,CP_F_CLOSE_TIMESTAMP,CP_F_READ_END_TIMESTAMP,"
              "CP_F_WRITE_END_TIMESTAMP,CP_F_POSIX_READ_TIME,CP_F_POSIX_WRITE_TIME,CP_F_POSIX_META_TIME,CP_F_MPI_META_TIME,CP_F_MPI_READ_TIME,"
              "CP_F_MPI_WRITE_TIME,CP_F_MAX_READ_TIME,CP_F_MAX_WRITE_TIME,CP_F_FASTEST_RANK_TIME,CP_F_SLOWEST_RANK_TIME,CP_F_VARIANCE_RANK_TIME,"
              "CP_F_VARIANCE_RANK_BYTES,\n");

    fclose(f);
    return 0;
}

int write_to_csv (char tmp_string[], struct darshan_job *job, struct darshan_file cp_file, char *filename_out) {
    FILE *f = fopen(filename_out, "a");
    if (f == NULL) {
        printf("Error opening csv %s\n", filename_out);
        return -1;
    }
    fprintf(f, "%s,%x,%i,%s,%s,%i,%i,%i,%i,%i,%i", tmp_string, cp_file.hash, cp_file.rank, cp_file.name_suffix,
                                      job->version_string, job->magic_nr, job->uid, job->start_time, job->end_time, job->nprocs, job->jobid);
    int i;
    for(i = 0; i < CP_NUM_INDICES; i++) {
        fprintf(f, "%i,", cp_file.counters[i]);
    }
    for(i = 0; i < CP_F_NUM_INDICES; i++) {
        fprintf(f, "%x,", cp_file.fcounters[i]);
    }
    fprintf(f, "\n");
    fclose(f);
    return 0;
}
