#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/utsname.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>

#define bufsize 255
#define namesize 20

int tcpSocket()
{
  int n;

  if ( (n = socket(PF_INET,SOCK_STREAM,0))==-1)
  {
    perror("TCP Socket error");
    exit(1);
  }
  return(n);
}


void Setsockopt(int s)
{
  int on = 1;
  struct linger linger = { 0 };
  linger.l_onoff = 1;
  linger.l_linger = 30;

  if ( setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (const char *) &on, sizeof(on))==-1)
  {
    perror("Setsockopt(...,SO_REUSEADDR,...)");
    exit(1);
  }

  if ( setsockopt(s, SOL_SOCKET, SO_LINGER, (const char *) &linger, sizeof(linger))==-1)
  {
    perror("Setsockopt(...,SO_LINGER,...)");
    exit(1);
  }
}

int Bind(int sockfd, const struct sockaddr *my_addr, socklen_t addrlen)
{
  return bind(sockfd,my_addr,addrlen);
}

void Listen(int s)
{
  if (-1 == listen(s,5))
  {
    perror("Listen()");
    exit(1);
  }
}

int Accept(int s, struct sockaddr *addr, socklen_t *addrlen)
{
  int newSocket;
  if ((newSocket=accept(s, addr, addrlen))==-1)
  {
    perror("Accept()");
    exit(1);
  }
  return newSocket;
}




void Connect(int  sockfd,  const  struct sockaddr *sock_addr)
{
  if (-1 == connect(sockfd, sock_addr, sizeof(*sock_addr)))
  {
    printf("Server haven't started\n");
    exit(1);
  }
}

void GetHostName(char *buffer, int length)
{
  struct utsname sysname = { 0 };
  int status = 0;

  status = uname(&sysname);
  if (-1 != status)
  {
    strncpy(buffer, sysname.nodename, length);
  }
  else
  {
    perror("GetHostName()");
    exit(1);
  }
}

void CreateSockAddr(const char *hostname,struct sockaddr_in *sockaddress,int port)
{
  struct hostent *host = NULL;
  host = gethostbyname(hostname);
  if (NULL == host)
  {
    host = gethostbyaddr(hostname,
                         strlen(hostname), AF_INET);
    if (NULL == host)
    {
      perror("Error resolving server address");
      exit(1);
    }
  }
  (void) memset(sockaddress, 0, sizeof(sockaddress));
  (void) memcpy(&((*sockaddress).sin_addr), host->h_addr, host->h_length);
  sockaddress->sin_addr.s_addr=htonl(INADDR_ANY);
  sockaddress->sin_family = AF_INET;
  sockaddress->sin_port = htons(port);
}

ssize_t Send(int s, const void *buf)
{
  ssize_t sendn;
  if( -1==(sendn=send(s, buf, strlen(buf), 0)))
  {
    perror("Send()");
    close(s);
  }
  return sendn;
}


ssize_t Recv(int s, void *buf)
{
  ssize_t recvn;

  if( -1==(recvn= recv(s, buf, bufsize, 0)))
  {
    perror("Recv()");
    close(s);
  }
  return recvn;
}



char *Fgets(char *s)
{
  bzero(s,bufsize);
  if(fgets(s,bufsize,stdin)==NULL)
  {
    perror("Fgets()");
    exit(1);
  }
  return s;
}

void Rtrim(char *buf)
{
  int i;
  for(i=0;i<255;i++)
  {
    if(buf[i]=='\n')
    {
      buf[i]=0;
      break;
    }
  }
}
