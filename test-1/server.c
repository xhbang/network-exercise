#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <pthread.h>
#include "server.h"
#include "link.h"

typedef struct threadargs
{
  int sock;
  LList *list;
}
threadargs;

int getuser(char *buf,char *username,LList *userlist);
char iscmd(const char * message);
int useratlist(LList *userlist,char *username);
int findclientsock(LList *userlist,char *username);
char *getsecond(char *message);
char *getthird(char *message);
void domessage(char *dest,char * userfrom,char *message);
void accept_cli(threadargs *newargs);

int main(int argc, char *argv[])
{
  int sctcp;
  char hostname[80] = "";
  struct sockaddr_in SC_link = { 0 };
  int server_port=8000;
  int childPid=0;
  static LList userlist;
  InitList(&userlist);
  pthread_t id;
  int ret;

  printf("Server is starting\n");
  sctcp=tcpSocket();	//client-server comunicate with tcp
  Setsockopt(sctcp);		//set SO_REUSEADDR,SO_LINGER opt
  GetHostName(hostname, sizeof(hostname));
  CreateSockAddr(hostname,&SC_link,server_port);
  Bind(sctcp, (struct sockaddr *) &SC_link,sizeof(SC_link));
  Listen(sctcp);
  printf("Server started successfully and it is ready now\n");
  printf("Now entered listening mode\n");
  for (;;)
  {
    struct sockaddr_in client_sockaddr = { 0 };
    int cli_socket, cli_sock2,clientLength = sizeof(client_sockaddr);
    (void) memset(&client_sockaddr, 0, sizeof(client_sockaddr));
    cli_socket = Accept(sctcp,(struct sockaddr *) &client_sockaddr, &clientLength);
    if (-1 == cli_socket)
    {
      perror("accept()");
    }
    threadargs newargs;
    newargs.sock=cli_socket;
    newargs.list=&userlist;
    //      accept_cli(&newargs);
    ret=pthread_create(&id,NULL,(void *)accept_cli,&newargs);
    if(ret!=0)
      perror("thread create error");
  }


  return EXIT_SUCCESS;
}

