#include <stdio.h>
#include "darshan-logutils.h"

int main (int argc, char **argv)
{
    darshan_fd          file;    
    char               *filename;
    int                 ret;
    struct darshan_job  job;
    struct darshan_file cp_file;
    int                 mount_count;
    int64_t*            devs;
    char**              mnt_pts;
    char**              fs_types;
    char                tmp_string[4096];
    int                 record_count;

    filename = argv[1];

    file = darshan_log_open(filename, "r");
    if(!file)
    {
        fprintf(stderr, "darshan_log_open() failed to open %s\n.", filename);
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
            //
            // you get one record per file per rank unless it is fully shared
            record_count++;
        }
    }

    darshan_log_close(file);

    printf("log contains %d records.\n", record_count);

    return 0;
}

