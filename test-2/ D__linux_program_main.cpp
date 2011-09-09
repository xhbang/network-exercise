/*************************************************************************** 
                          main.cpp  -  description 
                             ------------------- 
    begin                : Áù  1ÔÂ 29 23:36:00 CST 2005 
    copyright            : (C) 2005 by lgl 
    email                : dreamer_888@163.com 
 ***************************************************************************/ 
 
/*************************************************************************** 
 *                                                                         * 
 *   This program is free software; you can redistribute it and/or modify  * 
 *   it under the terms of the GNU General Public License as published by  * 
 *   the Free Software Foundation; either version 2 of the License, or     * 
 *   (at your option) any later version.                                   * 
 *                                                                         * 
 ***************************************************************************/ 
 
#ifdef HAVE_CONFIG_H 
  #include <config.h> 
#endif 
 
#include <stdio.h> 
#include <iostream> 
#include <stdlib.h> 
//#include "head.h" 
//#include "myclass.h" 
 
#include <string.h> 
#include<string> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netdb.h> 
#include <errno.h> 
#include <arpa/inet.h> 
#include <wait.h> 
 
//#include<neinet.in.h> // 
 
#include<fcntl.h> 
#include <signal.h> 
#include<unistd.h> // 
#include<sys/ioctl.h> 
#include<pthread.h> 
#include <algorithm> 
 
using namespace std; 
 
//#define USE_SELECT 
//#define USE_MUTIL_PROCESS 
#define USE_MUTIL_THREAD 
 
 
 
#define MAXCLI 3 
 
struct threadarg 
{ 
   
  int sockid; 
  char addr[32]; 
  }; 
 
int sock; 
volatile int client[MAXCLI]; 
 
void closeSock(int sockid) 
{ 
 
    for(int k=0; k<MAXCLI; k++) 
      { 
	if(client[k]  == sockid) 
	  client[k] = -1; 
      } 
    close(sockid); 
} 
void * threadfun( void* arg) 
{ 
 
  char msgbuf[500]; 
  char temp[500]; 
  int sockid; 
  int n, buflen; 
  pthread_t tid; 
  struct  threadarg*  targ = (threadarg*)arg; 
   
  tid = pthread_self(); 
  pthread_detach(tid); 
  sockid  = targ->sockid; 
   
  buflen = sizeof(msgbuf); 
  memset(msgbuf, 0, buflen); 
 
  pthread_mutex_t mutex; 
  pthread_mutexattr_t mutexattr; 
  pthread_mutex_init (&mutex, &mutexattr); 
  while( (n= read(sockid, msgbuf, sizeof(msgbuf))) >0) 
  { 
    memset(temp, 0, buflen); 
    strcpy(temp,"*"); 
 
    sprintf(temp, "** %d : ", sockid); 
    msgbuf[n] =0; // ?? 
    strcat(temp, msgbuf); 
	     
    pthread_mutex_lock(&mutex); 
    for(int k=0; k<MAXCLI; k++) 
      { 
	if(client[k]!=-1) 
	  { 
	    //int writelen = write(sockid, temp, buflen); 
	    int writelen = write(client[k], temp, buflen); 
	    if(writelen =0) 
	      { 
		cout< tid < " write error "< endl; 
		pthread_mutex_lock(&mutex); 
		closeSock(client[k]); 
		pthread_mutex_unlock(&mutex);   
 
	      } 
	  } 
      } 
    pthread_mutex_unlock(&mutex);     
 
  }//while 
 
  if(n<0) 
  { 
     
    cout< tid < " read error !" <<endl; 
    //    close(sockid); 
    //closeSock(sockid); 
    //return (void*) NULL; 
  } //if 
   
  else if (n==0) 
    { 
    cout< tid < " quit "< endl; 
    //close(sockid); 
    //closeSock(sockid); 
    //return (void*) NULL; 
  
  } 
 
  pthread_mutex_lock(&mutex); 
  closeSock(sockid); 
  pthread_mutex_unlock(&mutex);   
 
  pthread_mutex_destroy(&mutex);	 
  return (void*) NULL; 
} 
 
 
 
