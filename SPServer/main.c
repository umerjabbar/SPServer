//
//  Server.c
//  SPMileStone2
//
//  Created by Umer Jabbar on 27/04/2018.
//  Copyright © 2018 ZotionApps. All rights reserved.
//

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/types.h>
#include <netdb.h>
#include <time.h>
#include <fcntl.h>
#include <arpa/inet.h>

int maxProcessLimit = 50;

struct process{
    int sno;
    char name[100];
    int pid;
    int status;
    time_t startTime;
    time_t endTime;
    double elapsedTime;
    
};
struct connection{
    int sno;
    char ip[15];
    int port;
    int pid;
    int sockfd;
    int sendfd;
    int revfd;
    int status;
};

struct connection connectionList[50];
struct process processList[50];

void server(char* buff, ssize_t size, int fd2, struct process *processList);
void run(char* buff, char* buff2, int* fd3, int connfd, struct process *processList);
void killAll (struct process *processList);
void* serverInteraction(void* sock);
void* processToServer(void* sock);


void signalHandlerChild(int signal){
    
    int status;
    int pid = wait(&status);
    
    switch (signal) {
        case SIGCHLD:
            
            for (int i = 0 ; i < maxProcessLimit; i++) {
                
                if(processList[i].pid < 1 ){
                    break;
                }
                if(processList[i].pid == pid){
                    processList[i].status = 0;
                    processList[i].endTime = clock();
                    processList[i].elapsedTime = ((double)processList[i].endTime - processList[i].startTime)/CLOCKS_PER_SEC;
                    break;
                }
            }
            
            printf("Child Died: I am sorry for your loss");
            break;
            
        default:
            printf("water you looking at");
            break;
    }
    
}
void signalHandlerParent(int signal){
    
    int status;
    int pid = wait(&status);
    
    switch (signal) {
        case SIGCHLD:
            
            for (int i = 0 ; i < maxProcessLimit; i++) {
                
                if(connectionList[i].pid < 1 ){
                    break;
                }
                if(connectionList[i].pid == pid){
                    connectionList[i].status = 0;
                    break;
                }
            }
            
            printf("Child Died: I am sorry for your loss");
            break;
            
        default:
            printf("water you looking at");
            break;
    }
    
}

int main (){
    
    int listenfd = 0;
    int connfd = 0;
    struct sockaddr_in serv_addr;
    char sendBuff[1025];
    
    //initialization of socket
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, '0', sizeof(serv_addr));
    memset(sendBuff, '0', sizeof(sendBuff));
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(7714);
    
    bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    listen(listenfd, 10);
    
    write(1, "Welcome to our Server", sizeof("Welcome to our Server"));
    

    pthread_t sTod;
    //    pthread_t cTos;
    pthread_create(&sTod, NULL, serverInteraction, (void*) &listenfd);
    //    pthread_create(&cTos, NULL, processToServer, (void*) &pfd);
    
    for (int i = 0; i < maxProcessLimit; i++) {
        connectionList[i].pid = 0;
    }
    
    while(0==0){
        struct sockaddr_in clients;
        int c = sizeof(struct sockaddr_in);
        connfd = accept(listenfd, (struct sockaddr*) &clients, (socklen_t *) &c );
        char hostname[1024];
        
        if(getnameinfo((struct sockaddr*)&serv_addr, sizeof(serv_addr), hostname, 1024, NULL, 0, 0) == -1){
            perror("getnameInfo");
        }else{
            
        }
        
        char buff[2000];
        
        int fd[2];
        if(pipe(fd) == -1){
            perror("error on pipe fd");
        }
        
        int pid = fork();
        if(pid == -1){
            perror("fork on connect");
        }
        
        if(pid > 0){
            
            if(signal(SIGCHLD, signalHandlerParent)==SIG_ERR){
                perror("sigchild error");
            }
            
            int pC = 0;
            for (int i = 0 ; i < maxProcessLimit; i++) {
                pC = i;
                if(connectionList[i].pid < 1 ){
                    break;
                }
            }
            
            connectionList[pC].sno = pC;
            connectionList[pC].pid = pid;
            connectionList[pC].sockfd = connfd;
            connectionList[pC].revfd = fd[0];
            connectionList[pC].sendfd = fd[1];
            connectionList[pC].port = clients.sin_port;
            connectionList[pC].status = 1;
            strcpy(connectionList[pC].ip, inet_ntoa(clients.sin_addr));
            
        }
        
        if(pid == 0){
            
            if(signal(SIGCHLD, signalHandlerChild)==SIG_ERR){
                perror("sigchild error");
            }
            
            for (int i = 0; i < maxProcessLimit; i++) {
                processList[i].pid = 0;
                processList[i].status = 0;
            }
            
            while(0==0){
                ssize_t r1 = read(connfd, buff, 2000);
                if(r1 == -1){
                    perror("read from fd1[0]");
                    break;
                }else if(r1 == 0){
                    close(connfd);
                }
                //                ssize_t ser = write(STDOUT_FILENO, "\nserver:~ ", strlen("\nserver:~ "));
                //                if(ser == -1){
                //                    perror("server on console");
                //                    continue;
                //                }
                server(buff, r1, connfd, processList);
            }
        }
    }
    
    return 0;
}

