#pragma once 
#include<sys/socket.h>
#include<stdlib.h>
#include<iostream>
using namespace std;
#include<sys/types.h>
#include<netinet/in.h>
#include<strings.h>
#include<arpa/inet.h>
#include"log.hpp"

void sys_err(const char* s){
  perror(s);
  exit(1);
}

class Sock{
public:
  static int Socket(){
    int sockfd = socket(AF_INET,SOCK_STREAM,0);
    if(sockfd<0)
      LOG(FATAL,"socket error!");
      //sys_err("socket error");
      
    LOG(INFO,"create socket success");
    return sockfd;
  }

  static void Bind(int fd,int port){
    struct sockaddr_in addr;
    bzero(&addr,sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr =htonl(INADDR_ANY);
    int ret = bind(fd,(struct sockaddr*)&addr,sizeof(addr));
    if(ret<0){
      LOG(FATAL,"bind error!");
      //cerr<<"bind error"<<endl;
      exit(3);
    }
    LOG(INFO,"bind socket ... success");
  }

  static void Listen(int fd){
    if(listen(fd,1024)<0)
      LOG(FATAL,"listen error!");
    LOG(INFO,"listen success");
  }

  static int Accept(int fd){
    struct sockaddr_in c_addr;
    socklen_t c_len;
    int cfd = accept(fd,(struct sockaddr*)&c_addr,&c_len);
    if(cfd<0)
      sys_err("accept error");
    
    return cfd;
  }

  static void Setsockopt(int fd){
    //int setsockopt(int sockfd, int level, int optname,
    //                  const void *optval, socklen_t optlen);
    int opt = 1;
    setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
    
  }


};
