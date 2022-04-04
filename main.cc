#include"HttpServer.hpp"
#include<memory>
#include"log.hpp"

void Use(char* s){
  cout<<"method: "<<s<<" port"<<endl;
  exit(2);
}

int main(int argc,char* argv[]){
  //LOG(WARNING,"listen error");
  
  if(2 != argc){
    Use(argv[0]);
  }

  int port = atoi(argv[1]);
  std::shared_ptr<HttpServer> http_server(new HttpServer(port));
  http_server->InitServer();
  http_server->Loop();


  return 0;
}