void* processToServer(void* sock){
    
    //    int sockfd = *(int*) sock;
    
    
    
    pthread_exit(NULL);
}


void* serverInteraction(void* sock){
    
    //    int sockfd = *(int*) sock;
    printf("a\n");
    while (0==0) {
        char buff[2000];
        char processes[2000];
        int n = 0;
        
        ssize_t wd0 = write(1, "\n~", 2);
        if(wd0 == -1){
            perror("read from console");
            continue;
        }
        
        ssize_t rd1 = read(0, buff, 2000);
        if(rd1 == -1){
            perror("read from console");
            continue;
        }if(rd1 == 1){
            if(strcmp(buff, "\n")){
                continue;
            }
        }
        
        buff[rd1-1] = '\0';
        
        char * token;
        token = strtok(buff, " \n");
        
        printf("b\n");
        
        if(token != NULL){
            
            if(strcmp(token, "list") == 0){
                int count = 0;
                token = strtok(NULL, " ");
                char* temp = token;
                while (token != NULL) {
                    count++;
                    token = strtok(NULL, " ");
                }
                if(count == 1){
                    printf("c\n");
                    if(strcmp(temp, "connections") == 0){
                        
                        for (int i = 0; i < maxProcessLimit; i++) {
                            if(i==0){
                                sprintf(buff, "\n");
                                if(connectionList[i].pid < 1){
                                    n = sprintf(buff, "");
                                    break;
                                }
                            }
                            if(connectionList[i].pid < 1){
                                break;
                            }
                            char temp[2000];
                            n += sprintf(temp, "SNO: %d, PID: %d, IP: %s, Port: %d, SocketFD: %d, SendFD: %d, ReceiveFD: %d, Status: %d \n", connectionList[i].sno, connectionList[i].pid, connectionList[i].ip, connectionList[i].port, connectionList[i].sockfd, connectionList[i].sendfd, connectionList[i].revfd, connectionList[i].status);
                            strcat(buff, temp);
                        
                        }
                        
                    }else if(strcmp(temp, "processes") == 0){
                        for (int i = 0; i < maxProcessLimit; i++) {
                            if(connectionList[i].pid < 1){
                                break;
                            }
                            if (connectionList[i].status == 0) {
                                continue;
                            }
                            
                            ssize_t wp = write(connectionList[i].sendfd, "process", sizeof("process"));
                            if (wp == -1) {
                                perror("error on wp");
                            }else if (wp == 0){
                                continue;
                            }
                            
                            ssize_t rp = read(connectionList[i].revfd, buff, 2000);
                            if (rp == -1) {
                                perror("error on rp");
                            } else if (rp == 0) {
                                continue;
                            }
                            buff[rp - 1] = '\0';
                            
                            strcpy(processes, buff);
                        }
                        
                    }else{
                        n = sprintf(buff, "invalid command");
                    }
                }else{
                    n = sprintf(buff, "invalid command");
                }
                
            }else if(strcmp(token, "exit") == 0){
                
                
            }else{
                
                n = sprintf(buff, "command not found");
                
            }
            
            
            
        }
        
        printf("f\n");
        
        ssize_t wd1 = write(1, buff, n);
        if(wd1 == -1){
            perror("read from console");
            continue;
        }
        
    }
    
    //    pthread_exit(NULL);
}


void getProcessList(int sendfd, int revfd){
    
    
    
}


