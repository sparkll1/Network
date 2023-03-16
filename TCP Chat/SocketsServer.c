#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
 #include <semaphore.h>

void communicate(int, int); /* function prototype */
char myNickname[256]="coolServer";
char otherNickname[256]="otherName";
char *client_ip;

  void error(char *msg)
  {
      perror(msg);
      exit(1);
  }

//int main(int argc, char *argv[]){
//    int sockfd, newsockfd, portno, clilen, pid;
//    char buffer[256];
//    struct sockaddr_in serv_addr, cli_addr;
//    int n;
//    int m;
//    int p;
//    int counter = 0;
//    char *othersip;
//
//    char exitCommand[4] = {'e','x','i','t'};
//    char mynickname[256];
//    char othernickname[256];
//    if (argc < 2) {
//        fprintf(stderr,"ERROR, no port provided\n");
//        exit(1);
//    }
//
//    //socket is created from here
//    sockfd = socket(AF_INET, SOCK_STREAM, 0);
//
//    if (sockfd < 0) error("ERROR opening socket");
//
//    bzero((char *) &serv_addr, sizeof(serv_addr));
//    portno = atoi(argv[1]);
//
//        //@@set the second argument for nickname
//        //nickname = atoi(argv[2]);
//        printf("User name : ");
//        bzero(mynickname,255);
//        fgets(mynickname,255,stdin);
//        mynickname[strlen(mynickname)-1] = '\0';
//        printf("\nWaiting for connection...\n");
//        //specify the family,port, address
//        serv_addr.sin_family = AF_INET;
//        serv_addr.sin_port = htons(portno);
//        serv_addr.sin_addr.s_addr = INADDR_ANY;
//
//        //asking for nickname
//
//        //Bind
//        if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
//                 error("ERROR on binding");
//
//        //listen
//        listen(sockfd,5);
//        clilen = sizeof(cli_addr);
//        clilen = sizeof(cli_addr);
//        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
//        if (newsockfd < 0) {
//             error("ERROR on accept");
//        }
//
//        while (1){
//            pid_t pid = fork(); // if pid=0,child process; pid=1,parent process
////            newsockfd = accept(sockfd,
////                  (struct sockaddr *) &cli_addr, &clilen);
////            printf("trying");
//            if (newsockfd < 0)
//                error("ERROR on accept");
//            pid = fork();
//            if (pid < 0) {
//                error("ERROR on fork");
//            }
//            if (pid == 0) {
//                printf("CONNECTED");
//                close(sockfd);
//                communicate(newsockfd);
//                exit(0);
//            }
//            else {
//                close(newsockfd);
//            }
//        }
  int main(int argc, char *argv[])
  {
       int sockfd, newsockfd, portno, clilen, pid;
       struct sockaddr_in serv_addr, cli_addr;
       char buffer[256];

       int n;
       int m;
       int p;
       int counter = 0;
       char *othersip;

       char exitCommand[4] = {'e','x','i','t'};
       char othernickname[256];

       if (argc < 2) {
           fprintf(stderr,"ERROR, no port provided\n");
           exit(1);
       }
       sockfd = socket(AF_INET, SOCK_STREAM, 0);
       if (sockfd < 0)
          error("ERROR opening socket");
       memset(&serv_addr, 0, sizeof(serv_addr));
       portno = atoi(argv[1]);

       //@@set the second argument for nickname
       //nickname = atoi(argv[2]);
       printf("Provide user name: ");
       bzero(myNickname,255);
       fgets(myNickname,255,stdin);
       myNickname[strlen(myNickname)-1] = '\0';

       printf("\nWaiting for connection...\n");

       //specify the family,port, address

       serv_addr.sin_family = AF_INET;
       serv_addr.sin_addr.s_addr = INADDR_ANY;
       serv_addr.sin_port = htons(portno);

       if (bind(sockfd, (struct sockaddr *) &serv_addr,
                sizeof(serv_addr)) < 0)
                error("ERROR on binding");
       listen(sockfd,5);
       clilen = sizeof(cli_addr);

       client_ip = inet_ntoa(cli_addr.sin_addr);

       while (1) {
           newsockfd = accept(sockfd,
                 (struct sockaddr *) &cli_addr, &clilen);
           if (newsockfd < 0)
               error("ERROR on accept");

           bzero(buffer,255);
           write(newsockfd,myNickname,255);
           read(newsockfd,otherNickname,255);
//           printf("%s", otherNickname);

           pid = fork();
           if (pid < 0)
               error("ERROR on fork");
           if (pid >= 0)  {
               close(sockfd);
               communicate(newsockfd, pid);
               exit(0);
           }
           else close(newsockfd);
       } /* end of while */
       return 0; /* we never get here */
  }


