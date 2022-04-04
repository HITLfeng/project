#include<iostream>
using namespace std;
#include<string>
#include<sstream>

int main(){
  string str = "GET /a/b/c/xu.html http/1.0";
  string method;
  string uri;
  string version;
  //default 按照空格对字符串进行输出
  stringstream ss(str);
  ss>>method>>uri>>version;
  cout<<method<<uri<<version<<endl;

  return 0;
}

