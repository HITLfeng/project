#pragma once 
#include"socket.hpp"
#include<pthread.h>
#include<unistd.h>

//设置单例模式
//netstat -nltp
class TcpServer{
  private:
    TcpServer(int port):_port(port){}
  public:
    static TcpServer* GetInstance(int port){
      static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;//静态或全局用这个初始化不需要init destroy
      if(nullptr == svr){
        pthread_mutex_lock(&mutex);
        if(nullptr == svr){
          svr = new TcpServer(port);
          svr->InitServer();
        }
        pthread_mutex_unlock(&mutex);
      }
      return svr;
    }
    void InitServer()
    {
      lsock = Sock::Socket();
      Sock::Setsockopt(lsock);
      Sock::Bind(lsock,_port);// 云服务器不能直接绑定公网IP （虚拟的？）
      Sock::Listen(lsock);
      LOG(INFO,"tcp_server init ... success");
    }

    int GetLsock(){
      return lsock;
    }
    ~TcpServer(){
      if(lsock>=0)
        close(lsock);
    }
  private:
    int _port;
    int lsock;
    static TcpServer* svr;
};

TcpServer* TcpServer::svr = nullptr;