void server(char* buff, ssize_t size, int fd2, struct process *processList){
    
    buff[size] = '\0';
    
    char* token = strtok(buff, " ,-\n");
    if(token != NULL){
        
        if(strcmp(token, "add") == 0){
            int count = 0;
            size = 0;
            token = strtok(NULL, " ,-\n");
            while(token != NULL){
                size += atoi(token);
                count++;
                token = strtok(NULL, " ,-\n");
            }
            if(count == 0){
                ssize_t w1 = write(fd2, "usage: add [n1] [n2] ...", strlen("usage: add [n1] [n2] ..."));
                if(w1 == -1){
                    perror("write on fd2[1]");
                }
            }else if(count < 2){
                ssize_t w1 = write(fd2, "invalid arguments", strlen("invalid arguments"));
                if(w1 == -1){
                    perror("write on fd2[1]");
                }
            }else {
                
                size = sprintf(buff, "addition result is %zd", size);
                ssize_t w1 = write(fd2, buff, size);
                if(w1 == -1){
                    perror("write on fd2[1]");
                }
            }
            
        }
        else if(strcmp(token, "sub") == 0){
            int count = 0;
            size = 0;
            token = strtok(NULL, " ,-\n");
            if(token != NULL){
                size += atoi(token);
                token = strtok(NULL, " ,-\n");
                count++;
            }
            while(token != NULL){
                size -= atoi(token);
                count++;
                token = strtok(NULL, " ,-\n");
            }
            if(count == 0){
                ssize_t w1 = write(fd2, "usage: sub [n1] [n2] ...", strlen("usage: subtract [n1] [n2] ..."));
                if(w1 == -1){
                    perror("write on fd2[1]");
                }
            }else if(count < 2){
                ssize_t w1 = write(fd2, "invalid arguments", strlen("invalid arguments"));
                if(w1 == -1){
                    perror("write on fd2[1]");
                }
            }else {
                size = sprintf(buff, "subtraction result is %zd", size);
                ssize_t w1 = write(fd2, buff, size);
                if(w1 == -1){
                    perror("write on fd2[1]");
                }
            }
            
        }
        else if(strcmp(token, "div") == 0){
            int count = 0;
            size = 0;
            int invalid = 0;
            token = strtok(NULL, " ,-\n");
            if(token != NULL){
                size += atoi(token);
                if(size == 0){
                    invalid = -1;
                }
                token = strtok(NULL, " ,-\n");
                count++;
            }
            while(token != NULL){
                size /= atoi(token);
                count++;
                token = strtok(NULL, " ,-\n");
            }
            if(count == 0){
                ssize_t w1 = write(fd2, "usage: div [n1] [n2] ...", strlen("usage: divide [n1] [n2] ..."));
                if(w1 == -1){
                    perror("write on fd2[1]");
                }
            }else if(count < 2){
                ssize_t w1 = write(fd2, "invalid arguments", strlen("invalid arguments"));
                if(w1 == -1){
                    perror("write on fd2[1]");
                }
            }else {
                //                printf("invalid %d",invalid);
                if(invalid == -1){
                    ssize_t w1 = write(fd2, "invalid math", strlen("invalid math"));
                    if(w1 == -1){
                        perror("write on fd2[1]");
                    }
                }
                else{
                    size = sprintf(buff, "division result is %zd", size);
                    ssize_t w1 = write(fd2, buff, size);
                    if(w1 == -1){
                        perror("write on fd2[1]");
                    }
                }
            }
            
        }
        else if(strcmp(token, "mult") == 0){
            int count = 0;
            size = 0;
            token = strtok(NULL, " ,-\n");
            if(token != NULL){
                size += atoi(token);
                count++;
            }
            while(token != NULL){
                size *= atoi(token);
                count++;
                token = strtok(NULL, " ,-\n");
            }
            if(count == 0){
                ssize_t w1 = write(fd2, "usage: mult [n1] [n2] ...", strlen("usage: multiply [n1] [n2] ..."));
                if(w1 == -1){
                    perror("write on fd2[1]");
                }
            }else if(count < 2){
                ssize_t w1 = write(fd2, "invalid arguments", strlen("invalid arguments"));
                if(w1 == -1){
                    perror("write on fd2[1]");
                }
            }else {
                size = sprintf(buff, "multiplication result is %zd", size);
                ssize_t w1 = write(fd2, buff, size);
                if(w1 == -1){
                    perror("write on fd2[1]");
                }
            }
            
        }
        else if(strcmp(token, "run") == 0){
            int count = 0;
            char* name;
            char* program = NULL;
            token = strtok(NULL, " ,-\n");
            name = token;
            while(token != NULL){
                program = token;
                count++;
                token = strtok(NULL, " ,-\n");
            }
            if(count == 0){
                ssize_t w1 = write(fd2, "usage: run [programName] [fileName] ", strlen("usage: run [programName] [fileName] "));
                if(w1 == -1){
                    perror("write on fd2[1]");
                }
            }else if(count > 2){
                ssize_t w1 = write(fd2, "invalid arguments", strlen("invalid arguments"));
                if(w1 == -1){
                    perror("write on fd2[1]");
                }
            }else {
                int fd3[2];
                int p3 = pipe(fd3);
                if(p3 == -1){
                    perror("pipe3");
                }
                run(name, program, fd3, fd2, processList);
                
                
            }
            
        }
        else if(strcmp(token, "list") == 0 ){
            int count = 0;
            token = strtok(NULL, " ,-\n");
            while(token != NULL){
                count++;
                token = strtok(NULL, " ,-\n");
            }
            int n = 0;
            
            if(count == 0){
//                write(1, "started", sizeof("started"));
                for (int i = 0; i < maxProcessLimit; i++) {
                    if(i==0){
                        sprintf(buff, "\n");
                    }
                    if(processList[i].pid < 1){
                        break;
                    }
//                    write(1, "loop started", sizeof("loop started"));
                    char temp[2000];
                    if(processList[i].endTime == 0){
                        processList[i].elapsedTime = ((double) clock() - processList[i].startTime);
                    }
                    n += sprintf(temp, "SNO: %d, Name: %s, PID: %d, Status: %d, StartTime: %lu, EndTime: %lu, ElapsedTime: %f \n", processList[i].sno, processList[i].name, processList[i].pid, processList[i].status, processList[i].startTime, processList[i].endTime, processList[i].elapsedTime);
//                    printf("%s", temp);
                    strcat(buff, temp);
                }
                if(n == 0){
                    ssize_t w1 = write(fd2, "No process Available", sizeof("No process Available"));
                    if(w1 == -1){
                        perror("write on fd2[1]");
                    }
                }else{
                    ssize_t w1 = write(fd2, buff, n);
                    if(w1 == -1){
                        perror("write on fd2[1]");
                    }
                }
            }else {
                ssize_t w1 = write(fd2, "invalid arguments", strlen("invalid arguments"));
                if(w1 == -1){
                    perror("write on fd2[1]");
                }
            }
            
        }
        else if(strcmp(token, "help") == 0 || strcmp(token, "man") == 0){
            int count = 0;
            token = strtok(NULL, " ,-\n");
            while(token != NULL){
                count++;
                token = strtok(NULL, " ,-\n");
            }
            if(count == 0){
                ssize_t w1 = write(fd2, "List Of Possible Commands:\nadd [num 1] [num 2] ..\nsub [num 1] [num 2] ..\ndiv [num 1] [num 2] ..\nmul [num 1] [num 2] ..\nrun [program]", strlen("List Of Possible Commands:\nadd [num 1] [num 2] ..\nsub [num 1] [num 2] ..\ndiv [num 1] [num 2] ..\nmul [num 1] [num 2] ..\nrun [program]"));
                if(w1 == -1){
                    perror("write on fd2[1]");
                }
            }else if(count > 1){
                ssize_t w1 = write(fd2, "invalid arguments", strlen("invalid arguments"));
                if(w1 == -1){
                    perror("write on fd2[1]");
                }
            }else {
                ssize_t w1 = write(fd2, buff, size);
                if(w1 == -1){
                    perror("write on fd2[1]");
                }
            }
            
        }
        else if(strcmp(token, "kill") == 0){
            int count = 0;
            char* pid;
            token = strtok(NULL, " ,-\n");
            pid = token;
            while(token != NULL){
                count++;
                token = strtok(NULL, " ,-\n");
            }
            if(count == 0){
                ssize_t w1 = write(fd2, "usage: kill [pid] ", strlen("usage: kill [pid] "));
                if(w1 == -1){
                    perror("write on fd2[1]");
                }
            }else if(count > 1){
                ssize_t w1 = write(fd2, "invalid arguments", strlen("invalid arguments"));
                if(w1 == -1){
                    perror("write on fd2[1]");
                }
            }else {
                if(strcmp(pid, "me") == 0){
                    ssize_t w1 = write(fd2, "exit", strlen("exit"));
                    if(w1 == -1){
                        perror("write on fd2[1]");
                    }
                    shutdown(fd2, 2);
                    kill(getpid(), SIGKILL);
                }
                
                
                int killed = 0;
                int num = atoi(pid);
                if(num == 0){
                    
                    if(strcmp(pid, "All") == 0 || strcmp(pid, "all") == 0){
                        ssize_t w1 = write(fd2, "exit", strlen("exit"));
                        if(w1 == -1){
                            perror("write on fd2[1]");
                        }
                    }
                    killed = kill(-111 ,SIGKILL);
                }else{
                    killed = kill(num ,SIGKILL);
                }
                if(killed == -1){
                    ssize_t w1 = write(fd2, "process not existed", strlen("process not existed"));
                    if(w1 == -1){
                        perror("write on fd2[1]");
                    }
                }else{
                    ssize_t w1 = write(fd2, "successfully killed", strlen("successfully killed"));
                    if(w1 == -1){
                        perror("write on fd2[1]");
                    }
                }
            }
            
        }
        else if(strcmp(token, "exit") == 0){
            int count = 0;
            token = strtok(NULL, " ,-\n");
            while(token != NULL){
                count++;
                token = strtok(NULL, " ,-\n");
            }
            if(count == 0){
                ssize_t w1 = write(fd2, buff, size);
                if(w1 == -1){
                    perror("write on fd2[1]");
                }
                close(fd2);
                kill(getpid(), SIGKILL);
                //                exit(0);
                
            }else {
                ssize_t w1 = write(fd2, "invalid arguments", strlen("invalid arguments"));
                if(w1 == -1){
                    perror("write on fd2[1]");
                }
            }
            
        }
        else if(strcmp(token, "disconnect") == 0){
            int count = 0;
            token = strtok(NULL, " ,-\n");
            while(token != NULL){
                count++;
                token = strtok(NULL, " ,-\n");
            }
            if(count == 0){
                ssize_t w1 = write(fd2, "exit", sizeof("exit"));
                if(w1 == -1){
                    perror("write on fd2[1]");
                }
                killAll(processList);
                close(fd2);
                kill(getpid(), SIGKILL);
                //                exit(0);
                
            }else {
                ssize_t w1 = write(fd2, "invalid arguments", strlen("invalid arguments"));
                if(w1 == -1){
                    perror("write on fd2[1]");
                }
            }
            
            
        }
        else {
            ssize_t w1 = write(fd2, "command not found", strlen("command not found"));
            if(w1 == -1){
                perror("write on fd2[1]");
            }
        }
        
    }
    else{
        ssize_t w1 = write(fd2, "write something", strlen("write something"));
        if(w1 == -1){
            perror("write on fd2[1]");
        }
    }
    
    
    
}


