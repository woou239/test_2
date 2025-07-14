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
    // �������һֱ���ã����ص�һֱ���Ѿ�����ʼ���ľ�̬ʵ������ͬһ����ַ
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



