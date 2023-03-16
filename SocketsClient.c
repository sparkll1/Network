#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <signal.h>
#include<arpa/inet.h>


void communicate(int, int); /* function prototype */
char myNickname[256]="coolClient";
char otherNickname[256];
char serverIP[256];



   void error(char *msg)
   {
       perror(msg);
       exit(0);
   }

   int main(int argc, char *argv[])
   {
     int newsockfd, portno, n, pid, sockfd;
     struct sockaddr_in serv_addr, cli_addr;

     char exitCommand[4] = {'e','x','i','t'};
     char othernickname[256];

//     struct sockaddr_in serv_addr;
     struct hostent *server;
     char buffer[256];
     if (argc < 3) {
        fprintf(stderr,"usage %s hostname port\n", argv[0]);
        exit(0);
     }
     portno = atoi(argv[2]);

     //@@set the second argument for nickname
      //nickname = atoi(argv[2]);
      printf("Provide user name: ");
      bzero(myNickname,255);
      fgets(myNickname,255,stdin);
      myNickname[strlen(myNickname)-1] = '\0';

     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0)
         error("ERROR opening socket");
     server = gethostbyname(argv[1]);
     if (server == NULL) {
         fprintf(stderr,"ERROR, no such host\n");
         exit(0);
     }
     bzero((char *) &serv_addr, sizeof(serv_addr));
     serv_addr.sin_family = AF_INET;
     bcopy((char *)server->h_addr,
          (char *)&serv_addr.sin_addr.s_addr,
          server->h_length);
     serv_addr.sin_port = htons(portno);
     if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0)
         error("ERROR connecting");

  //   server_ip = inet_aton(serv_addr.sin_addr);
	inet_ntop(AF_INET, &(serv_addr.sin_addr), serverIP, 256);
	 
	 

     while (1) {
//            newsockfd = accept(sockfd,
//                  (struct sockaddr *) &cli_addr, &clilen);
//            if (newsockfd < 0)
//                error("ERROR on accept");
            pid = fork();
            if (pid < 0)
                error("ERROR on fork");
            if (pid >= 0)  {
//                close(sockfd);
                communicate(sockfd, pid);
                exit(0);
            }
//            else close(newsockfd);
        } /* end of while */
            return 0; /* we never get here */

   }

   void communicate (int sock, int pid)
   {
      int n,inc;
      char buffer[256];

      if (pid==0){
          //send username to client
          bzero(buffer,255);
          n = write(sock,myNickname,255);
       }
      else {
          //read client username
          read(sock,otherNickname,255);
      }

      if (pid!=0){
//          fflush(stdout);
          printf("\nConnection established with: (%s) (%s)\n", otherNickname, serverIP);
//          printf("<%s>: ", myNickname);
      }
      else{
          inc=inc+1;
      }


      while(1){
//          buffer[strcspn(buffer, "\n")] = '\0';
          if(strcmp(buffer, "exit\n") == 0){
                 n = write(sock, "exit\n", 5);
                 if(pid==0){
                     printf("Exiting the program \n");
                 }
                 close(sock);
                 break;
          }

          if (pid == 0){
              fflush(stdout);
              printf("<%s>: ", myNickname);
                  while(1){
                  bzero(buffer,255);
                  fgets(buffer,255,stdin);
                  fflush(stdout);
                  printf("<%s>: ", myNickname);
                  if('\n' == buffer[strlen(buffer) - 1])
                      buffer[strlen(buffer) - 1] = '\n';

                  n = write(sock,buffer,strlen(buffer));
                  if (n < 0) error("ERROR writing to socket");

                  if(strcmp(buffer, "exit\n") == 0){
                      n = write(sock, "exit\n", 5);
                      printf("Exiting the program \n");
                      close(sock);
                      break;
                  }
              }
          }

          else if (pid > 0) {
                  //read
                  bzero(buffer,255);
                  n = read(sock,buffer,255);
                  if (n < 0) error("ERROR reading from socket");

                  if(strcmp(buffer, "exit\n")==0) { //decide if receive exit signal
                          n = write(sock, "exit\n", 5);
                          printf("Exiting the program \n");
                          close(sock);
                          break;
                      }

                  if (buffer != 0) {
                      fflush(stdout);
                      printf("\n<%s>: %s",otherNickname,buffer);
                      fflush(stdout);
                      printf("<%s>: ", myNickname);
                      fflush(stdout);
                  }
              }
          }
   }
