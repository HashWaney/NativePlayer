# NativePlayer
A native player  for audio  play


## 空指针异常 记录：

   `Fatal signal 11 (SIGSEGV), code 1, fault addr 0x0 in tid 31286 `

    ```c++

        pthread_t* thread;

        pthread_create(thread,NULL,callback,this);

    ```

    上诉代码直接崩溃,原因如下：
        指针引用不能访问地址；
        pthread_t 是线程的标识符

        pthread_create(thread,....)
        thread 被定义一个指针，不能直接访问地址，
        而pthread_create(....)在创建指针的过程中，第一个参数是线程标志的地址，也就是需要通过&thread的方式来访问thread的地址，
        但是thread被定义为指针，指针是不能直接访问地址的，因此触发了崩溃。


    正确做法：
        声明一个pthread_t thread;
        因此我们只需要声明一个线程标识符，thread, 通过&thread获取thread的地址，然后通过pthread_create()方法来创建线程。
        pthread_create(&thread,....);


