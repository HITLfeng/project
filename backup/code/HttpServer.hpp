#pragma once 
#include<signal.h>
#define PORT 8888
#include"Protocol.hpp"
#include"TcpServer.hpp"

class HttpServer{
  private:
    int _port;
    TcpServer* tcp_server;//单例
    bool stop;
  public:
    HttpServer(int port = PORT)
      :_port(port)
      ,tcp_server(nullptr)
      ,stop(false)
    {}
    
    void InitServer(){
      tcp_server = TcpServer::GetInstance(_port);
      signal(SIGPIPE,SIG_IGN);//写入出错 防止终止服务器
    }

    void Loop(){
      LOG(INFO,"Loop begin...");
      int lsock = tcp_server->GetLsock();
      while(!stop){
        //accept 要拿到监听套接字
        struct sockaddr_in c_addr;
        socklen_t clen = sizeof(c_addr); 
        int csock = accept(lsock,(struct sockaddr*)&c_addr,&clen);
        if(csock<0){
          //打印日志信息
          LOG(WARNING,"accept failed !!!");
          continue;
        } 
        LOG(INFO,"get a new link");
        int* _sock = new int(csock);
        pthread_t tid;
        pthread_create(&tid,nullptr,Entrance::HandlerRequest,_sock);//第四个值要避免传入发生改变的量，因为线程函数解析变量也需要时间
        pthread_detach(tid);
      }
    }
    
    ~HttpServer(){}



};
