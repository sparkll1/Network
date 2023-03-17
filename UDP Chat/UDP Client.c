/* client.c
 * Rose-Hulman Protocol Project
 *
 * By Sadie Park and Jesus Capo
 *
 * This client code allows for communication to a server using UDP. This code employs
 * the RHP (Rose-Hulman Protocol) which is built above UDP for purposes of communicating
 * metadata between server and client as well as performing error checks (i.e., checksums).
 * This provides functionality to communication which UDP on its own lacks.
 *
 */

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>


#define SERVER "137.112.38.47"
#define MESSAGE "hello"
#define PORT 1874
#define BUFSIZE 1024

//set parameters for RHP
#define SOURCEPORT 2267 //2267, CM of Jesus Capo
#define DSTPORT 312 //312

#define VERSION 7
#define TYPE 0 // RHP = 0, RHMP = 2

// set parameters for RHMP
#define COMMID 2541

/* get_checksum
 * Purpose: Return 16-bit internet checksum of given datagram
 * Inputs: (uint8_t) *buffer: buffer  with original datagram words, (int) nBytes: number of bytes in input datagram
 * Output: (uint16_t) checksum: calculated checksum
 */
uint16_t get_checksum(uint8_t *buffer, int nBytes){

  uint16_t new_buffer_for_checksum[nBytes/2];
  for (int i = 0; i < nBytes/2; i++){
    new_buffer_for_checksum[i] = (uint16_t)(buffer[2*i]<<8) + (uint16_t)buffer[2*i+1];
  }

  uint32_t checksum = 0;
  for (int i = 0; i < nBytes/2; i++){
    checksum = checksum + new_buffer_for_checksum[i];
    if (checksum >= 0x010000){
      checksum = checksum & 0xFFFF;
      checksum += 1;
    }
  }
  checksum = 0xFFFF - (uint16_t)checksum;
  return checksum;
}

/* create_RHPmessage
 * Purpose: Creates the frame for the RHP protocol. It uses the predefined values
 * defined at the very top of the code.
 * Inputs: (char) *message: payload, (uint8_t) *buffer: send_buffer where values to send are assigned in
 * Output: (int) numbBytes: total number of bytes
 */
int create_RHPmessage(char *message, uint8_t *buffer){
  int numBytes = 0;

  //version and type: 0-7th bit
  uint8_t type = ((TYPE & 0xF) <<6);
  uint8_t version = (VERSION & 0x3F);
  uint8_t version_and_type = (uint8_t)(version + type);
  buffer[0] = version_and_type;

  //destination port : 8 - 23th bit
  uint16_t dstPort = (uint16_t) DSTPORT;
  uint8_t upper8_dstPort = (dstPort & 0xFF00)>>8;
  uint8_t lower8_dstPort = (dstPort) & 0xFF;
  buffer[2] = upper8_dstPort;
  buffer[1] = lower8_dstPort;

  //source port : 24 - 39th bit

  uint16_t srcPort = (uint16_t) SOURCEPORT;
  uint8_t upper8_srcPort = (srcPort & 0xFF00)>>8;//shift right to store the upper 8 bits
  uint8_t lower8_srcPort = (srcPort) & 0xFF;
  buffer[4] = upper8_srcPort;
  buffer[3] = lower8_srcPort;
  
  //length : 40-46th bit, type : 47th bit.
  uint8_t length =(strlen(MESSAGE)) & 0xFF;
  uint8_t len_type = (length);//shift right than add type to get 8 bit len_type.
  buffer[5] = len_type;

  //payload : 48 - 48+len-1th bit. 1 letter = 1 byte.
  for (int i = 0 ; i < length; i++){
    buffer[6 + i] = MESSAGE[i];
  }
  numBytes = 6 + length;

  //buffer(optional) : if length of message odd, put 8bit of zeros to make octets even.
  if(length % 2 == 1){
    uint8_t buf = 0xFF;
    buffer[5+length+1] = buf;
    numBytes = 7 + length;
  }

  //checksum : last 16 bits.
  uint16_t checksum = get_checksum(buffer, numBytes);
  uint8_t upper8_checksum = (checksum >> 8) & 0xFF;
  uint8_t lower8_checksum = checksum & 0xFF;
  buffer[numBytes] = upper8_checksum;
  buffer[numBytes+1] = lower8_checksum;
  numBytes = numBytes + 2;
  
  return numBytes;
}

/* printRHP
 * Purpose: Prints out the associated data and payload from the message received in response from the server
 * Inputs: (uint8_t) *buffer: buffer of received message to decode, (int) nBytes: number of bytes in received message
 * Output: (int) 1 if successful checksum, 0 if unsuccessful checksum
 */
