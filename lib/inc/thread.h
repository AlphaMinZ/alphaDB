#ifndef __ALPHA_THREAD_H__
#define __ALPHA_THREAD_H__

#include "mutex.h"

namespace alphaMin {

//线程类
class Thread : Noncopyable {
public:
    typedef std::shared_ptr<Thread> ptr;

    Thread(std::function<void()> cb, const std::string& name);

    ~Thread();

    // thread Id
    pid_t getId() const { return m_id;}

    const std::string& getName() const { return m_name;}

    // wait thread over
    void join();

    // get curr thread ptr
    static Thread* GetThis();

    // get curr thread name
    static const std::string& GetName();

    // set curr thread name
    static void SetName(const std::string& name);

private:
    static void* run(void* arg);
private:
    
    pid_t m_id = -1;

    pthread_t m_thread = 0;

    std::function<void()> m_cb;

    std::string m_name;

    Semaphore m_semaphore;


};

}

#endif