void accept_cli(threadargs *newargs)
{
  LList *userlist=newargs->list;
  int cli_socket=newargs->sock;
  int cli_sock2;
  int recvn;	//num of recv bytes
  char buf[bufsize+1]="";
  char buf2[bufsize+1]="";
  char cmd;
  client newcli;
  client lastuser;	//the user which client talk to last time
  bzero(&newcli,sizeof(client));
  bzero(&lastuser,sizeof(client));
  if(-1==Recv(cli_socket,buf))
    pthread_exit(NULL);
  printf("%s",buf);
  if(-1==Send(cli_socket,"Server OCP v0.0.1\n"))
    pthread_exit(NULL);
  bzero(buf,bufsize);
  if(-1==Recv(cli_socket,buf))
    pthread_exit(NULL);
  Rtrim(buf);
  while(useratlist(userlist,buf)==0) //username has been used
  {
    if(-1==Send(cli_socket,":x"))
      pthread_exit(NULL);
    bzero(buf,bufsize);
    if(-1==Recv(cli_socket,buf))
      pthread_exit(NULL);

    Rtrim(buf);
  }
  Send (cli_socket,"Longin Successfully\n");

  strncpy(newcli.nick,buf,strlen(buf));
  newcli.sock=cli_socket;
  ListInsert(userlist,newcli);


  while(1)
  {
    LNode *node=userlist->head->next; //use in :a
    bzero(buf,bufsize);
    if(Recv(cli_socket,buf)==-1)	//client offline
    {
      ListDelete(userlist,cli_socket);
      pthread_exit(NULL);
    }
    if((cmd=iscmd(buf))==0)	//if message body contains only message(not have a command)
    {
      if(useratlist(userlist,lastuser.nick)==0)

      {
        cli_sock2=lastuser.sock;
        domessage(buf2,newcli.nick,buf);
        if(-1==Send(cli_sock2,buf2))
          pthread_exit(NULL);
      }

      else
      {
        if(-1==Send(cli_socket,"The user you want to talk isn't online\n"))
          pthread_exit(NULL);
      }
      continue;
    }
    switch(cmd)
    {
    case 'l':
      bzero(buf,bufsize);
      LNode *user=userlist->head->next;
      while(user!=NULL)
      {
        strcat(buf,user->e.nick);
        strcat(buf,"\n");
        user=user->next;
      }

      if(-1==Send(cli_socket,buf))
        pthread_exit(NULL);
      break;
    case 'u':	//client change user which will talk to
      if(getuser(buf,lastuser.nick,userlist)!=-1)	//buf client's message //buf2 username
      {cli_sock2=findclientsock(userlist,lastuser.nick);
        lastuser.sock=cli_sock2;
        if(getthird(buf)!=NULL)
        {
          domessage(buf2,newcli.nick,getthird(buf));
          if(-1==Send(cli_sock2,buf2))
            pthread_exit(NULL);
        }
      }
      else
      {
        if(-1==Send(cli_socket,"You doesn't specify a user,or the user you want to talk to isn't online\n"))
          pthread_exit(NULL);
      }
      break;
    case 'q':	//client quit
      if(-1==Send(cli_socket,buf))
        pthread_exit(NULL);
      ListDelete(userlist,cli_socket);
      close(cli_socket);
      pthread_exit(NULL);
      break;
    case 'a':	//client talk to all user

      while(node!=NULL)
      {
        client user=node->e;

        cli_sock2=user.sock;
        if (cli_sock2!=cli_socket)	//don't send the message to your
          {   if(getsecond(buf)!=NULL)	//if the message body only contains the :a string

            domessage(buf2,newcli.nick,getsecond(buf));
          if(-1==Send(cli_sock2,buf2))
            pthread_exit(NULL);
        }

        node=node->next;
      }
      break;
    default :
      if(-1==Send(cli_socket,"Sever can't recognize your command\n"))
        pthread_exit(NULL);

    }
  }

}

int getuser(char *buf,char *username,LList *userlist)	//if the user is online(int the userlist),set the username string,return 0,else return -1
{
  const char delimiters[] = " ";
  char *token, *cp;
  cp = strdup(buf);
  token = strtok (cp, delimiters);
  token = strtok (NULL, delimiters);	//token=username
  if(token==NULL) return -1;
  strncpy(username,token,namesize);
  if(username[strlen(username)-1]='\n')
    username[strlen(username)-1]=0;
  return useratlist(userlist,username);

}

char iscmd(const char * message)	//get command
{
  char cmd;
  if((cmd=message[0])!=':')
    return 0;
  return message[1];
}

int useratlist(LList *userlist,char *username)
{
  LNode *node=userlist->head->next;
  client user;
  while(node!=NULL)
  {
    user=node->e;
    if(strncmp(user.nick,username,strlen(username))==0)
      return 0;
    else node=node->next;
  }
  return -1;

}

int findclientsock(LList *userlist,char *username)
{
  LNode *node=userlist->head->next;
  client user;
  while(node!=NULL)
  {
    user=node->e;
    if(strncmp(user.nick,username,strlen(username))==0)
      return user.sock;
    else node=node->next;
  }
  return -1;
}

char *getsecond(char *message)
{
  const char delimiters[] = " ";
  char *token, *cp;
  cp = strdup(message);
  token = strtok (cp, delimiters);
  token = strtok (NULL, delimiters);
  return token;
}

char *getthird(char *message)
{
  const char delimiters[] = " ";
  char *token, *cp;
  cp = strdup(message);
  token = strtok (cp, delimiters);
  token = strtok (NULL, delimiters);
  token = strtok (NULL, delimiters);
  return token;
}



void domessage(char *dest,char * userfrom,char *message)
{
  strcpy(dest,"From ");
  strcat(dest,userfrom);
  strcat(dest,": ");
  strcat(dest,message);
}