bool printRHP(uint8_t *buffer, int nBytes){

  // analyze checksum
  uint16_t calc_checksum = get_checksum(buffer, nBytes-2);
  uint16_t returned_checksum = (buffer[nBytes-2]<<8) + buffer[nBytes-1];
  if (calc_checksum != (returned_checksum)){
    printf("\nBAD CHECKSUM, INVALID MESSAGE. Received 0x%hx but should be 0x%hx\n", returned_checksum, calc_checksum);
    printf("INVALID MESSAGE SHOWN BELOW:");
  }
  else {
      printf("\nValid Message Received:");
  }
  printf("\nComplete Hex Message : 0");
  for (int i = 0; i < nBytes; i++){
    printf("%hx ", buffer[i]);
  }

  //number of bytes
  printf("\n");
  printf("Total Number of Bytes : %d\n", nBytes);

  //version
  uint8_t version = (buffer[0] & 0x3F); //bitwise AND to get version: 0b00111111
  printf("RHP Version : %d\n", version);

  //type
  uint8_t type = (buffer[0] & 0xC0); //bitwise AND to get type: 0b11000000
  printf("RHP Type : %d\n", type);

  //Destination port
  uint16_t srcPt = ((uint16_t)buffer[2]<<8) +  ((uint8_t)buffer[1]);
  printf("Destination Port : %d\n", srcPt);

  //Source port
  uint16_t dstPt = ((uint16_t)buffer[4] << 8) + ((uint8_t)buffer[3]);
  printf("Source Port : %d\n", dstPt);

  //length
  uint8_t len = ((uint16_t)buffer[5]);
  printf("Payload Number of Bytes : %d\n", len);

  //calculate whether there is a padding buffer
  int padding_buffer = 0;

  if(len%2==1){
      padding_buffer = 1;
  }

  //payload
  printf("Payload : ");
  for (int i = 6; i < nBytes - (2+padding_buffer); i++){
    printf("%c", buffer[i]);
  }

  //buffer
  if (padding_buffer) {
      printf("\nHeader Padding Buffer : Yes");
  }
  else {
      printf("\nHeader Padding Buffer : No");
  }

  //checksum
  printf("\n");
  uint16_t lowchksum = (uint16_t)buffer[nBytes-1];
  uint16_t upchksum = (uint16_t)buffer[nBytes-2];
  printf("Checksum : 0x%hx%hx\n", upchksum, lowchksum );
  
  
  

  return (calc_checksum == returned_checksum);
}
//second part of the project
//type will be 0d2 0b0010 (Id Request) and 0d8 0b1000(Messae Request)


