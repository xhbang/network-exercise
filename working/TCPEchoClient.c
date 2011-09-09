#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include "Practical.h"
#include "Framer.h"

int main(int argc, char *argv[]) {

  if (argc != 3) // Test for correct number of arguments
    DieWithUserMessage("Parameter(s)",
        "<Server Address/Name> [<Server Port/Service>]");

  char *server = argv[1];     // First arg: server address/name
  char *service = argv[2]; 

  // Create a connected TCP socket
  int sock = SetupTCPClientSocket(server, service);
  if (sock < 0)
    DieWithUserMessage("SetupTCPClientSocket() failed", "unable to connect");

  FILE *str = fdopen(sock,"r+");
  char buffer[MAXSTRINGLENGTH];
  //while((scanf("%s",buffer) != EOF)){
  while(gets(buffer) && strlen(buffer) != 0){
      int sSize;
      int stringLen = strlen(buffer); // Determine input length
      if((sSize = PutMsg(buffer, stringLen, str)) < 0)
          DieWithSystemMessage("PutMsg() failed");
      //printf("Sending %d-byte..\n",sSize);
      /*
      GetNextMsg(str,buffer,MAXSTRINGLENGTH);
      printf("get echo\n");
      printf("%s\n",buffer);
      */
      memset(buffer,0,sizeof(buffer));
  }
  fclose(str);
  printf("quit.\n");
  return 0;
}
