## 生产者 消费者 模型

    参考：https://blog.csdn.net/chengonghao/article/details/51779279

### 原理分析：
    假设有两个线程同时访问一个全局变量，这个全局变量的初始值为0，
    int n =0 ；
    临界区：（n>0) 消费者A可以继续消费，生产者将条件（n>0)通知消费者A线程。
    消费者线程A 进入临界区，访问n，A必须等到n大于0才能接着往下执行，如果n==0，那么A将一直等待
    生产者线程B 进入临界区，修改n的值，使得n大于0，当n大于0，B通知等待n大于0条件的消费者A线程，A被通知后可以继续执行。



    死锁：
        当消费者A进入临界区，其他线程不能进入临界区，意味着B没有机会修改n n的值一直为0，不满足A继续执行的条件（n大于0），A一直等待
        消费者线程（拿到）互斥锁 --> 进入临界区-->发现共享资源n不满足继续执行的条件（n大于0）--->等待n大于0
        消费者线程（占有）互斥锁 --> 生产者无法进入临界区-->无法修改n的值---->生产者等待消费者释放互斥锁


    解决办法：
        条件变量


     通常情况下，对于共享资源保护要用到锁操作，当一个进程进入临界区会拿到互斥锁（lock操作），
     然后其他进程拿不到互斥锁，就无法进入临界区，当该进程发现共享资源不满足继续向下执行的条件时，
     就应该释放锁，让其他进程修改共享资源，以满足自己所需的执行条件


     1。 消费者拿到互斥锁
     2。 消费者进入临界区
     3。 判断共享条件是否满足
     4。 条件不满足，释放互斥锁，
     5。 消费者等待条件变量
     6。 生产者拿到互斥锁
     7。 生产者进入临界区
     8。 修改共享资源
     9。 生产者通知等待条件变量的消费者
     10。生产者释放互斥锁
     11。消费者拿到互斥锁
     12。消费者消费资源（条件满足3---->12）
     13。消费者释放互斥锁(多个消费进程形成队列 13--->1)
     14. 结束




     使用方法：
     条件变量的使用

       1. 初始化一个条件变量
       int pthread_cond_init(pthread_cond_t* cond,pthread_condattr_t* cond_attr);

       2.销毁一个条件变量
       int pthread_cond_destroy(pthread_cond_t* cond);

       3.令一个消费者等待条件变量上
       int pthread_cond_wait(pthread_cond_t* cond,pthread_mutex_t* mutex)

       4.生产者通知等待在条件变量上的消费者
       int pthread_cond_signal(pthread_cond_t* cond)

       5.生产者向消费者广播消息
       int pthread_cond_broadcast(pthread_cond_t* cond)



    消费者等待条件
        1.拿到互斥锁,进入临界区
        pthread_mutex_lock(&mutex);

        2.while(不满足进行消费的条件）{
            pthread_cond_wait(&cond,&mutex);//令进程等待条件变量

        }

        3.释放互斥锁
          pthread_mutex_unlock(&mutex);


    生产者通知消费者的伪代码：

        1.生产者线程拿到互斥锁进入到临界区
        pthread_mutex_lock(&mutex);

        2.修改条件

        3.通知等待条件变量的消费者
        pthread_cond_signal(cond);

        4.释放互斥锁
        pthread_mutex_unlock(&mutex);