void killAll (struct process *processList){
    
    for (int i = 0; i < maxProcessLimit; i++) {
        if(processList[i].pid < 1 ){
            break;
        }
        
    }
    
}



void run(char* buff, char* buff2, int* fd3, int connfd, struct process *processList){
    
    fcntl(fd3[1], F_SETFD, FD_CLOEXEC);
    fcntl(fd3[0], F_SETFD, FD_CLOEXEC);
    
    int pid = fork();
    if(pid == -1){
        ssize_t w3 = write(fd3[1], "unable to start program", strlen("unable to start program"));
        if(w3 == 0){
            perror("client is dead");
        }
    }else if(pid > 0){
        close(fd3[1]);
        sleep(1);
        
        
        ssize_t r3 = read(fd3[0], buff, 100);
        if(r3 == -1){
            perror("read from fd3[0]");
        }else if(r3 == 0){
            //            ssize_t w3 = write(fd3[1], "successfully launched", strlen("successfully launched"));
            //            if(w3 == 0){
            //                perror("server is dead");
            //            }
            
            int pC = 0;
            for (int i = 0 ; i < maxProcessLimit; i++) {
                pC = i;
                if(processList[i].pid < 1 ){
                    break;
                }
            }
            
            strcpy(processList[pC].name, buff);
            processList[pC].sno = pC;
            processList[pC].pid = pid;
            processList[pC].status = 1;
            processList[pC].startTime = clock();
            //        processList[pC].endTime = 0;
            //        processList[pC].elapsedTime = 0;
            
            ssize_t w1 = write(connfd, "successfully launched", sizeof("successfully launched"));
            if(w1 == -1){
                perror("write on fd2[1]");
            }
        }else {
            ssize_t w1 = write(connfd, buff, r3);
            if(w1 == -1){
                perror("write on fd2[1]");
            }
        }
        
        
        
    }else if(pid == 0){
        close(fd3[0]);
        if(execlp(buff,
                  buff, buff2,
                  (char*) NULL) == -1){
            //            kill(getppid(), SIGKILL);
            //                        perror("Unable to run");
            
            ssize_t w3 = write(fd3[1], "unable to start program", strlen("unable to start program"));
            if(w3 == 0){
                perror("server is dead");
            }
        }
        else{
            
        }
        
    }
    
}

