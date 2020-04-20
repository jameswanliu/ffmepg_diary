//
// Created by James Tony on 2020/4/18.
//

#ifndef FFMPEG_DIARY_SAFEQUEUE_H
#define FFMPEG_DIARY_SAFEQUEUE_H

#include <pthread.h>

#ifdef C11
#include <thread>
#endif

#include "queue"

using namespace std;

template<typename T>
class SafeQueue {

    typedef void (*ReleaseHandle)(T &);

    typedef void(*SysncHandle)(queue<T> &);


public:
    SafeQueue() {
#ifdef C11

#else
        pthread_mutex_init(&mutex, NULL);
        pthread_cond_init(&cond, NULL);
#endif
    }

    ~SafeQueue() {
#ifdef C11
#else
        pthread_mutex_destroy(&mutex);
        pthread_cond_destroy(&cond);
#endif
    }



    void enQueue(T t){
#ifdef C11
        lock_guard<mutex> lk(mt);
        if (work) {
            q.push(new_value);
            cv.notify_one();
        }
#else
        pthread_mutex_lock(&mutex);
        if (work) {
            queue.push(t);
            pthread_cond_signal(&cond);
            pthread_mutex_unlock(&mutex);
        } else {
            releaseHandle(t);
        }
        pthread_mutex_unlock(&mutex);
#endif

    }

    void setWork(int work) {
#ifdef C11
        lock_guard<mutex> lk(mt);
        this->work = work;
#else
        pthread_mutex_lock(&mutex);
        this->work = work;
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);
#endif

    }




    int deQueue(T &q){
        int ret = 0;
#ifdef C11
        //占用空间相对lock_guard 更大一点且相对更慢一点，但是配合条件必须使用它，更灵活
        unique_lock<mutex> lk(mt);
        //false则不阻塞 往下走
        cv.wait(lk,[this]{return !work || !q.empty();});
        if (!q.empty()) {
            value = q.front();
            q.pop();
            ret = 1;
        }
#else
        pthread_mutex_lock(&mutex);
        //在多核处理器下 由于竞争可能虚假唤醒 包括jdk也说明了
        while (work && queue.empty()) {
            pthread_cond_wait(&cond, &mutex);
        }
        if (!queue.empty()) {
            q = queue.front();
            queue.pop();
            ret = 1;
        }
        pthread_mutex_unlock(&mutex);
#endif
        return ret;
    }


    void clear() {
#ifdef C11
        lock_guard<mutex> lk(mt);
        int size = queue.size();
        for(int i = 0;i<size;i++){
            T t = queue.front();
           releaseHandle(t);
           queue.pop();
        }

#else
        pthread_mutex_lock(&mutex);
        int size = queue.size();
        for (int i = 0; i < size; i++) {
            T t = queue.front();
            releaseHandle(t);
            queue.pop();
        }
        pthread_mutex_unlock(&mutex);
#endif
    }

    bool empty() {
        return queue.empty();
    }

    int size() {
        return queue.size();
    }


    void setReleaseHandle(ReleaseHandle r) {
        releaseHandle = r;
    }

    void setSysncHandle(SysncHandle s) {
        sysncHandle = s;
    }

private:


#ifdef C11
    mutex tx;
    conditation_avaiable ca;
#else
    pthread_mutex_t mutex;
    pthread_cond_t cond;
#endif
    int work;
    queue<T> queue;
    ReleaseHandle releaseHandle;
    SysncHandle sysncHandle;


};

#endif
