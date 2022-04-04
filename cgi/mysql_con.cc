#include <iostream>
using namespace std;
#include "my_sql/include/mysql.h"
//#include "mysql.h"
#include<string>
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

bool InsertSql(string& sql){

  //cout<<"version: "<<mysql_get_client_info()<<endl;
  MYSQL* con = mysql_init(nullptr); 
  //mysql_set_character_set(con,"utf8");
  if(nullptr == mysql_real_connect(con,"127.0.0.1","http_test","erpeng","httpsql",3306,nullptr,0)){
    cerr<<"connect to sql error"<<endl;
    return false;
  }

  
  cerr<<"connect to sql success"<<endl;
  int ret = mysql_query(con,sql.c_str());
  cerr<<"ret = "<<ret<<endl;
  mysql_close(con);

  return true;
}

void CutString(string& src,string &out1,string& out2,string seq){
  size_t pos = src.find(seq);
  if(pos!=string::npos){
    out1 = src.substr(0,pos);
    out2 = src.substr(pos+seq.size());
  }
}

int main(){
  string arg,sql;
  if(GetData(arg)){
    // name=tom&passwd=111111
    
    string str1,str2;
    string var1,value1,var2,value2;
    CutString(arg,str1,str2,"&");
    CutString(str1,var1,value1,"=");
    CutString(str2,var2,value2,"=");
    sql = "insert into user (name,password) values(\'";
    sql += value1;
    sql += "\',\'";
    sql += value2;
    sql += "\');";  // ; 可有可无
    //string sql = "insert into user (name,password) values(\"二碰\",\'erpengge\');";
    if(InsertSql(sql)){
      cout<<"<html>";
      cout<<"<head><meta charset=\"utf-8\"></head>";
      cout<<"<body><h1>注册成功!</h1></body>";
    }

    //cerr<<"query = "<<sql<<endl;
  }

  return 0;
}

/*
int main(){
  //cout<<"version: "<<mysql_get_client_info()<<endl;
  MYSQL* con = mysql_init(nullptr); 
  //mysql_set_character_set(con,"utf8");
  if(nullptr == mysql_real_connect(con,"127.0.0.1","http_test","erpeng","httpsql",3306,nullptr,0)){
    cerr<<"connect to sql error"<<endl;
    return 1;
  }

  
  cerr<<"connect to sql success"<<endl;
  string sql = "insert into user (name,password) values(\"二碰\",\'erpengge\');";
  int ret = mysql_query(con,sql.c_str());
  cerr<<"query = "<<sql<<endl;
  cerr<<"ret = "<<ret<<endl;
  mysql_close(con);
  
  return 0;
}
*/
