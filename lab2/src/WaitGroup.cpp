#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>


class WaitGroup {

  private:
    int count;
    pthread_mutex_t mutex;
    pthread_cond_t cond;

  public:
    WaitGroup(int _count) {
        count = _count;
        pthread_mutex_init(&mutex, NULL);
        pthread_cond_init(&cond, NULL);
    }
    void Add(int delta) {
        pthread_mutex_lock(&mutex);
        count += delta;

        if (count <= 0) {
            pthread_cond_broadcast(&cond);
        }

        pthread_mutex_unlock(&mutex);
    }

    void Done() {
        this -> Add(-1);
    }

    void Wait() {
        pthread_mutex_lock(&mutex);
        while (count > 0) {
            pthread_cond_wait(&cond, &mutex);
        }
        pthread_mutex_unlock(&mutex);
    }
};
