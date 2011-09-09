#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "client.h"



int main(int argc, char *argv[])
{
  int cstcp,csudp,ccudp;
  int server_port=8000;	//c-s tcp port
  int CC_port=5000;	//c-c udp port
  struct hostent *hostPtr = NULL;
  struct sockaddr_in CS_link = { 0 };
  struct sockaddr_in CC_link = { 0 };
  char hostname[80] = "";
  char buf[bufsize+1]="";
  char *msg;
  char *S_addr = NULL;
  int recvn;
  int childPid;

  if (2 != argc)
  {
    fprintf(stderr, "Usage: %s <serverAddress> \n", argv[0]);
    exit(1);
  }

  S_addr = argv[1];
  cstcp = tcpSocket();

  GetHostName(hostname, sizeof(hostname));	//get local host
  CreateSockAddr(S_addr,&CS_link,server_port);	//C-S
  Connect(cstcp,(struct sockaddr*) &CS_link);
  msg="Client OPC 0.0.1\n";
  Send(cstcp,msg);
  Recv(cstcp,buf);
  printf("%s",buf);
  printf("Connect to server successfully,please input your nickname\n");
  Fgets(buf);
  Send(cstcp,buf);	//send user name
  bzero(buf,bufsize);
  Recv(cstcp,buf);
  while(strncmp(buf,":x",2)==0)
  {
    printf("%s","The name has been used,please change your name:\n");
    Fgets(buf);
    Send(cstcp,buf);	//send user name
    bzero(buf,bufsize);
    Recv(cstcp,buf);
  }
  printf("%s",buf);
  Send(cstcp,":l");	//send com to get userlist form server
  printf("OnLine Users:\n");
  bzero(buf,bufsize);
  Recv(cstcp,buf);	//get userlist from server
  printf("%s",buf);

  childPid = fork();

  switch (childPid)
  {
  case -1: // ERROR
    perror("fork()");
    exit(1);
  case 0: // child process
    while(1)
    {
      bzero(buf,bufsize);
      if(Recv(cstcp,buf)>0)
        if(strncmp(buf,":q",2)==0)


          exit(0);	//get userlist from server
      printf("%s",buf);

    }
  default :
    while(1)
    {
      bzero(buf,bufsize);
      Fgets(buf);
      Send(cstcp,buf);
      if(strncmp(buf,":q",2)==0)
        exit(0);

    }
  }

  return EXIT_SUCCESS;
}	
