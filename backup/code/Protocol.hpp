#pragma once 
#include<iostream>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include"Util.hpp"
#include<string>
#include<vector>
#include"log.hpp"
#include<sstream>
#include<unordered_map>
#include<sys/stat.h>
#include<algorithm>
#include<fcntl.h>
#include<sys/sendfile.h>
#include<sys/wait.h>
#include<stdlib.h>

#define SERVER_ERROR 500
#define BAD_REQUEST 400
#define OK 200
#define NOT_FOUND 404
#define WEB_ROOT "wwwroot"
#define HOME_PAGE "index.html"
#define HTTP_VERSION "HTTP/1.0"
#define LINE_END "\r\n"
#define PAGE_404 "404.html"

/*
class Code2Desc{
  private:
    std::unordered_map<int,std::string> code2desc;//响应状态行 200 OK 
  public:
    Code2Desc(){}
    void Init(){
      code2desc[200] = "OK";//...
    }
    ~Code2Desc(){}
};*/

#include<signal.h>

void handler(int signo){
  std::cout<<"get a signo "<<signo<<std::endl;
  exit(1);
}

static std::string Code2Desc(int code){
  std::string desc;
  switch(code){
    case 200:
      desc = "OK";
      break;
    case 404:
      desc = "NOT FOUND";
      break;
    default:
      break;
  }
  return desc;
}

static std::string Suffix2Desc(std::string suffix){
  static std::unordered_map<std::string,std::string> suffix2desc = {
    {".html","text/html"},
    {".css","text/css"},
    {".js","application/javascript"},
    {".jpg","application/x-jpg"},
    {".xml","application/xml"}
  };
  auto iter = suffix2desc.find(suffix);
  if(iter!=suffix2desc.end()){
    return iter->second;
  }
  return "text/html";
}


class HttpRequest{
  public:
    std::string request_line;
    std::vector<std::string> request_header;//请求报头
    std::string blank;
    std::string request_body;

    //解析后的结果
    std::string method;
    std::string uri;
    std::string version;
    
    std::unordered_map<std::string,std::string> header_kv;

    int content_length;

    std::string path;
    std::string arg;

    std::string suffix;

    int size;
    bool cgi;
  public:
    HttpRequest():content_length(0),cgi(false){}
    ~HttpRequest(){}
};

class HttpResponse{
  public:
    std::string status_line;
    std::vector<std::string> reponse_header;//请求报头
    std::string blank;
    std::string response_body;

    int status_code;
    int fd;
  public:
    HttpResponse():blank(LINE_END),status_code(OK),fd(-1){}
    ~HttpResponse(){}

};

class EndPoint{
  public:
    //完成具体要做的事 业务逻辑
    //读取请求 分析请求 构建响应 IO通信
    EndPoint(int _sock):sock(_sock),stop(false){}
    bool IsStop(){
      return stop;
    }
    /*
    void RcvHttpRequest(){
      RcvHttpRequestLine();
      RcvHttpRequestHeader();
    }*/