int create_RHMPMessage(uint8_t *buffer, int rhmp_type)
{
    uint16_t srcPort = (uint16_t) SOURCEPORT;

    //RHMP Portion
    int rhmp_buffsize = 2;
//    if (rhmp_type==2||rhmp_type==8){
//        int rhmp_buffsize = 2;
//    }
//    else {
//        int rhmp_buffsize = 3;
//    }

    uint8_t RHMPBuffer[rhmp_buffsize];

    // Type and CommId
	uint8_t buffer1 = ((srcPort) & 0x0FF0)>>4; //upper 2 bytes of COMMID
	uint8_t buffer0 = (((srcPort & 0x000F)<<4) + rhmp_type); //leftmost byte: lower byte of COMMID, rightmost byte: rhmp_type
	
	RHMPBuffer[0] = buffer0;
	RHMPBuffer[1] = buffer1;

	int numBytes = 0;

	//RHP PORTION

    //version and type: 0-7th bit
    uint8_t rhp_type = ((2 & 0xF) <<6);
    uint8_t version = (VERSION & 0x3F);
    uint8_t version_and_type = (uint8_t)(version + rhp_type);
    buffer[0] = version_and_type;

    //destination port : 8 - 23th bit
    uint16_t dstPort = (uint16_t) DSTPORT;
    uint8_t upper8_dstPort = (dstPort & 0xFF00)>>8;
    uint8_t lower8_dstPort = (dstPort) & 0xFF;
    buffer[2] = upper8_dstPort;
    buffer[1] = lower8_dstPort;

    //source port : 24 - 39th bit

    uint8_t upper8_srcPort = (srcPort & 0xFF00)>>8;//shift right to store the upper 8 bits
    uint8_t lower8_srcPort = (srcPort) & 0xFF;
    buffer[4] = upper8_srcPort;
    buffer[3] = lower8_srcPort;

    buffer[5] = RHMPBuffer[0];
    buffer[6] = RHMPBuffer[1];



    //buffer(optional) : if length of message odd, put 8bit of ones to make octets even.
    buffer[7] = 0xFF;
    numBytes = 8;

    //checksum : last 16 bits.
    uint16_t checksum = get_checksum(buffer, numBytes);
    uint8_t upper8_checksum = (checksum >> 8) & 0xFF;
    uint8_t lower8_checksum = checksum & 0xFF;
    buffer[numBytes] = upper8_checksum;
    buffer[numBytes+1] = lower8_checksum;
    numBytes = numBytes + 2;

    return numBytes;
}
int printRHMPMessage(uint8_t *buffer, int nBytes)
{
	
	printf("\n");

    // analyze checksum
    uint16_t calc_checksum = get_checksum(buffer, nBytes-2);
    uint16_t returned_checksum = (buffer[nBytes-2]<<8) + buffer[nBytes-1];
    if (calc_checksum != (returned_checksum)){
    printf("\nBAD CHECKSUM, INVALID MESSAGE. Received 0x%hx but should be 0x%hx\n", returned_checksum, calc_checksum);
    printf("INVALID MESSAGE SHOWN BELOW:");
    }
    else {
      printf("\nValid Message Received:");
    }
    printf("\nComplete Hex Message : 0");
    for (int i = 0; i < nBytes; i++){
    printf("%hx ", buffer[i]);
    }

    //number of bytes
    printf("\n");
    printf("Total Number of Bytes : %d\n", nBytes);

    //version
    uint8_t version = (buffer[0] & 0x3F); //bitwise AND to get version: 0b00111111
    printf("RHP Version : %d\n", version);

    //type
    uint8_t type = (buffer[0] & 0xC0)>>6; //bitwise AND to get type: 0b11000000
    printf("RHP Type : %d\n", type);

    //Destination port
    uint16_t srcPt = ((uint16_t)buffer[2]<<8) +  ((uint8_t)buffer[1]);
    printf("Destination Port : %d\n", srcPt);

    //Source port
    uint16_t dstPt = ((uint16_t)buffer[4] << 8) + ((uint8_t)buffer[3]);
    printf("Source Port : %d\n", dstPt);

    //calculate whether there is a padding buffer
    int padding_buffer = 0;

//    if(len%2==1){
//      padding_buffer = 1;
//    }

    //PAYLOAD

    //payload metadata
    printf("\nPayload Metadata: \n");

    //rhmp type
    uint16_t rhmp_type = buffer[5] & 0xF;
    printf("RHMP Type : %d - ", rhmp_type);
    if (rhmp_type == 9){
        printf("Message Response\n");
    }
    else if (rhmp_type==3) {
        printf("ID Response\n");
    }

    //rhmp commID
    uint16_t commID = ((uint16_t)buffer[6]<<4) +  ((buffer[5] & 0xF0)>>4);
    printf("COMMID : %d\n", commID);

    uint16_t rhmp_length = 0;

    if (rhmp_type == 9){
        //rhmp payload length
        uint16_t rhmp_length = buffer[7];
        printf("RHMP Payload Number of Bytes : %d\n", rhmp_length);

        printf("RHMP Payload: ");

        for (int i = 8; i < nBytes - (2+padding_buffer); i++){
        printf("%c", buffer[i]);
        }
    }
    else if (rhmp_type==3) {
        uint16_t rhmp_length = 4;
        printf("RHMP Payload: ");

        uint32_t rhmp_controlID = ((uint32_t)buffer[10] << 24) + ((uint32_t)buffer[9] << 16) + ((uint32_t)buffer[8] << 8) + ((uint8_t)buffer[7]);
        printf("%i\n", rhmp_controlID);
    }

    if(rhmp_length%2==1){
        padding_buffer = 1;
    }

    //buffer
    if (padding_buffer) {
      printf("\nHeader Padding Buffer : Yes");
    }
    else {
      printf("\nHeader Padding Buffer : No");
    }

    //checksum
    printf("\n");
    uint16_t lowchksum = (uint16_t)buffer[nBytes-1];
    uint16_t upchksum = (uint16_t)buffer[nBytes-2];
    printf("Checksum : 0x%hx%hx\n", upchksum, lowchksum );

    return (calc_checksum == returned_checksum);
    return 0;
}



/* main
 * Purpose: Sets up UDP communication for sending a single RHP frame.
 * Performs a single resend attempt if the sending of the frame was
 * unsuccessful (i.e., bad checksum).
 */
