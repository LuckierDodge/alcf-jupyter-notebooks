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
    fprintf(f, "%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,\n", "exe_name", "hash", "rank", "name_suffix", 
                                 "version_string", "magic_nr", "uid", "start_time", "end_time", "nprocs", "jobid");
    fclose(f);
    return 0;
}

int write_to_csv (char tmp_string[], struct darshan_job *job, struct darshan_file cp_file, char *filename_out) {
    FILE *f = fopen(filename_out, "a");
    if (f == NULL) {
        printf("Error opening csv %s\n", filename_out);
        return -1;
    }
    fprintf(f, "%s,%x,%i,%s,%s,%i,%i,%i,%i,%i,%i\n", tmp_string, cp_file.hash, cp_file.rank, cp_file.name_suffix,
                                      job->version_string, job->magic_nr, job->uid, job->start_time, job->end_time, job->nprocs, job->jobid);
    fclose(f);
    return 0;
}
