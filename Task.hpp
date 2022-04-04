#pragma once 
#include"Protocol.hpp"

class Task{
  private:
    int sock;
    CallBack handler;//仿函数类   operator()()
  public:
    Task(){}
    Task(int k):sock(k){

    }
    void ProcessOn(){
      handler(sock);
    }
    ~Task(){}


};