void clean_child(int signo) 
{ 
 
  int pid , stat; 
  while( (pid = waitpid(-1, &stat, WNOHANG) ) >0) 
	cout<<" child " < pid < " return "  < stat<<endl;  
 
} 
 
void servIn_thread(int signo) 
{ 
      char str[256]; 
      char buf[500]; 
      int fdd; 
      fd_set fds; 
      struct timeval tv; 
      pthread_mutex_t mutex; 
      pthread_mutexattr_t mutexattr; 
 
      pthread_mutex_init (&mutex, &mutexattr); 
       
      fdd  = fileno(stdout); 
      tv.tv_sec  =0; 
      tv.tv_usec = 0; 
      FD_ZERO(&fds); 
      FD_SET(fdd,&fds); 
      select(fdd+1, &fds, 0, 0, &tv); 
 
      memset(buf, 0, sizeof(buf)); 
      memset(str, 0, sizeof(str)); 
 
      if(FD_ISSET(fdd, &fds)) 
	{	 
	  int send_ret =0; 
	  int ret = read(fdd, str, sizeof(str)); 
	  if(ret<1) 
	    return; 
	   
	  strcat(buf, "#server#  "); 
	  strcat(buf, str); 
	  pthread_mutex_lock(&mutex); 
	  for(int k = 0; k<MAXCLI; k++) 
	    { 
	      if(client[k] != -1) 
		send_ret =send(client[k], buf,  strlen(buf)+1, 0); 
	      if(send_ret <0) 
		{ 
		  cout< client[k] < " send error ! "  < " so close" < client[k] <<endl; 
		  closeSock(client[k]);		 
		} 
	    }//for 
	  pthread_mutex_unlock(&mutex);	        
	} 
} 
 
void servIn(int signo) 
{ 
      char str[256]; 
      char buf[500]; 
      int fdd; 
      fd_set fds; 
      struct timeval tv; 
       
      fdd  = fileno(stdout); 
      tv.tv_sec  =0; 
      tv.tv_usec = 0; 
      FD_ZERO(&fds); 
      FD_SET(fdd,&fds); 
      select(fdd+1, &fds, 0, 0, &tv); 
 
      memset(buf, 0, sizeof(buf)); 
      memset(str, 0, sizeof(str)); 
 
      if(FD_ISSET(fdd, &fds)) 
	{	 
	  int ret = read(fdd, str, sizeof(str)); 
	  if(ret<1) 
	    return; 
	   
	  strcat(buf, "#server#  "); 
	  strcat(buf, str); 
	  for(int k = 0; k<MAXCLI; k++) 
	    { 
	      if(client[k] != -1) 
		int ret =send(client[k], buf,  strlen(buf)+1, 0); 
	      if(ret <0) 
		cout< client[k] < " send error"<<endl; 
	    }	        
	} 
} 
 
void ioaction(int signo) 
{ 
  int n; 
  char msg[500]; 
  fd_set fds; 
  struct timeval tv; 
  tv.tv_sec  =0; 
  tv.tv_usec = 0; 
  FD_ZERO(&fds); 
  FD_SET(sock,&fds); 
  select(sock+1, &fds, 0, 0, &tv); 
 
  if(FD_ISSET(sock, &fds)) 
    { 
      n= read(sock, msg, sizeof(msg)); 
      if(n ==-1) 
	{ 
	  cout< "recv length is -1,  server closed!" <<endl; 
 
	  exit(1); 
	} 
      else if( n==0) 
	{ 
	  cout< "server closeed! " <<"(recv lent is 0)" <<endl; 
	  exit(0); 
	} 
      else 
	{ 
	  char temp[500]; 
	  strcpy(temp, "*"); 
	  strcat(temp , msg); 
	  cout < temp <<endl;  
	   
	} 
    } 
 
} 
 