    /*void ParseHttpRequest(){
      ParseHttpRequestLine();//解析第一行 请求行
      ParseHttpRequestHeader();
      RcvHttpRequestBody();
    }*/
    void RcvHttpRequest(){
      if(RcvHttpRequestLine()||RcvHttpRequestHeader()){
        //stop 为真 recv error
      }
      else{
        //全部正确
        ParseHttpRequestLine();//解析第一行 请求行
        ParseHttpRequestHeader();
        RcvHttpRequestBody();
      }
    }
    void BuildHttpReponse(){
      std::string _path;
      int size = 0;
      size_t pos = 0;
      auto& code = http_response.status_code;
      if(http_request.method != "GET" && http_request.method != "POST"){
        LOG(WARNING,"method is wrong");
        code = BAD_REQUEST;
        goto END;
      }
      // B 上传 登录注册  POST(正文) GET（URL）搜索NBA 网页后面的一长串URL 就是GET传参  // 或 下载 GET
      if(http_request.method == "GET"){
        //PATH ARG
        size_t pos = http_request.uri.find('?');
        if(pos != std::string::npos){
          Util::KVStr2Map(http_request.uri,http_request.path,http_request.arg,'?'); 
          http_request.cgi = true;
        }
        else{
          http_request.path = http_request.uri;
        }
      }
      else if(http_request.method == "POST"){
        //POST method 
        http_request.cgi = true; 
        http_request.path = http_request.uri;
      }
      else{

      }
      //1.这里的路径表明了服务器上的某种资源，是从根目录开始的吗？  服务器必须指明web根目录  mkdir www.root  默认首页 index.html
      //2.路径是否合法，对应路径的资源是否存在？
      //拼接 wwwroot 
      _path = http_request.path;
      http_request.path = WEB_ROOT;
      http_request.path += _path;
      if(http_request.path[http_request.path.size()-1] == '/'){
        //wwwroot/
        http_request.path += HOME_PAGE; 
      }
      //查看文件是否存在
      struct stat st;
      if(stat(http_request.path.c_str(),&st) == 0){
        //    wwwroot/a/b/c
        //    1.c是路径   //每个目录都会对应一个index.html 
        //    2.c是文件
        if(S_ISDIR(st.st_mode)){
          http_request.path += "/";
          http_request.path += HOME_PAGE;
          //st 已经变了 路径变了 
          stat(http_request.path.c_str(),&st);
        }
        //有没有可能是可执行程序？
        if((st.st_mode & S_IXUSR)||(st.st_mode & S_IXGRP)||(st.st_mode & S_IXOTH))
        {
          http_request.cgi = true;
        }
        size = st.st_size;
        http_request.size = size;
      }
      else{
        // source didnt exist
        std::string info = http_request.path;
        info += " Not Found";
        LOG(WARNING,info);
        code = NOT_FOUND;
        goto END;
      }

      pos = http_request.path.rfind('.');
      if(pos!=std::string::npos){
        http_request.suffix = http_request.path.substr(pos);
      }
      else{
        http_request.suffix = "html";
      }

      //CGI 用户有数据上传时
      if(http_request.cgi){
        //process cgi
        code = ProcessCgi();
      }
      else{
        //none cgi 返回静态网页
       code = ProcessNonCgi();//构建HTTP响应
      }

END:
      BuildHttpReponseHelper(code);
      return;
    }

    void SendHttpReponse(){
      send(sock,http_response.status_line.c_str(),http_response.status_line.size(),0);
      for(auto iter:http_response.reponse_header){
        send(sock,iter.c_str(),iter.size(),0);//LINE_END别忘了加
      }
      //具体什么时候发由TCP决定 这只是拷贝到内核发送缓冲区
      send(sock,http_response.blank.c_str(),http_response.blank.size(),0);
      if(http_request.cgi){
        auto& body = http_response.response_body;
        int total = 0;
        int size = 0;
        const char* start = body.c_str();
        while((size = send(sock,start + total,body.size()-total,0)) > 0){
          total += size; 
        }
      }
      else{
        //std::cout<<"test for 404\n";//*******************************************************************************
        //std::cout<<"fd = "<<http_response.fd<<std::endl;//*******************************************************************************
        //std::cout<<"size = "<<http_request.size<<std::endl;//*******************************************************************************
        //int fd = http_response.fd;
        //char buf[2048];
        //ssize_t s = read(fd,buf,2048);
        //buf[s] = 0;
        //write(sock,buf,s);
        sendfile(sock,http_response.fd,nullptr,http_request.size);
        close(http_response.fd);
      }
    }

    ~EndPoint(){
      close(sock);
    }
  private:
    bool RcvHttpRequestLine(){
      if(Util::ReadLine(sock,http_request.request_line) > 0){
        http_request.request_line.resize(http_request.request_line.size()-1);
        LOG(INFO,http_request.request_line);
      }
      else{
        stop = true;//注意这只是判断第一次！
      }
      return stop;
    }
    bool RcvHttpRequestHeader()//调用上一个函数 读到http_request.request_header里 blank 为止
    {
      std::string line;
      while(true){
        line.clear();
        if(Util::ReadLine(sock,line)<=0){
          stop = true;
          break;
        }
        if(line=="\n"){
          http_request.blank = line;
          break;
        }
        line.resize(line.size()-1);//去掉\n
        http_request.request_header.push_back(line);
        //LOG(INFO,line);
      }
      return stop;
    }
    void ParseHttpRequestLine()//解析第一行 请求行
    {
      //method URI verson
      std::string& line = http_request.request_line;
      std::stringstream ss(line);
      ss>>http_request.method>>http_request.uri>>http_request.version;
      auto& method = http_request.method;
      std::transform(method.begin(),method.end(),method.begin(),::toupper);
    }

    void ParseHttpRequestHeader()//解析第一行 请求行
    {
      //解析报文 都是Host: 42.194.135.248:8888键值对
      std::string key,val; 
      for(auto e:http_request.request_header){
        //Util::KVStr2Map(e,key,val,':');
       if(Util::KVStr2Map(e,key,val,":"))
         http_request.header_kv.insert(make_pair(key,val));
      }
      
    }

