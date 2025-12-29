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
#include <sys/stat.h> 
#define PERMS 0666
#define MAX_MSG_LEN 256
#define MAX_MESSAGES 10000
void toLowerCase(char *str)
{
    for (int i = 0; str[i]; i++)
    {
        str[i] = tolower(str[i]);
    }
}

void ArrayOnScreen(int arr[][50], int N)
{
   // - printf("     ");
    for (int j = 0; j < 50; j++)
    {
       // - printf("C%-3d ", j);
    }
   // - printf("\n");

   // - printf("     ");
    for (int j = 0; j < 50; j++)
    {
       // - printf("---- ");
    }
   // - printf("\n");

    for (int i = 0; i < N; i++)
    {
       // - printf("R%-3d ", i);
        for (int j = 0; j < 50; j++)
        {
           // - printf("%-4d ", arr[i][j]);
        }
       // - printf("\n");
    }
}

int main(int argc, char *argv[])
{
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

    if (argc < 2)
    {
       // - printf("testcase no not provided");
        return 0;
    }

    int testcase_x = atoi(argv[1]);
    setbuf(stdout,NULL);
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

   // - printf("%d\n", N);
   // - printf("%d\n", keyG_V);
   // - printf("%d\n", keyG_A);
   // - printf("%d\n", keyG_M);
   // - printf("%d\n", threshold);
   int userBanned[30][50];
   for (int i = 0; i < 30; i++)
   {
       for (int j = 0; j < 50; j++)
           userBanned[i][j] = 0;
   }
    snprintf(path, sizeof(path), "testcase_%d/%s", testcase_x, "filtered_words.txt");
    FILE *filtered_file = fopen(path, "r");
    if (filtered_file == NULL)
    {
        perror("Error opening file");
        return 1;
    }

    char word[250];
    char filtered_words[50][21];
    int count = 0;

    while (fgets(word, sizeof(word), filtered_file) != NULL)
    {
        char *token = strtok(word, "\n");
        strcpy(filtered_words[count], token);
        count++;
    }

    int user_violation_data[30][50];
    for (int i = 0; i < 30; i++)
    {
        for (int j = 0; j < 50; j++)
            user_violation_data[i][j] = 0;
    }

    // Message msgr;
    MessageS msgr;
    MessageS temp_msg;
    int msgG_M_id = msgget(keyG_M, PERMS | IPC_CREAT);
    // while (msgrcv(msgG_M_id, &temp_msg, sizeof(Message) - sizeof(long), 0, IPC_NOWAIT) > 0) {
    //     // Messages are being read and discarded
    // }
      
    while (msgrcv(msgG_M_id, &msgr, sizeof(msgr) - sizeof(msgr.mtype), -61, 0) > 0)
    { // - printf("recieved a message with mtype %ld\n",msgr.mtype);
        if (msgr.mtype >= 30 && msgr.mtype <70)
        {  
           // - printf("Checking that user [%d] from  grp[%d] with timestamp [%d] has violation or not \n", msgr.user, msgr.modifyingGroup,msgr.timestamp);
            strcpy(word, msgr.mtext);
           //// - printf("%s\n", word);
            for (int i = 0; i < count; i++)
            {
                char Lfilter[256];
                strcpy(Lfilter, filtered_words[i]);
                toLowerCase(Lfilter);
                toLowerCase(word);
                if(msgr.modifyingGroup==0&&msgr.user==32)
                {
                   // - printf(" word %s   ",word);
                   // - printf("filter %s",Lfilter);
                    // - printf("is %s",strstr(word, Lfilter));
                   // - printf( "  vio %d \n",user_violation_data[msgr.modifyingGroup][msgr.user]);
                }
                if (strstr(word, Lfilter) != NULL)
                {
                    
                    user_violation_data[msgr.modifyingGroup][msgr.user]++;
                   // - printf("Violation ! User [%d] from  grp[%d] with timestamp [%d] has violation ,violation count [%d] \n", msgr.user, msgr.modifyingGroup,msgr.timestamp,user_violation_data[msgr.modifyingGroup][msgr.user]);
                
                   
                }
            }
            // if(msgr.modifyingGroup==0&&msgr.user==5)
             // - printf("\nviolations %d and threshold %d time %d ",user_violation_data[msgr.modifyingGroup][msgr.user],threshold,msgr.timestamp);
            MessageS msg_T;
            if (user_violation_data[msgr.modifyingGroup][msgr.user] >= threshold)
            {
                msg_T.mtype = 70+msgr.modifyingGroup;
                msg_T.modifyingGroup = msgr.modifyingGroup;
                msg_T.user = msgr.user;
                msg_T.timestamp =msgr.timestamp;
                strcpy(msg_T.mtext,msgr.mtext);
                strcpy(msg_T.status, "bann");
                if(userBanned [msgr.modifyingGroup][msgr.user]==0)
                printf("User [%d] from group [%d] has been removed due to [%d] violations.\n", msgr.user, msgr.modifyingGroup,user_violation_data[msgr.modifyingGroup][msgr.user]);
                userBanned [msgr.modifyingGroup][msgr.user] =1 ;
                int retry_count = 0;
                int max_retries = 3;
                while (msgsnd(msgG_M_id, &msg_T, sizeof(Message) - sizeof(long), 0) == -1 && retry_count < max_retries)
                {
                    perror("error :");
                   // - printf("failed to terminate user [%d] from grp[%d], retrying...\n", msg_T.user, msg_T.modifyingGroup);
                    retry_count++;
                    sleep(1); // wait for 1 second before retrying
                }
                if (retry_count == max_retries)
                {
                   // - printf("failed to terminate user [%d] from grp[%d] after %d retries\n", msg_T.user, msg_T.modifyingGroup, max_retries);
                }
                else
                {   
                  


                  // - printf(" Message Bann sent to groups for  user [%d] from grp[%d] timestamp [%d]\n", msg_T.user, msg_T.modifyingGroup,msg_T.timestamp);
                }
            }
            else if (user_violation_data[msgr.modifyingGroup][msgr.user] < threshold)
            {
               
                  // user and group not needed for this message
                MessageS msg_S;
                msg_S.mtype = 70+msgr.modifyingGroup;
                msg_S.modifyingGroup = msgr.modifyingGroup;
                msg_S.timestamp =msgr.timestamp;
                strcpy(msg_S.mtext,msgr.mtext);
                msg_S.user = msgr.user;
                strcpy(msg_S.status, "ok");

                int retry_count = 0;
                int max_retries = 5;
                // - printf("Safe ,modreator found msg from User [%d] from  grp[%d] with timestamp [%d] is safe [%d] \n", msgr.user, msgr.modifyingGroup,msgr.timestamp,user_violation_data[msgr.modifyingGroup][msgr.user]);
                // - printf("Size of msg_S: %lu, sending: %lu\n", sizeof(msg_S), sizeof(msg_S) - sizeof(msg_S.mtype));
                while (msgsnd(msgG_M_id, &msg_S, sizeof(msg_S) - sizeof(msg_S.mtype), 0) == -1)
                {
                    perror("error :");
                   // - printf("failed to send that user [%d] is present in grp[%d], retrying...\n", msg_S.user, msg_S.modifyingGroup);
                    retry_count++;
                    // sleep(1); // wait for 1 second before retrying
                    if (retry_count == max_retries)
                    {
                       // - printf("failed to send that user [%d] is present in grp[%d] after %d retries\n", msg_S.user, msg_S.modifyingGroup, max_retries);
                    }
                    
                   
                }
               // - printf("sent from mod to grp  User [%d] from  grp[%d] with mtype %ld",msgr.user, msgr.modifyingGroup, msg_S.mtype);
                
            }
        }
    }
    exit (0);
    return(0);
}