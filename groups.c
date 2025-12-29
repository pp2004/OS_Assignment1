#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <math.h>
#define PERMS 0666
#define MAX_MSG_LEN 256
#define MAX_MESSAGES 26000
#include <sys/stat.h>
typedef struct
{
    long mtype;
    int timestamp;
    int user;
    char mtext[256];
    int modifyingGroup;
} Message;
typedef struct
{
    long mtype;
    int timestamp;
    int user;
    char mtext[256];
    int modifyingGroup;
    char status[256];
} MessageS;
typedef struct
{
    long mtype;
    int user;
    int group;
    int message_count;
} UserCap;

Message msgA;
Message msgU;
int msg_count = 0;
Message message_list[MAX_MESSAGES];
int userMessages[30][50];
int banned_user[50];
int N, keyG_V, keyG_A, keyG_M, threshold;
int msgG_M;
int grpno;
int msqidV;
int compare_timestamps(const void *a, const void *b)
{
    return ((Message *)a)->timestamp - ((Message *)b)->timestamp;
}
int max(int a, int b)
{

    return (a > b) ? a : b;
}
void removeWhitespaces(char *str)
{
    int i = 0, j = 0;
    while (str[i])
    {
        if (!isspace((unsigned char)str[i]))
        {
            str[j++] = str[i];
        }
        i++;
    }
    str[j] = '\0';
}
void read_msg_frm_pipe(int pipe_fd[2])
{
    Message msg_cpy;
    close(pipe_fd[1]);
    while (read(pipe_fd[0], &msg_cpy, sizeof(Message)) > 0)
    {
        if (msg_count < MAX_MESSAGES)
        {
            message_list[msg_count++] = msg_cpy;

            userMessages[msg_cpy.modifyingGroup][msg_cpy.user] = max(userMessages[msg_cpy.modifyingGroup][msg_cpy.user], msg_cpy.timestamp);
        }
    }
}
int main(int argc, char *argv[])
{
    //- printf("in grp\n");
    char input_path[100];
    int testcase_x = atoi(argv[2]);
    snprintf(input_path, sizeof(input_path), "testcase_%d/input.txt", testcase_x);

    FILE *input = fopen(input_path, "r");
    if (input == NULL)
    {
        //  perror("Error opening file");
        return 1;
    }

    Message msgG_V;

    fscanf(input, "%d", &N);
    fscanf(input, "%d", &keyG_V);
    fscanf(input, "%d", &keyG_A);
    fscanf(input, "%d", &keyG_M);
    fscanf(input, "%d", &threshold);
    fclose(input);

    char *my_path = argv[1];

    sscanf(my_path, "groups/group_%d.txt", &grpno);

    msqidV = msgget(keyG_V, PERMS | IPC_CREAT);
    int msqA_G = msgget(keyG_A, PERMS | IPC_CREAT);
    msgG_V.mtype = 1;
    msgG_V.modifyingGroup = grpno;
    usleep(1000 * grpno);

    if (msgsnd(msqidV, &msgG_V, sizeof(msgG_V) - sizeof(long), 0) == -1)
    {
        // perror("grp failed to send msg");
    }
    else
    {
        // printf("grp sent msg\n");
    }
    usleep(500);

    int pipe_fd[2];
    if (pipe(pipe_fd) == -1)
    {
        perror("Failed to create a pipe");
        return 1;
    }

    char final_path[100];
    snprintf(final_path, sizeof(final_path), "testcase_%d/%s", testcase_x, my_path);
    removeWhitespaces(final_path);

    int u_no;
    FILE *groupfile = fopen(final_path, "r");
    if (groupfile == NULL)
    {
        // perror("Error opening file");
        return 1;
    }

    fscanf(groupfile, "%d", &u_no);
    char **user = (char **)malloc(u_no * sizeof(char *));
    for (int j = 0; j < u_no; j++)
    {
        user[j] = (char *)malloc(100 * sizeof(char));
        fscanf(groupfile, "%s", user[j]);
    }
    fclose(groupfile);

    for (int j = 0; j < u_no; j++)
    {
        pid_t pid_u = fork();
        if (pid_u == 0)
        {
            close(pipe_fd[0]);

            Message msgU_V;
            char user_path[100];
            int usrno, ugrpno;
            sscanf(user[j], "users/user_%d_%d.txt", &ugrpno, &usrno);
            snprintf(user_path, sizeof(user_path), "testcase_%d/%s", testcase_x, user[j]);
            removeWhitespaces(user_path);

            FILE *userfile = fopen(user_path, "r");
            if (!userfile)
            {
                // perror("Error opening file");
                return 1;
            }

            msgU_V.mtype = 2;
            msgU_V.modifyingGroup = ugrpno;
            msgU_V.user = usrno;
            int msqidV = msgget(keyG_V, PERMS | IPC_CREAT);

            if (msgsnd(msqidV, &msgU_V, sizeof(msgU_V) - sizeof(long), 0) == -1)
            {
                //  perror("user failed to send msg");
            }
            else
            {
                // printf("user sent msg\n");
            }
            usleep(5000);

            char line[250];
            Message msgr;
            msgr.mtype = 2;
            msgr.modifyingGroup = grpno;
            msgr.user = usrno;
            int user_message_count = 0;
            while (fgets(line, sizeof(line), userfile) != NULL)
            {
                line[strcspn(line, "\n")] = 0;
                char *token = strtok(line, " ");
                if (token != NULL)
                {
                    msgr.timestamp = atol(token);
                    token = strtok(NULL, "");
                    if (token != NULL)
                    {
                        strncpy(msgr.mtext, token, MAX_MSG_LEN);
                        write(pipe_fd[1], &msgr, sizeof(Message));
                        user_message_count++;
                    }
                }
            }
            close(pipe_fd[1]);
            fclose(userfile);
            exit(0);
        }
    }

    close(pipe_fd[1]);

    Message msg_cpy;
    // int msg_count = 0;
    Message message_list[MAX_MESSAGES];
    // int userMessages[30][50];
    // int banned_user[50];
    for (int i = 0; i < 50; i++)
    {
        banned_user[i] = 0;
    }
    for (int i = 0; i < 30; i++)
    {
        for (int j = 0; j < 50; j++)
            userMessages[i][j] = 0;
    }
    // read_msg_frm_pipe(pipe_fd);
    while (read(pipe_fd[0], &msg_cpy, sizeof(Message)) > 0)
    {
        if (msg_count < MAX_MESSAGES)
        {
            message_list[msg_count++] = msg_cpy;

            userMessages[msg_cpy.modifyingGroup][msg_cpy.user] = max(userMessages[msg_cpy.modifyingGroup][msg_cpy.user], msg_cpy.timestamp);
        }
    }
    usleep(10000);
    qsort(message_list, msg_count, sizeof(Message), compare_timestamps);
    msgG_M = msgget(keyG_M, PERMS | IPC_CREAT);
    MessageS temp_msg;
    // while (msgrcv(msgG_M, &temp_msg, sizeof(MessageS) - sizeof(long), 0, IPC_NOWAIT) > 0)
    // {
    //     // Messages are being read and discarded
    // }
    for (int i = 0; i < msg_count; i++ )
    {

        MessageS msgU_V2s;
        msgU_V2s.mtype = 30 + grpno;
        msgU_V2s.modifyingGroup = message_list[i].modifyingGroup;
        msgU_V2s.timestamp = message_list[i].timestamp;
        strcpy(msgU_V2s.mtext, message_list[i].mtext);
        msgU_V2s.user = message_list[i].user;
        strcpy(msgU_V2s.status, "");
        int max_send_retries = 5;
        int send_retry_count = 0;
        while (send_retry_count < max_send_retries)
        {
              //- printf(" waiting for grp to send to mod \n");
            if (msgsnd(msgG_M, &msgU_V2s, sizeof(MessageS) - sizeof(long), 0) == -1)
            {
                // printf(" waiting for sender 2\n");
                if (errno == EAGAIN)
                {
                    usleep(5000);
                    send_retry_count++;
                    continue;
                }
                //  perror("grp failed to send msg to moderator");
                break;
            }
            else
            {
               //  printf("A user [%d] from grp [%d] timestamp [%d] sent %s to moderator\n", msgU_V2s.user, grpno,msgU_V2s.timestamp, message_list[i].mtext);

                break;
            }
        }

        MessageS msgM_rcv;
        int max_retries = 5;
        int retry_count = 0;
        // printf("\n grp no %d \n", grpno);
        // for (int i = 0; i < 10; i++)
        // {
        //     for (int j = 0; j < 20; j++)
        //         printf("%d ", userMessages[i][j]);
        //     printf("\n");
        // }
        while (retry_count < max_retries)
        {
            struct msqid_ds buf;
            msgctl(msgG_M, IPC_STAT, &buf);

           // printf("waiting for rcv from mod of user: %d, group: %d, mtype: %d | Messages in queue: %lu\n",
           //        msgU_V2s.user, msgU_V2s.modifyingGroup, 70 + msgU_V2s.modifyingGroup, buf.msg_qnum);

            // printf("waiting for rcv from mod of user : %d and group %d adn mtype %d\n", msgU_V2s.user, msgU_V2s.modifyingGroup, 70 + msgU_V2s.modifyingGroup);
       // printf("Size of msgM_rcv: %lu, expecting: %lu\n", sizeof(msgM_rcv), sizeof(msgM_rcv) - sizeof(msgM_rcv.mtype));
            if (msgrcv(msgG_M, &msgM_rcv, sizeof(msgM_rcv) - sizeof(msgM_rcv.mtype), 70 + msgU_V2s.modifyingGroup, 0) != -1)
            {

                // if (msgM_rcv.modifyingGroup ==0)
                //  printf(" Recieved a message in Grp[%d] with mtype[%ld] from user [%d] from grp [%d] has  %d messages remaing  and timestamp %d\n",grpno,msgM_rcv.mtype, msgM_rcv.user, msgM_rcv.modifyingGroup, userMessages[msgM_rcv.modifyingGroup][msgM_rcv.user], msgM_rcv.timestamp);
                if (userMessages[msgM_rcv.modifyingGroup][msgM_rcv.user] == msgM_rcv.timestamp && strcmp(msgM_rcv.status, "bann"))
                {
                    strcpy(msgM_rcv.status, "over");
                    // printf("it was over");
                }
                // printf("user [%d] from grp [%d] is [%s]\n", msgM_rcv.user, msgM_rcv.modifyingGroup, msgM_rcv.status);
                break;
            }
            else if (errno != ENOMSG)
            {
                // perror("Error receiving moderator response");
                break;
            }

            retry_count++;

            if (retry_count == max_retries)
            {
                printf("Timeout waiting for moderator response for user [%d] grp [%d]\n", msgM_rcv.user, msgM_rcv.modifyingGroup);
            }
        }
        Message msgU_V2;
        msgU_V2.mtype = 30 + msgM_rcv.modifyingGroup;
        msgU_V2.modifyingGroup = msgM_rcv.modifyingGroup;
        msgU_V2.user = msgM_rcv.user;
        msgU_V2.timestamp = msgM_rcv.timestamp;
        // printf(" received text is %s and send text is %s and real text is %s \n",msgM_rcv.mtext,msgU_V2s.mtext, message_list[i].mtext);
        strcpy(msgU_V2.mtext, msgM_rcv.mtext);
        /// code to send validation messages

        // if (!strcmp(msgM_rcv.status, "bann"))
        // {
        //     banned_user[msgM_rcv.user] = 1;
        // }

        // printf("%d  %d %s %d  %d\n", u_no, banned_user[msgM_rcv.user], msgM_rcv.status, msgM_rcv.modifyingGroup, msgM_rcv.user);

        if (u_no - 1 && banned_user[msgM_rcv.user] == 0)
        {
            if (!strcmp(msgM_rcv.status, "ok"))

            { // if (msgM_rcv.modifyingGroup ==0)
              //  printf("entered ok\n");
                // Message msgU_V2;
                // printf("text of moderator rcv %s and of  Validator send is %s", msgM_rcv.mtext, msgU_V2.mtext);
                // printf("type %ld user %d and grp %d  \n", msgU_V2.mtype, msgU_V2.user, msgU_V2.modifyingGroup);
                if (msgsnd(msqidV, &msgU_V2, sizeof(Message) - sizeof(long), 0) == -1)
                {

                    perror("grp  failed to send ok to validator");
                }
                else
                {

                   //- printf(" user [%d] from grp [%d] was verifed and sent to validator timestamp  %d\n", msgU_V2.user, msgU_V2.modifyingGroup, msgU_V2.timestamp);
                }
            }
            else if (!strcmp(msgM_rcv.status, "over"))
            {
                // printf("typ e %ld user %d and grp %d  \n", msgU_V2.mtype, msgU_V2.user, msgU_V2.modifyingGroup);
                // if (msgM_rcv.modifyingGroup ==0)
                // printf("entered over\n");
                if (msgsnd(msqidV, &msgU_V2, sizeof(Message) - sizeof(long), 0) == -1)
                {
                    perror("grp  failed to send over to validator");
                }
                else
                {

                    //-  printf(" user [%d] from grp [%d] was verifed as over and sent to validator  [%d]\n", msgU_V2.user, msgU_V2.modifyingGroup, msgU_V2.timestamp);
                    //   printf(" grp no %d and  u_no %d \n", grpno, u_no);
                    u_no--;
                    if (u_no <= 1)
                    {
                        Message msg_GT;
                        msg_GT.mtype = 3;
                        msg_GT.modifyingGroup = msgU_V2.modifyingGroup;
                        int count = 0;
                        for (int i = 0; i < 50; i++)
                            count += banned_user[i];

                        // if(msg_GT.modifyingGroup==2)
                        // msg_GT.user=0;
                        // else if(msg_GT.modifyingGroup==5)
                        // msg_GT.user=1;
                        msg_GT.user = count;
                        if (msgsnd(msqidV, &msg_GT, sizeof(Message) - sizeof(long), 0) == -1)
                        {
                            //  perror("grp  failed to terminate");
                        }
                        else
                        {

                            //-   printf(" Ov: successfully terminated grp [%d] no of violated users[%d]\n ", msg_GT.modifyingGroup,msg_GT.user);
                            // kill(getpid(), SIGTERM);
                        }

                        if (msgsnd(msqA_G, &msg_GT, sizeof(Message) - sizeof(long), 0) == -1)
                        {
                            //  perror("grp  failed to terminate");
                        }
                        else
                        {

                           //-    printf("successfully Group sent to app, terminated grp [%d] no of violated users[%d]\n ", msg_GT.modifyingGroup,msg_GT.user);
                            // kill(getpid(), SIGTERM);
                        }
                    }
                }
            }
            else if (!strcmp(msgM_rcv.status, "bann"))
            { // if (msgM_rcv.modifyingGroup ==0)
                // printf("entered bann\n");
                if (msgsnd(msqidV, &msgU_V2, sizeof(Message) - sizeof(long), 0) == -1)
                {

                    perror("ugrp  failed to send bann to validator");
                }
                else
                {

                   //-   printf(" user [%d] from grp [%d] is banned from now on  validator timestamp  %d\n", msgU_V2.user, msgU_V2.modifyingGroup, msgU_V2.timestamp);
                }
                banned_user[msgM_rcv.user] = 1;
                u_no--;
                if (u_no <= 1)
                {
                    Message msg_GT;
                    msg_GT.mtype = 3;
                    msg_GT.modifyingGroup = msgU_V2.modifyingGroup;
                    int count = 0;
                    for (int i = 0; i < 50; i++)
                        count += banned_user[i];

                    // if(msg_GT.modifyingGroup==2)
                    // msg_GT.user=0;
                    // else if(msg_GT.modifyingGroup==5)
                    // msg_GT.user=1;
                    msg_GT.user = count;
                    if (msgsnd(msqidV, &msg_GT, sizeof(Message) - sizeof(long), 0) == -1)
                    {
                        //  perror("grp  failed to terminate");
                    }
                    else
                    {

                        //-  printf("Ban: successfully terminated grp [%d] no of violated users[%d]\n ", msg_GT.modifyingGroup,msg_GT.user);
                        // kill(getpid(), SIGTERM);
                    }
                    if (msgsnd(msqA_G, &msg_GT, sizeof(Message) - sizeof(long), 0) == -1)
                    {
                        //  perror("grp  failed to terminate");
                    }
                    else
                    {

                        //-   printf("successfully Group sent to app, terminated grp [%d] no of violated users[%d]\n ", msg_GT.modifyingGroup,msg_GT.user);
                        // kill(getpid(), SIGTERM);
                    }
                }
            }
        }
    }

    close(pipe_fd[0]);
    for (int j = 0; j < u_no; j++)
    {
        wait(NULL);
    }

    for (int j = 0; j < u_no; j++)
    {
        free(user[j]);
    }
    free(user);

    exit(0);
    return 0;
}