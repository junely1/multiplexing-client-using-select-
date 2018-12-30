/*
 * Homework (multiplexing client using select())
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUF_SIZE 100
void error_handling(char *msg);
void read_routine(int sock, char* buf);
void write_routine(int sock, char* buf);

int main(int argc, char *argv[]){
    /*initializations*/
    int sock;
    int fd_max;
    int fd_num;
    int i;
    char buf[BUF_SIZE];
    struct sockaddr_in serv_adr;
    fd_set reads, cpy_reads;    //fd set
    struct timeval timeout;     //timeout
    
    if(argc!=3) {
        printf("Usage : %s <IP> <port>\n", argv[0]);
        exit(1);
    }
    /*create and set socket*/
    sock = socket(PF_INET, SOCK_STREAM, 0);
    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_adr.sin_port = htons(atoi(argv[2]));
    if(connect(sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr)) == -1)  //connect
        error_handling("connect() error!");
    
    FD_ZERO(&reads);        //initialize all bits
    FD_SET(sock, &reads);   //set sock fd
    FD_SET(0 ,&reads);      //set stdin fd
    fd_max = sock + 1;      //total fd number
    
    while(1){
        cpy_reads = reads;      //backup
        timeout.tv_sec = 5;     //set time out value
        timeout.tv_usec = 5000;
        
        if((fd_num = select(fd_max, &cpy_reads, 0, 0, &timeout)) == -1)   //monitor
            break;                                                        //select() fail
        if(fd_num == 0){                                                  //if idle, timeout
            puts("Time-out!");
            continue;
        }
        if(FD_ISSET(0, &cpy_reads)){            //if there is something to input from stdin
            write_routine(sock, buf);           //invoke write routine (read from stdin and write to server)
        }
        else if(FD_ISSET(sock, &cpy_reads)){    //if there is a message from client to read
            read_routine(sock, buf);            //invoke read routine (read message from server)
        }
    }
    
    close(sock);  //close socket
    return 0;
}

/*
 * read message from server
 */
void read_routine(int sock, char* buf){
    int str_len = read(sock, buf, BUF_SIZE);    //read
    if(str_len == 0){                           //if done, return
        return;
    }
    buf[str_len] = 0;
    printf("Message from server: %s", buf);     //print
    
}

/*
 * read input from console and write to server
 */
void write_routine(int sock, char* buf){
    fgets(buf, BUF_SIZE, stdin);                    //std input
    if(!strcmp(buf, "q\n") || !strcmp(buf,"Q\n")){  //if quit
        shutdown(sock, SHUT_WR);                    //half close
        close(sock);
        return;
    }
    write(sock, buf, strlen(buf));                  //write to server
}

/*
 * error handler
 */
void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
