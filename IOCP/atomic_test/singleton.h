#pragma once
#include <iostream>

//Singleton_3* Singleton_3::m_instance = new Singleton_3();

class Singleton_3
{
private:
    Singleton_3()
    {
        std::cout << "Singleton_3 created" << std::endl;
    }

public:
    // 如果这里一直调用，返回的一直是已经被初始化的静态实例，是同一个地址
    static Singleton_3& getInstance()
    {
        static Singleton_3 m_instance;
        return m_instance;
    }
    //static void destroyInstance()
    //{
    //    if (getInstance() != NULL)
    //    {
    //        delete getInstance();
    //        /*m_instance = nullptr;*/
    //    }
    //}

    void test_fun();
    int *flag = new int(1);
};



