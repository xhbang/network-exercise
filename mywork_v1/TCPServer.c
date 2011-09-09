#include <stdio.h>
#include <unistd.h>
#include "CommonUtility.h"

int main(int argc, char *argv[]){
    //（1）程序建立和参数解析
    if(argc != 2)
        UserMessageHandler("参数: ","端口号");
    char *service = argv[1];
    //（2）套接字创建和设置
    int servSock = SetupTCPServerSocket(service);
    if(servSock < 0)
        UserMessageHandler("SetupTCPServerSocket 失败",service);
    //（3）反复处理
    for(;;){
        int clntSock = AcceptTCPConnnection(servSock);
        TCPClientHandler(clntSock);
        close(clntSock);
    }
//NEVER REACH
}
