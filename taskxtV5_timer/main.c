#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "taskxt.h" //for taskxt_arg_t

void set_vars(int fd, char *processName)
{
        taskxt_arg_t processInfo;

        strcpy(processInfo.processName, processName);
        processInfo.status;

        if(ioctl(fd, TASKXT_SET_PROCESSNAME, &processInfo) == -1)
        {
                perror("TASKXT_APPS IOCTL SET\n");
        }
}

int main(int argc, char *argv[])
{
        char *file_name = "/dev/taskxt";
        int fd;
        char pName[100];
        int i;

        if(argc != 3)
        {
                printf("Error: format: main -p processname\n");
        }

        pName[0] = 0;

        if(argc >= 3)
        {
                if(strcmp(argv[1], "-p") == 0)
                {
                        strcpy(pName, argv[2]);
                }
        }

        if((strlen(pName) == 0))
        {
                printf("Error: format: samp_pname -p processname\n");
                return -1;
        }

        printf("Sample mode: -p %s\n", pName);

        fd = open(file_name, O_RDWR);
        if(fd == -1)
        {
                perror("ERROR: taskxt_apps open\n");
                return 2;
        }

        set_vars(fd, pName);
        close(fd);

        return 0;
}