int main(int argc, char *argv[]) 
{ 
  int sd; 
  int work; 
  char buf[500]; 
  int peerlen;    
  int on =1; 
  char  *p; 
  int socklen; 
  sockaddr_in addr; 
  sockaddr_in peeraddr; 
 
  socklen = sizeof(sockaddr); 
  memset(&addr, 0, socklen); 
  memset(&peeraddr, 0, socklen); 
  addr.sin_addr.s_addr = inet_addr("192.168.1.100"); 
  addr.sin_family = AF_INET; 
  addr.sin_port  = htons(1028); 
     
#ifndef DEBUG 
     
  if(argc != 2) 
    {  
      // goto LABEL; 
      cout < "argmengs : <program> <mode>" <<endl; 
      exit(0); 
    } 
 
 
 
 
    if (argc ==2) 
    { 
      p = argv[1];    
       
    } 
 
#endif 
 
 
#ifdef USE_SELECT 
 
    /////////////////////////////////////////// 
  if(*p == 's') 
  { 
    fd_set fds, rfds; 
 
    sockaddr_in addr; 
    sockaddr_in peeraddr; 
    addr.sin_addr.s_addr = inet_addr("192.168.1.100"); 
    addr.sin_family = AF_INET; 
    addr.sin_port  =htons(1028); 
    int k=0; 
     
    sd  = socket(AF_INET,  SOCK_STREAM, 0); 
    if(sd <0) 
      { 
	cout< " creat socket error!"  <<endl; 
 
      }// if 
     
  if( bind(sd, (sockaddr*)&addr, sizeof(sockaddr) ) <0 ) 
    cout< " bind socket error!"  <<endl; 
  if (listen(sd, 5) <0 ) 
    cout< " listen socket error!"  <<endl; 
 
 
  FD_ZERO(&fds); 
  FD_ZERO(&rfds); 
  FD_SET(sd, &fds); 
  
   memset(buf, 0, sizeof(buf)); 
   int len = sizeof(buf); 
   int maxfd = sd; 
 
   cout < " server start ! " <<endl; 
   int fdd  = fileno(stdout); 
   signal(SIGIO, servIn); 
   fcntl(fdd, F_SETOWN, getpid()); 
   ioctl(fdd, FIOASYNC, &on); 
 
    for( k=0; k<MAXCLI; k++) 
      { 
	client[k] = -1; 
      } 
    
  while(1) 
  { 
    memcpy(&rfds, &fds, sizeof(fds)); 
   int ret = select(maxfd +1, &rfds, 0,0,0); 
   if(ret <0) 
     { 
       if( errno != EINTR) 
	 { 
	   cout <<"sleect error ___inruppt !"<<endl; 
	   exit(1); 
	 } 
       else  
	 continue; 
     } // if ret <0 
 
   if( ret ==0) 
     continue; 
 
   if(FD_ISSET(sd, &rfds)) 
    { 
 
       work = accept(sd,  (sockaddr*)&peeraddr, (socklen_t*)&peerlen ); 
       if(work <0) 
              cout< "accept error"<<endl; 
       else 
       {  
         // cout< inet_ntoa(peeraddr.sin_addr) < endl; 
         // cout< ntohs(peeraddr.sin_port) <<endl; 
         FD_SET(work, &fds); 
	 cout< work < "  enter !" <<endl; 
         if(maxfd =work) 
            maxfd = work; 
	 for(k= 0; k<MAXCLI; k++) 
	   { 
	     if(client[k] ==-1) 
	       { 
		 client[k] = work; 
		 break; 
	       } 
	   } 
         
       }//else 
     }//if 
 
     for(int fd =0; fd  maxfd+1; fd++) 
     { 
           if(fd != sd && FD_ISSET(fd, &rfds)) 
           { 
              int recvlen = recv(fd, buf, len, 0);    // block!!! 
              if(recvlen <0) 
		cout< fd < " recv error" <<endl; 
              if(recvlen ==0 ) 
              { 
                cout< " recv len is 0,  clent  " < fd < "  droped!" <<endl; 
              
		for(k =0; k<MAXCLI; k++) 
		  { 
		    if(client[k] == fd) 
		       client[k] = -1; 
		  } 
		FD_CLR(fd , &fds); 
		//close(fd); 
		shutdown(fd,2); 
              } 
               
             if(recvlen>0) 
	       { 
		 char temp[500]; 
		 temp[0] =0; 
		 sprintf(temp, "%d :  ", fd); 
		 strcat(temp, buf);   
                cout < temp <<endl; 
		 
		for(k = 0; k<MAXCLI; k++) 
		  { 
		    if(client[k] != -1) 
		      int ret =send(client[k], temp,  strlen(temp)+1, 0); 
		    if(ret <0) 
		      cout< client[k] < " send error"<<endl; 
		  } 
 
		 
	       } //if recvlen >0 
 
           }//if  work  fd changed  
 
            
 
     }// for  // 0~ maxfd+1 
 
 
 
 
 
  }// while (1) 
 
  //  shutdown(sd, 2);  
  close(sd); 
  cout < " server  halt " <<endl; 
  //closesocket(sd); 
   
  } // if 's' 
 
/////////////////////////////////////////// 
#endif 
   
#ifdef  USE_MUTIL_PROCESS 
   
  if(*p == 's') 
    { 
    signal(SIGCHLD, clean_child); 
     
    sd  = socket(AF_INET,  SOCK_STREAM, 0); 
    if(sd <0) 
      { 
	cout< " creat socket error!"  <<endl; 
	 
      }// if 
     
     
    if( bind(sd, (sockaddr*)&addr, sizeof(sockaddr) ) <0 ) 
      { 
	cout< " bind socket error!"  <<endl; 
	exit(0); 
      } 
    else  
      cout< " server start!" <<endl; 
    if (listen(sd, 5) <0 ) 
      cout< " listen socket error!"  <<endl; 
     
    while(1) 
      { 
	 
	if(work = accept(sd, 0,0)<0) 
	  { 
	    cout < "accept  errror" <<endl; 
	    break; 
	  } 
	else 
	  { 
	     
	    cout< work < "enter" <<endl;   
	  int retc = fork(); 
	  if(retc<0) 
	    { 
	      cout<<" faile to fork!" <<endl; 
	      exit (4); 
	    }    
	  else if (retc==0) 
	    { 
	      close(sd); 
	      char msgbuf[500]; 
	      char temp[500]; 
	      int recvlen; 
	      int writelen; 
	       
	      cout< " process id " < (int)getpid() <<"will work"<<endl; 
	      while(1) 
		{ 
		  recvlen = read(work, msgbuf, sizeof(msgbuf)); 
		  if(recvlen ==0) 
		    { 
		      cout< work < " close !"<<endl; 
		      close(work); 
		      exit(0); 
		    } 
		  else if (recvlen <0) 
		    { 
		      cout< work < "recv error !" <<endl; 
		      // exit(0); 
		    } 
		  else if (recvlen >0) 
		    { 
		      msgbuf[recvlen] =0; 
		      writelen  = write (work , msgbuf, recvlen); 
		      if(writelen <0) 
			cout< work <<"  send error!" <<endl; 
		      if(writelen ==0) 
			cout< work <<" send length = 0" <<endl; 
		    } 
		}//while(1) 
	       
	    }// if retc ==0 
	  else if(retc >0) 
	    close(work); 
	   
	  }//else accept  
	 
	 
      }//while(1) 
     
     
    } //if(*p == 's') 
   
   
   
#endif 
   
   
#ifdef USE_MUTIL_THREAD 
   
  //typedef  emnu   {red, green} TColor;  
  if(*p == 's') 
    {     
      threadarg  targ; 
      pthread_attr_t attr; 
      pthread_t tid; 
      pid_t pid; 
      pthread_mutex_t mutex; 
      pthread_mutexattr_t mutexattr; 
      int counter =0; 
       
      sd  = socket(AF_INET, SOCK_STREAM, 0); 
      if(sd<0) 
	{ 
	  cout<<"fail to create socket!" <<endl; 
	  exit(0); 
	} 
      if(bind (sd, (sockaddr*)&addr, socklen)<0) 
	{ 
	  cout< "bind error" <<endl; 
	  close(sd); 
	  exit(0); 
	} 
      listen(sd, 5); 
      pid  = getpid(); 
      signal(SIGSEGV, SIG_IGN); 
 
      pthread_mutex_init (&mutex, &mutexattr); 
       
      for(int k=0; k<MAXCLI; k++) 
	{ 
	  client[k] = -1; 
	} 
      //int on =1; 
   int fdd  = fileno(stdout); 
   signal(SIGIO, servIn_thread); 
   fcntl(fdd, F_SETOWN, getpid()); 
   ioctl(fdd, FIOASYNC, &on); 
 
      cout < "server start !" < endl; 
      cout < " current pid = " < pid < endl; 
      while(1) 
	{   
	  // work = accept(sd, (sockaddr*)&peeraddr, &socklen); 
	  work = accept(sd, 0, 0); 
	  if(work <0) 
	    { 
	      cout<<" accept error "< endl; 
	      break; 
	    } 
	  else  
	    { 
	      cout < "sock id " < work < " enter !" <<endl; 
	      pthread_mutex_lock(&mutex); 
	      for( counter=0; counter<MAXCLI; counter++) 
		{ 
		  if(client[counter] == -1) 
		    { 
		      client[counter] = work; 
		      break; 
		    } 
		}//for 
	      if(counter ==MAXCLI) 
		{ 
		  send(work, "overflow", 9, 0); 
		  close(work); 
		} 
	      pthread_mutex_unlock(&mutex); 
 
	    }//else 
	  targ.sockid = work;      
	  //strcpy(targ.addr, inet_ntoa( peeraddr.sin_addr)); 
	  pthread_attr_init(&attr); 
	  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);      
	  if(pthread_create(&tid, &attr, threadfun, (void*)&targ) <0) 
	    //if(pthread_create(&tid, 0, threadfun, 0) <0) 
	  { 
	    cout<<" create thread error!" <<endl; 
	    exit(0); 
	  } 
	   cout< "thread id " < tid <<" will work" <<endl; 
	   
	}//while(1) 
 
    }// if (*p == 's') 
   
 
