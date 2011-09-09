#include <stdbool.h>
#include <stdio.h>
#include <sys/socket.h>

//ErrorHandler.c
//用户错误信息
void UserMessageHandler(const char *msg, const char *detail);
//系统错误信息
void SystemMessageHandler(const char *msg);

//AddressUtility.c
//打印v4 or v6地址
void PrintSocketAddress(const struct sockaddr *addr,FILE *stream);
//对比地址
bool SockAddrsEqual(const struct sockaddr *addr1,const struct sockaddr *addr2);

//TCPServerUtility.c
//监听
int SetupTCPServerSocket(const char *service);
//接受连接
int AcceptTCPConnection(int servSock);
//应用层处理
void TCPClientHandler(int clntSock);
//新建socket
int SetupTCPClientSocket(const char *server, const char *service);

//定义最大和缓存大小
#define MAXSTRINGLENGTH = 128;
#define BUFSIZE = 512;