int main() {
    int clientSocket, nBytes;
    uint8_t sendbuffer[BUFSIZE], receivebuffer[BUFSIZE];
    struct sockaddr_in clientAddr, serverAddr;

    /*Create UDP socket*/
    if ((clientSocket = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("cannot create socket");
        return 0;
    }

    /* Bind to an arbitrary return address.
     * Because this is the client side, we don't care about the address
     * since no application will initiate communication here - it will
     * just send responses
     * INADDR_ANY is the IP address and 0 is the port (allow OS to select port)
     * htonl converts a long integer (e.g. address) to a network representation
     * htons converts a short integer (e.g. port) to a network representation */
    memset((char *) &clientAddr, 0, sizeof (clientAddr));
    clientAddr.sin_family = AF_INET;
    clientAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    clientAddr.sin_port = htons(0);

    if (bind(clientSocket, (struct sockaddr *) &clientAddr, sizeof (clientAddr)) < 0) {
        perror("bind failed");
        return 0;
    }

    /* Configure settings in server address struct */
    memset((char*) &serverAddr, 0, sizeof (serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr(SERVER);
    memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);

    /* send a message to the server */
    //RHP CONTROL MESSAGE
    printf("Sending an RHP control message to the server : \n");
    char *message = MESSAGE;
    int nBytes_from_client = create_RHPmessage(message, sendbuffer);
    if (sendto(clientSocket, sendbuffer, nBytes_from_client, 0,
            (struct sockaddr *) &serverAddr, sizeof (serverAddr)) < 0) {
        perror("sendto failed");
        return 0;
    }
    printf("message sent : %s\n", message);
    /* Receive message from server */
    nBytes = recvfrom(clientSocket, receivebuffer, BUFSIZE, 0, NULL, NULL);

    if (printRHP(receivebuffer, nBytes) == false){
      printf("RESENDING MESSAGE.\n");
      int nBytes_from_client = create_RHPmessage(message, sendbuffer);
         if (sendto(clientSocket, sendbuffer, nBytes_from_client, 0,
                 (struct sockaddr *) &serverAddr, sizeof (serverAddr)) < 0) {
             perror("sendto failed");
             return 0;
         }
      nBytes = recvfrom(clientSocket, receivebuffer, BUFSIZE, 0, NULL, NULL);
      printRHP(receivebuffer, nBytes);
    }

    printf("\n-----------------------------------------------------------------\n");

    //RHMP message_request
    printf("Sending an RHMP Message Request to the server : \n");
    nBytes_from_client = create_RHMPMessage(sendbuffer, 8);

    if (sendto(clientSocket, sendbuffer, nBytes_from_client, 0,
            (struct sockaddr *) &serverAddr, sizeof (serverAddr)) < 0) {
        perror("sendto failed");
        return 0;
    }
//    printf("RHMP Message Request Sent");
    /* Receive message from server */
    nBytes = recvfrom(clientSocket, receivebuffer, BUFSIZE, 0, NULL, NULL);
//    printf(receivebuffer);


    if (printRHMPMessage(receivebuffer, nBytes) == false){
      printf("RESENDING MESSAGE.\n");
      nBytes_from_client = create_RHMPMessage(sendbuffer, 8);
         if (sendto(clientSocket, sendbuffer, nBytes_from_client, 0,
                 (struct sockaddr *) &serverAddr, sizeof (serverAddr)) < 0) {
             perror("sendto failed");
             return 0;
         }
      nBytes = recvfrom(clientSocket, receivebuffer, BUFSIZE, 0, NULL, NULL);
      printRHMPMessage(receivebuffer, nBytes);
    }

    printf("\n-----------------------------------------------------------------\n");

    //RHMP ID_request
       printf("Sending an RHMP ID Request to the server : \n");
       nBytes_from_client = create_RHMPMessage(sendbuffer, 2);

       if (sendto(clientSocket, sendbuffer, nBytes_from_client, 0,
               (struct sockaddr *) &serverAddr, sizeof (serverAddr)) < 0) {
           perror("sendto failed");
           return 0;
       }

       /* Receive message from server */
       nBytes = recvfrom(clientSocket, receivebuffer, BUFSIZE, 0, NULL, NULL);
   //    printf(receivebuffer);


       if (printRHMPMessage(receivebuffer, nBytes) == false){
         printf("RESENDING MESSAGE.\n");
         nBytes_from_client = create_RHMPMessage(sendbuffer, 2);
            if (sendto(clientSocket, sendbuffer, nBytes_from_client, 0,
                    (struct sockaddr *) &serverAddr, sizeof (serverAddr)) < 0) {
                perror("sendto failed");
                return 0;
            }
         nBytes = recvfrom(clientSocket, receivebuffer, BUFSIZE, 0, NULL, NULL);
         printRHMPMessage(receivebuffer, nBytes);
       }


    close(clientSocket);
    return 0;
}