//        if(pid == 0){
//            //write
//            bzero(buffer,255);
//            fgets(buffer,255,stdin);
//
//            printf("Hello from child!");
//
//            n = write(newsockfd,buffer,255);
//            if (n < 0) error("ERROR writing to socket");
//
//            if(strcmp(buffer, "exit\n") == 0){
//                printf("Exiting the program \n");
//                return(0);
//            }
//        }
////        else if(pid == 1){
////                   while(1){
////                       //write
////                       printf("Hello from parent!");
////                       }
////                   }
//        else{
//            //read
//            bzero(buffer,255);
//            printf("Hello from parent!");
//            n = read(newsockfd,buffer,255);
//            if (n < 0) error("ERROR reading from socket");
//            if(strcmp(buffer, "exit\n")==0) { //decide if receive exit signal
//                    n = write(newsockfd, "exit\n", 5);
//                    printf("Exiting the program \n");
//                    close(newsockfd);
//                    return 0;
//                }
//
//            printf("<other> : %s", buffer);
//        }
//
//    return 0;
//}

//void communicate (int sock)
//{
//   int n;
//   char buffer[256];
//   printf("commTest");
//   while(1){
//       memset(buffer, 0, 256);
//       n = read(sock,buffer,255);
//       if (n < 0) {
//           error("ERROR reading from socket");
//       }
//       printf("Here is the message: %s\n",buffer);
//
//       n = write(sock,"I got your message",18);
//
//       if (n < 0) {
//           error("ERROR writing to socket");
//       }
////          if(strcmp(buffer, "exit\n") == 0){
////              printf("Exiting the program \n");
////              return;
////          }
//   }
//}

void communicate (int sock, int pid)
{
   int n;
   char buffer[256];
   int inc = 0;

//   printf("Fork made, pid: %i\n",pid);
//   setbuf(stdout, NULL);

//          if (pid == 0) {
//              //send username to client
//              bzero(buffer,255);
//              n = write(sock,myNickname,255);
//          }
//          else {
//              //read client username
//              read(sock,otherNickname,255);
//          }

          if (pid!=0){
//              fflush(stdout);
              printf("\nConnection established with: %s (%s)\n", client_ip, otherNickname);
          }

   while(1){
       if(strcmp(buffer, "exit\n") == 0){
               n = write(sock, "exit\n", 5);
               if(pid==0){
                   printf("\nExiting the program \n");
               }
               close(sock);
               kill(0, SIGTERM);
               printf("Program");
               break;
        }

       if (pid == 0){
           //WRITE
               while(1){
                   if(strcmp(buffer, "exit\n") == 0){
                      n = write(sock, "exit\n", 5);
                      close(sock);
                      break;
                   }
                   fflush(stdout);
                   printf("<%s>: ", myNickname);

               bzero(buffer,255);
               fgets(buffer,255,stdin);
               if('\n' == buffer[strlen(buffer) - 1])
                   buffer[strlen(buffer) - 1] = '\n';

               n = write(sock,buffer,255);
               if (n < 0) error("ERROR writing to socket");


           }
       }

       else if (pid > 0) {
               //READ
               bzero(buffer,255);
               n = read(sock,buffer,255);

               if (n < 0) error("ERROR reading from socket");

               if(strcmp(buffer, "exit\n")==0) { //decide if receive exit signal
                       n = write(sock, "exit\n", 5);
                       printf("\nExiting the program \n");
                       close(sock);
                       kill(0, SIGTERM);
                       printf("Program");
                       break;
                   }

               if (buffer != 0) {
                   printf("\n<%s>: %s",otherNickname,buffer);
                   fflush(stdout);
                   printf("<%s>: ", myNickname);
                   fflush(stdout);
               }
           }
       }
}
