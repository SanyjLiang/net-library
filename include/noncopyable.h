#pragma once            //防止头文件重复包含

//noncopyable类 用于防止派生类对象进行拷贝构造和赋值构造，
// 但派生类对象可正常构造和析构

class noncopyable
{
public:
    noncopyable(const noncopyable &)=delete;
    noncopyable &operator=(const noncopyable &)=delete;

protected:
    noncopyable()=default;
    ~noncopyable()=default;
};

