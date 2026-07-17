#pragma once

#include <string>
#include <iostream>

class Timestamp
{
public:
    Timestamp();    //默认构造函数

    //根据微秒数构造时间戳，explicit表示只能通过显式转换调用
    explicit Timestamp(int64_t microSecondsSinceEpoch);

    //获取当前系统时间
    //静态成员函数，不属于任何对象，直接通过类名调用即可
    static Timestamp now();
    
    //将时间戳转换为字符串，便于人阅读
    std::string toString() const;

private:
    int64_t microSecondsSinceEpoch_;    //存储从unix纪元开始的微秒数
};