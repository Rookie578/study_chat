#ifndef SINGLETON_H
#define SINGLETON_H

#include <global.h>

/******************************************************
* @单例模板类：
* 单例模式 ：其核心目的只有一个：确保一个类在全局范围内，
* 永远只能被创建出一个实例（对象），并且提供一个全局的访问点。
*
*
*
*
*******************************************************/

template <typename T>
class Singleton{

protected:
    //确保只能创建一个实例

    Singleton() = default; //放在protected中，意味着外部代码不能直接写 new Singleton来创建它，只有它的子类才能调用
    Singleton(const Singleton<T>&)=delete; // 删除拷贝函数
    Singleton& operator = (const Singleton<T>& st) = delete; //删除赋值函数，防止通过别的方式产生第二个实例
    static std::shared_ptr<T> _instance; // 存放唯一实例的静态指针

public:
    static std::shared_ptr<T> GetInstance(){ // 核心魔法（GetInstance），保证“多线程下也安全”
        static std::once_flag s_flag; //线程锁 std::once_flag 是一个“一次性旗帜/令牌”。其作用是标记后面的代码到底有没有被执行过
        std::call_once(s_flag,[&](){ //线程锁 std::call_once
            _instance = std::shared_ptr<T>(new T);  //智能指针，全自动内存管理，防止内存泄漏
        });

        return _instance;
    }

    void PrintAddress(){
        std::cout << _instance.get() << std::endl;
    }


    ~Singleton(){
        std::cout << "this is singleton destruct" << std::endl;
    }
};

//类里面的static静态成员变量，必须在类的外部单独声明一次，告诉编译器给它分配内存
template <typename T>
std::shared_ptr<T> Singleton<T>::_instance = nullptr;


#endif // SINGLETON_H