#endif  // MUTIL_THREAD 
   
   
 
 
 
////////////////////////////client /////////////////////////////////////// 
 
  if( *p == 'c') 
    { 
      cout < " client" <<endl; 
       
      //sockaddr_in peeraddr; 
       
      memset(  &peeraddr, 0, sizeof(sockaddr_in));    
      peeraddr.sin_addr.s_addr = inet_addr("192.168.1.100"); 
      peeraddr.sin_family = AF_INET; 
      peeraddr.sin_port  =htons(1028); 
      sock  = socket(AF_INET, SOCK_STREAM, 0); 
      if( sock <0) 
	cout < " creat client socket erro " < endl; 
       
      if( connect(sock, (sockaddr*)&peeraddr, sizeof(sockaddr))  <0) 
	{ 
	  cout < " connect error "  <<endl; 
	  exit(1); 
	} 
       
      cout< sock < " have connected sucessful !" <<endl; 
      
      signal(SIGIO, ioaction); 
      fcntl(sock, F_SETOWN, getpid()); 
      ioctl(sock, FIOASYNC, &on); 
       
       
       
      while(1) 
	{ 
	  char str[100]; 
	  memset(str, 0, sizeof(str)); 
	  gets(str); 
	   
	  if(strstr(str, "exit")) 
	    break; 
	  send(sock, str , strlen(str)+1 , 0); 
	   
	}// while(1) 
       
      shutdown(sock, 2); 
      cout < " client  exit" <<endl; 
      getchar(); 
      
    } //if is "c"   
   
   
 LABEL: 
  return EXIT_SUCCESS; 
} 
