#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include "CommonUtility.h"

//最多未完成链接
static const int MAXPENDING = 3;

int SetupTCPServerSocket(const char *service){
    struct addrinfo addrSetting;    //接受标准
    memset(&addrSetting, 0, sizeof(addrSetting));   //全置0
    addrSetting.ai_family = AF_UNSPEC;              //v4 或者 v6地址
    addrSetting.ai_flags = AI_PASSIVE;              //接受所有地址和端口
    addrSetting.ai_socktype = SOCK_STREAM;          //只接受流式
    addrSetting.ai_protocol = IPPROTO_TCP;          //只接受tcp

    struct addrinfo *servAddr;      //服务地址
    /*使用getaddrinfo获得套接字设置，接受第一个可以使用的
      int getaddrinfo(const char *node, const char *service,
                      const struct addrinfo *hints,
                      struct addrinfo **res)
    */

    int rtnVal = getaddrinfo(NULL, service, &addrSetting, &servAddr);
    if(rtnVal != 0)
        UserMessageHandler("getaddrinfo() failed",gai_strerror(rtnVal));

    int servSock = -1;
    struct addrinfo *addr;
    for(addr = servAddr; addr != NULL; addr = addr->ai_next){
        servSock = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
        if(servSock < 0)
            continue;

        if((bind(servSock, addr->ai_addr, addr->ai_addrlen) == 0) && (listen(servSock, MAXPENDING) == 0)){
            //打印地址
            struct sockaddr_storage localAddr;
            socklen_t addrSize = sizeof(localAddr);
            if(getsockname(servSock, (struct sockaddr *) &localAddr, &addrSize) < 0)
                SystemMessageHandler("getsockname() failed");
            fputs("current addr binding to sock: ",stdout);
            PrintSocketAddress((struct sockaddr *)&localAddr, stdout);
            fputc('\n',stdout);
            break;
        }
        close(servSock);
        servSock = -1;
    }
    freeaddrinfo(servAddr);
    return servSock;
}


int AcceptTCPConnection(int servSock){
    struct sockaddr_storage clntAddr;
    socklen_t clntAddrLen = sizeof(clntAddr);

    //等待阻塞
    int clntSock = accept(servSock, (struct sockaddr*) &clntAddr, &clntAddrLen);
    if(clntSock < 0)
        SystemMessageHandler("accept() failed");
    fputs("handing client ",stdout);
    PrintSocketAddr((struct sockaddr *) &clntAddr, stdout);
    fputc('\n',stdout);
}

void TCPClientHandler(int clntSock){
    char buffer[BUFSIZE];
    ssize_t bytesIn = recv(clntSock, buffer, BUFSIZE, 0);
    if(bytesIn < 0)
        SystemMessageHandler("recv() failed");

    while(bytesIn > 0){
        ssize_t bytesOut = send(clntSock,buffer,bytesIn,0);
        if(bytesOut < 0)
            SystemMessgeHandler("send() failed");
        else if(bytesOut != bytesIn)
            UserMessageHandler("send() error: ","unexpected bytes sent");

        bytesIn = recv(clntSock, buffer, BUFSIZE, 0);
        //
    }
    close(clntSock);
}
