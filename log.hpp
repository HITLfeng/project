#pragma once 


#include<iostream>
#include<string>
#include<ctime>

//[日志级别][时间戳][日志信息][错误文件名称][行数]
// 日志 ： info  warning error fatal 严重级别上升

#define INFO    1
#define WARNING 2
#define ERROR   3
#define FATAL   4

#define LOG(level,message) Log(#level,message,__FILE__,__LINE__) // #level 转换成字符串


void Log(std::string level,std::string message,std::string file_name,int line)
{
  std::cout<<"["<<level<<"]"<<"["<<time(nullptr)<<"]"<<"["<<message<<"]"<<"["<<file_name<<"]"<<"["<<line<<"]"<<std::endl;



}

