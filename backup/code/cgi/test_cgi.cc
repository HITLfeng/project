#include<iostream>
#include<cstdlib>
using namespace std;
#include<unistd.h>

bool GetData(string& arg){
  //cerr<<"Debug "<<getenv("METHOD")<<endl;//服务器对 test_cgi  调用的时候才会打印出这个变量
  bool ret = false;
  string method = getenv("METHOD");
  if("POST" == method){
    // 管道读
    int content_length = atoi(getenv("CONTENT-LENGTH"));
    char c = 0;
    while(content_length--){
      read(0,&c,1);
      arg += c;
    }
    ret = true;
  }
  else if("GET" == method){
    // 环境变量读
    arg = getenv("DATAENV");
    ret = true;
  }
  else{
    ret = false;
  }
  return ret;
}

void CutString(string& src,string &out1,string& out2,string seq){
  size_t pos = src.find(seq);
  if(pos!=string::npos){
    out1 = src.substr(0,pos);
    out2 = src.substr(pos+seq.size());
  }
}

//这个就是子进程用来执行的程序
int main(){
  string data;
  GetData(data);
  //cerr<<"getdata "<<ret<<endl;
  // a=100&b=200
  //cerr<<data<<endl;
  string str1,str2;
  string var1,value1,var2,value2;
  CutString(data,str1,str2,"&");
  CutString(str1,var1,value1,"=");
  CutString(str2,var2,value2,"=");
  cerr<<var1<<"+"<<var2<<"="<<atoi(value1.c_str())+atoi(value2.c_str())<<endl;
  cerr<<"............................................\n";
  //cout<<1<<endl;
  //write(1,"hello",5);
  cout<<var1<<"+"<<var2<<"="<<atoi(value1.c_str())+atoi(value2.c_str())<<endl;
  return 0;
}
