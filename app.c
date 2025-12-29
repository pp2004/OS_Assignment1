#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <errno.h>
#include <string.h>
#define PERMS 0666

typedef struct
{
    long mtype;
    int timestamp;
    int user;
    char mtext[256];
    int modifyingGroup;
} Message;

int main(char argc, char *argv[])
{

    if (argc < 2)
    {
        printf("testcase no not provided");
        return 0;
    }
    int testcase_x = atoi(argv[1]);
    char path[100];
    snprintf(path, sizeof(path), "testcase_%d/%s", testcase_x, "input.txt");
    FILE *input = fopen(path, "r");
    if (input == NULL)
    {
        perror("Error opening file");
        return 1;
    }
    Message msg;
    int N, keyG_V, keyG_A, keyG_M, threshold;

    fscanf(input, "%d", &N);
    fscanf(input, "%d", &keyG_V);
    fscanf(input, "%d", &keyG_A);
    fscanf(input, "%d", &keyG_M);
    fscanf(input, "%d", &threshold);
    int msqidV = msgget(keyG_V, PERMS | IPC_CREAT);
    Message temp_msg;
    while (msgrcv(msqidV, &temp_msg, sizeof(Message) - sizeof(long), 0, IPC_NOWAIT) > 0)
    {
        // Messages are being read and discarded
    }

    // msgctl(msqidV, IPC_RMID, NULL);
    // Allocating an array for storing group names
    char **group = (char **)malloc(N * sizeof(char *));

    for (int i = 0; i < N; i++)
    {
        group[i] = (char *)malloc(18 * sizeof(char));
        fscanf(input, "%s\n", group[i]);
    }

    fclose(input);
    int activeGroups = N;
    // printing
    // printf("%d\n", N);
    // printf("%d\n", keyG_V);
    // printf("%d\n", keyG_A);
    // printf("%d\n", keyG_M);
    // printf("%d\n", threshold);

    // // Print group names
    // for (int i = 0; i < N; i++) {
    //     printf("%s\n", group[i]);
    // }
    int msqid;

    msqid = msgget(keyG_A, PERMS | IPC_CREAT);
    if (msqid == -1)
    {
        perror("msgget failed");
        exit(1);
    }

    int msgG_M = msgget(keyG_M, PERMS | IPC_CREAT);
    char keyG_A_str[20];
    int grp_pid[N];
    memset(grp_pid, 0, sizeof(grp_pid));

    for (int i = 0; i < N; i++)
    {
        pid_t pid = fork();
        if (pid == 0)
        {

            char grp_path[20];
            snprintf(grp_path, sizeof(grp_path), "%s", group[i]);
            char x[10];
            sprintf(x, "%d", testcase_x);
           //- printf("Entered Child up\n");
            execlp("./groups.out", "groups", grp_path, x, NULL);
        }
    }

    for (int i = 0; i < N; i++)
    {
        if (msgrcv(msqid, &msg, sizeof(msg), 0, 0) == -1)
        {
             perror("app failed to recieve message from grp");
        }
        else
        {
            if (msg.mtype == 3)
            { // A grp was terminated
                printf("All users terminated. Exiting group process %d\n", msg.modifyingGroup);
                activeGroups--;
            }
        }

        // printf("Received: %ld\n", msg.mtype);
     
    }
    for (int i = 0; i < N; i++)
    {
        wait(NULL);
        // printf("All users terminated. Exiting group process");
    }

    // int msqidV = msgget(keyG_V, PERMS | IPC_CREAT);
    printf("Exiting the program\n");
    msgctl(msqid, IPC_RMID, NULL);
    msgctl(msqidV, IPC_RMID, NULL);
    msgctl(msgG_M, IPC_RMID, NULL);

    return 0;
}
