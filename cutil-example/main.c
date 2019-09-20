#include <mpi.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "darshan-logutils.h"
#include <stdlib.h>

int main (int argc, char **argv)
{
    int world_rank;
    int world_size;
    MPI_Status status;

    // Initialize the MPI environment
    MPI_Init(NULL, NULL);
    // Find out rank, size
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    if (world_size < 2) {
        fprintf(stderr, "World size must be greater than 1 for %s\n", argv[0]);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    if (argc < 3) {
        fprintf(stderr, "Incorrect parameters, provide a list of files to process, and a location to output processed files");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    //fprintf(stdout, "World size and parameters OK\n");
    if (world_rank == 0) {
        //fprintf(stdout, "%s, %s\n", argv[1], argv[2]);
        // Master-controller Rank
        char* filename_list = argv[1];
        char filename_in[1024] = {'\0'};
        int rank = 1;
        int ranks_in_use;

        FILE *f = fopen(filename_list, "r");
        if (f == NULL) {
            fprintf(stderr, "Error opening list file %s\n", filename_list);
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        prepare_output_file(argv[2]);


        for (rank = 1, ranks_in_use = 0; rank < world_size && fscanf(f, "%s", filename_in) != EOF; rank++, ranks_in_use++) {
                //fprintf(stdout, "Sending %s (length %i) to rank %i\n", filename_in, strlen(filename_in), rank);
		MPI_Send(filename_in, strlen(filename_in), MPI_CHAR, rank, 0, MPI_COMM_WORLD);
        }
        while (fscanf(f, "%s", filename_in) != EOF) {
            char filename_processed[1024] = {'\0'};
            MPI_Recv(filename_processed, 1024, MPI_CHAR, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
            write_processed(filename_processed);
            MPI_Send(&filename_in, strlen(filename_in), MPI_CHAR, status.MPI_SOURCE, 0, MPI_COMM_WORLD);
        }
        fclose(f);
        while (ranks_in_use > 0) {
            char filename_processed[1024] = {'\0'};
            MPI_Recv(filename_processed, 1024, MPI_CHAR, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
            write_processed(filename_processed);
            MPI_Send(" ", 1, MPI_CHAR, status.MPI_SOURCE, 1, MPI_COMM_WORLD);
            ranks_in_use--;
        }
    } else {
        // File-processing Ranks
        while(1) {
            char filename_in[1024] = {'\0'};
            char filename_out[1024] = {'\0'};
            char name[1024] = {'\0'};
            MPI_Recv(filename_in, 1024, MPI_CHAR, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            //fprintf(stdout, "Rank %i: Recieved file %s from rank 0.\n", world_rank, filename_in);
            if (status.MPI_TAG == 0) {
                char *s = strrchr(filename_in, '/');
                if(s==NULL) {
                    strcpy(name, filename_in);
                } else {
                    strcpy(name, s + 1);
                }
                sprintf(filename_out, "%s/%s.csv\0", argv[2], name);
                //fprintf(stdout, "Rank %i: Processing file %s.\n", world_rank, filename_out);
                process_file(&filename_in, &filename_out);
	        MPI_Send(filename_in, strlen(filename_in), MPI_CHAR, 0, 0, MPI_COMM_WORLD);
            } else if (status.MPI_TAG == 1) {
                break;
            }
        }
    }
    MPI_Finalize();
}

int process_file(char* filename_in, char* filename_out) {
    darshan_fd          file;    
    int                 ret;
    struct darshan_job  job;
    struct darshan_file cp_file;
    int                 mount_count;
    int64_t*            devs;
    char**              mnt_pts;
    char**              fs_types;
    char                tmp_string[4096];
    int                 record_count;

    //fprintf(stdout, "Processing: %s, %s\n", filename_in, filename_out);

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

    // useful if you want to map files to file systems
    ret = darshan_log_getmounts(file, &devs, &mnt_pts, &fs_types, &mount_count);
    if (ret < 0)
    {
        fprintf(stderr, "mount info missing.\n");
    }

    int hret = write_header_to_csv(filename_out);
    if (hret < 0) {
        printf("can't open %s", filename_out);
	darshan_log_close(file);
	return(-1);
    }

    int cret = 0;
    for (ret = 1, record_count = 0; ret > 0;)
    {
        ret = darshan_log_getfile(file, &job, &cp_file);
        if(ret < 0)
        {
            fprintf(stderr, "Error: failed to parse log file.\n");
            fflush(stderr);
            darshan_log_close(file);
            return(-1);
        }
        else
        {
            cret = write_to_csv(&tmp_string, &job, cp_file, filename_out);
            if (cret < 0) {
                darshan_log_close(file);
                return(-1);
            }
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
        fprintf(stderr, "Error opening csv %s\n", filename_out);
        return -1;
    }
    fprintf(f, "exe_name,hash,rank,name_suffix,version_string,magic_nr,uid,start_time,end_time,nprocs,jobid"
               "CP_INDEP_OPENS,CP_COLL_OPENS,CP_INDEP_READS,CP_INDEP_WRITES,CP_COLL_READS,CP_COLL_WRITES,CP_SPLIT_READS,CP_SPLIT_WRITES,"
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
               "CP_SIZE_AT_OPEN,CP_FASTEST_RANK,CP_FASTEST_RANK_BYTES,CP_SLOWEST_RANK,CP_SLOWEST_RANK_BYTES,"
               // F_COUNTERS
               "CP_F_OPEN_TIMESTAMP ,CP_F_READ_START_TIMESTAMP,CP_F_WRITE_START_TIMESTAMP,CP_F_CLOSE_TIMESTAMP,CP_F_READ_END_TIMESTAMP,"
               "CP_F_WRITE_END_TIMESTAMP,CP_F_POSIX_READ_TIME,CP_F_POSIX_WRITE_TIME,CP_F_POSIX_META_TIME,CP_F_MPI_META_TIME,CP_F_MPI_READ_TIME,"
               "CP_F_MPI_WRITE_TIME,CP_F_MAX_READ_TIME,CP_F_MAX_WRITE_TIME,CP_F_FASTEST_RANK_TIME,CP_F_SLOWEST_RANK_TIME,CP_F_VARIANCE_RANK_TIME,"
               "CP_F_VARIANCE_RANK_BYTES,\n");
    fclose(f);
    return 0;
}

int write_to_csv (char* tmp_string, struct darshan_job *job, struct darshan_file cp_file, char *filename_out) {
    FILE *f = fopen(filename_out, "a");
    if (f == NULL) {
        fprintf(stderr, "Error opening csv %s\n", filename_out);
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

int write_processed(char* filename) {
    //fprintf(stdout, "File processed: %s\n", filename);
    FILE *f = fopen("./processed_list.txt", "a");
    if (f == NULL) {
        fprintf(stderr, "Error opening processed file list.\n");
        return -1;
    }
    fprintf(f, "%s\n", filename);
    fclose(f);
    return 0;
}

int prepare_output_file(char* filename_out) {
    char command[1024] = "";
    sprintf(command, "mkdir -p %s", filename_out);
    //fprintf(stdout, "%s\n", command);
    system(command);
    return 0;
}