    bool NeedRcvHttpRequestBody(){
      std::string method = http_request.method;
      if(method=="POST"){
        auto &header_kv = http_request.header_kv;
        //http_request.content_length = header_kv["Content-Length"];
        auto iter = header_kv.find("Content-Length");
        if(iter!=header_kv.end()){
          http_request.content_length = atoi(iter->second.c_str());
          return true;
        }
      }
      return false;
    }
    //是否有正文部分 ？ method == POST 有 RET 无 
    // Content-Length 决定正文长度
    bool RcvHttpRequestBody(){
      if(NeedRcvHttpRequestBody()){
        int content_length = http_request.content_length;
        auto& body = http_request.request_body;
        char ch;
        while(content_length){
          ssize_t s = recv(sock,&ch,1,0);
          if(s>0){
            body.push_back(ch);
            content_length--;
          }
          else{
            //TODO
            stop = true;
            break;
          }
        }
      } 
      return stop;
    }
    int ProcessNonCgi(){
      http_response.fd = open(http_request.path.c_str(),O_RDONLY);
      if(http_response.fd>=0){

        return OK;
      }
      return 404;
      //构建完响应行 空行已经初始化 现在呢我们进行下一步 将 http_request.path 文件内容读取到 response body 
      // 思路 open fd read  send 涉及多次内核到用户态转换(读到用户缓冲区  再切换为内核发送) 现介绍 接口 sendfile
      //sendfile 只在内核区进行
      
    }
    void HandlerError(std::string page){
      http_request.cgi = false;
      http_response.fd = open(page.c_str(),O_RDONLY);

      if(http_response.fd >= 0){
        struct stat st;
        stat(page.c_str(),&st);
        http_request.size = st.st_size;
        std::string line = "Content-Type:text/html";
        line += LINE_END;
        http_response.reponse_header.push_back(line);
        line = "Content-Length:";
        line += std::to_string(st.st_size);
        line += LINE_END;
        http_response.reponse_header.push_back(line);
      }
    }

    void BuildOkResponse(){
        std::string line = "Content-Type:";
        line += Suffix2Desc(http_request.suffix);
        line += LINE_END;
        http_response.reponse_header.push_back(line);
        line = "Content-Length:";
        if(http_request.cgi){
          line += std::to_string(http_response.response_body.size());
        }
        else 
          line += std::to_string(http_request.size);//静态网页大小
        line += LINE_END;
        http_response.reponse_header.push_back(line);
      
    }

