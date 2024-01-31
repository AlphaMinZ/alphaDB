#ifndef __ALPHA_SINGLETON_H__
#define __ALPHA_SINGLETON_H__

#include <memory>

namespace alphaMin {

namespace {

template<class T, class X, int N>
T& GetInstanceX() {
    static T v;
    return v;
}

template<class T, class X, int N>
std::shared_ptr<T> GetInstancePtr() {
    static std::shared_ptr<T> v(new T);
    return v;
}

}

// 单例模式封装类
template<class T, class X = void, int N = 0>
class Singleton {
public:
    // 返回单例裸指针
    static T* GetInstance() {
        static T v;
        return &v;
    }
};

template<class T, class X = void, int N = 0>
class SingletonPtr {
public:
    // 返回单例智能指针
    static std::shared_ptr<T> GetInstance() {
        static std::shared_ptr<T> v(new T);
        return v;
    }
};

}

#endif