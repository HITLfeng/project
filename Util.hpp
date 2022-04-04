#pragma once 
#include<iostream>
#include<string>
#include<sys/types.h>
#include<sys/socket.h>


class Util{
  public:
    static int ReadLine(int sock,std::string& str){
      char ch = 'X';
      while(ch!='\n'){
        ssize_t s = recv(sock,&ch,1,0);
        if(s>0){
          if(ch=='\r'){
            // \r --> \n 0r \r\n --> \n
            //查看以下\r后面的字符但并不取走 ！
            //数据窥探
            recv(sock,&ch,1,MSG_PEEK);
            if(ch=='\n'){
              recv(sock,&ch,1,0);//取走\n
            }
            else{
              ch = '\n';
            }
          }
          str.push_back(ch);
        }
        else if(s==0){
          return 0;
        }
        else{
          return -1;
        }
      }
      return str.size();
    }

    static bool KVStr2Map(std::string& target,std::string& key,std::string& value,char ch = ':'){
      //说明，本函数只处理遇到的第一个ch 
      size_t pos = target.find(ch);
      if(pos!=std::string::npos){
        key = target.substr(0,pos);
        value = target.substr(pos+1);
        return true;
      }
      return false;
    }
    
    static bool KVStr2Map(std::string& target,std::string& key,std::string& value,std::string ch = ":"){
      //说明，本函数只处理遇到的第一个ch 
      size_t pos = target.find(ch);
      if(pos!=std::string::npos){
        key = target.substr(0,pos);
        value = target.substr(pos+ch.size());
        return true;
      }
      return false;
    }



};