    void BuildHttpReponseHelper(int code)
    {
      auto& status_line = http_response.status_line;
      status_line = HTTP_VERSION;
      status_line += " ";
      status_line += std::to_string(code);
      status_line += " ";
      status_line += Code2Desc(code);
      status_line += "\n";

      std::string path = WEB_ROOT;
      path += "/";
      switch(code){
        case OK:
          BuildOkResponse();
          break;
        case NOT_FOUND:
          path +=  PAGE_404;
          //std::cout<<path<<std::endl;
          HandlerError(path);
          break;//发现代码逻辑和 ProcessNonCgi 非常相似 构建响应状态 报头 网页
        case SERVER_ERROR:
          path += PAGE_404;
          HandlerError(path);
          break;
        case BAD_REQUEST:
          path += PAGE_404;
          HandlerError(path);
          break;
        default:
          break;
      }

    }
    int ProcessCgi(){
      auto& bin = http_request.path;
      auto& data = http_request.arg;//GET  直接write 效率低 因为 data 通常比较小 环境变量！！！
      auto& body = http_request.request_body;//POST
      auto& method = http_request.method;
      int content_length = http_request.content_length;
      auto& response_body = http_response.response_body;

      int code = 200;
      std::string data_env; 
      std::string content_length_env; 
      std::string method_env;//告诉子进程是GET or POST 
      //父子之间涉及到进程间通信 向子进程发送数据，从子进程拿取数据 这里用匿名管道
      int input[2];
      int output[2];//都是相对父进程而言
      if(pipe(input)<0){
        LOG(ERROR,"pipe input error");
        code = SERVER_ERROR;
        return code;
      }
      if(pipe(output)<0){
        LOG(ERROR,"pipe output error");
        code = SERVER_ERROR;
        return code;
      }
      //pefd[0] refers to the read end of the pipe.  pipefd[1] refers to the write end of  the pipe.

      pid_t pid = fork();
      if(pid>0){
        close(output[0]);
        close(input[1]);
        if(method == "POST"){
          //int size = body.size();
          const char* start = body.c_str();
          int total = 0;
          int size = 0;
          while(total<content_length && (size = write(output[1],start+total,body.size()-total)) > 0){
            total += size;
          }
        }
        char ch;
        //while(read(input[0],&ch,1)>0){
        while(read(input[0],&ch,1)!=0){
          response_body.push_back(ch);
          LOG(INFO,"recv a ch from child");
        }

        std::string test = "response body=";
        test += std::to_string(response_body.size());
        LOG(WARNING,test);
        int status = 0;
        pid_t ret = waitpid(pid,&status,0);//阻塞等待有问题吗？
        //waitpid(pid,&status,0);//阻塞等待有问题吗？
        
        if(ret==pid){
          if(WIFEXITED(status)){
            if(WEXITSTATUS(status)==0){
              code = OK;
            }
            else{
              code = BAD_REQUEST;
            }
          }
          else{
            LOG(ERROR,"return error ");
            //std::cout<<"signal : "<<(status&0x7f)<<std::endl;
            code = SERVER_ERROR;
          }
        }
        close(output[1]);
        close(input[0]);
      }
      else if(pid==0){
        //exec
        close(output[1]);
        close(input[0]);

        method_env = "METHOD=";
        method_env += method;
        putenv((char*)method_env.c_str());// c_str const 类型 char* 
        if(method == "GET"){
          data_env = "DATAENV=";
          data_env += data;

          LOG(INFO,data_env);// a=100&b=200
          putenv((char*)data_env.c_str());
          LOG(INFO,"GET METHOD,ADD DATA env");// a=100&b=200
        }
        else if(method == "POST"){
          content_length_env = "CONTENT-LENGTH=";
          content_length_env += std::to_string(content_length);
          putenv((char*)content_length_env.c_str());
          LOG(INFO,"POST METHOD,ADD CONTENT-LENGTH env");
        }
        else{

        }

        dup2(output[0],0);//子进程往标准输入流写 就能写道缓冲区啦
        dup2(input[1],1);//子进程从标准输出文件里读
        execl(bin.c_str(),bin.c_str(),nullptr);// 啪一下，input output 都没了 exec 替换代码段、数据段、堆栈段 文件描述符打开表 PCB 不做替换 打开表还在但是
        // input output 没了 怎么唤醒记忆
        // 制定协议 exec前重定向
        exit(1);
      }
      else{
        LOG(ERROR,"fork error");
        code = 404;
        return code;
      }
      return code;
    }
    
  private:
    int sock;
    HttpRequest http_request;
    HttpResponse http_response;
    bool stop; //1.逻辑错误 已经读取完毕 要给对方回应 2.读取错误 如对方突然中断链接等 这种无需回应
};

class Entrance{
  public:
    static void* HandlerRequest(void* _sock){
      LOG(INFO,"handler request begin ... ");
      int sock = *(int*)_sock;
      delete (int*)_sock;
      //write(sock,"welcome to my web...",20);  http format
//#ifndef DEBUG 
//#define DEBUG 
//      //test 实际不能这么读取 可能会粘包
//      //按行读取 不推荐 getline 要兼容各种格式报文 \r\n \n \r  自己封装函数 Util.hpp
//      char buf[4096];//查看 http 请求报文
//      recv(sock,buf,sizeof(buf),0);
//      std::cout<<buf<<std::endl;
//#endif
      //std::cout<<"get a new link..."<<sock<<std::endl;
#ifdef DEBUG 
      //test 会粘包
      //按行读取 不推荐 getline 要兼容各种格式报文 \r\n \n \r  自己封装函数 Util.hpp
      char buf[4096];//查看 http 请求报文
      recv(sock,buf,sizeof(buf),0);
      std::cout<<buf<<std::endl;
#else
      EndPoint* ep = new EndPoint(sock);
      ep->RcvHttpRequest();
      //ep->ParseHttpRequest();
      if(!ep->IsStop()){
        LOG(INFO,"recv no error, now build and send response");
        ep->BuildHttpReponse();
        ep->SendHttpReponse();
      }
      else{
        LOG(WARNING,"recv error,stop build response");
      }
      delete ep;
#endif
      //std::string str;
      //Util::ReadLine(sock,str);
      //std::cout<<str<<std::endl;
      //close(sock);
      LOG(INFO,"handler request end ....");
      return nullptr;
    }



};

