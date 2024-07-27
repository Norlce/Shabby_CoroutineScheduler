# Shabby_CoroutineScheduler
-  基于c++20 corotine的一个***Head-Only***简易协程调度器
-  请确保你的编译器支持cpp20及以上标准，本项目用于调试的编译器为 ***gcc13.1***    [各家编译器对于c++20的支持情况](https://zh.cppreference.com/w/cpp/20)
# **开始使用**
-  仅需包含***include/*** 目录下的 ***Scheduler.hpp***头文件
##  对于本项目，关于c++20 coroutine一些必要的知识
>  此处仅针对本项目作最低需求的讲述，若想深入了解请移步  [cpprefrence/coroutine](https://en.cppreference.com/w/cpp/language/coroutines)
-  关键字 **co_await**、**co_yield**、**co_return**
    -  **co_await**
    >  **暂停**一个协程
    -  **co_yield**
    >  **暂停**一个协程并返回一个值（在本项目中称为 **“状态”** -states）
    -  **co_return**
    >  **终止**一个协程并返回状态（与普通函数的return作用大致相同）
-  awaiter
  >用于控制协程的**行为**（**暂停** or **运行**）


  ### 一个简单的协程函数示例
  ```
  coroutine_states<void> func(std::string num){
    for(int i = 0; i<30; i++){
        std::cout<<num<<':'<<__FUNCTION__<<" now value:"<<i<<std::endl;
        co_await awaiter_stop{};//在此处暂停执行
    }
  }
  ```
  >  该协程没有返回的状态
  >
  >  当调用 **co_await awaiter_stop{};** 的时候，该协程将暂停执行直至被恢复
  >
  >  该协程可作为单独个体使用
  ```
  auto co = func("co0");
  func.resume();//协程在此处恢复执行
  ```
  >  当 **resume()** 被调用时，协程将从**上次暂停的地方**恢复执行直至下次暂停或结束

  - 对于**返回控状态的协程**它具有如下**成员函数**
  ```
    operator bool();
  ```
  >  返回一个bool，标识该协程是否**可以继续执行**，如果该协程**尚未执行完成**，返回**true**、如果**已经执行完成**，则返回**false**
  ```
    value_type get_promise_value();
  ```
  >  返回协程最近一次执行后的状态
  ```
    void operator ()();
  ```
  >  将调用resume();
  ```
    void resume();
  ```
  >  如上所述，恢复协程执行
  ```
    get_handle();
  ```
  >  获取协程的底层handle的包装器
  ```
    get_id();
  ```
  >  获取协程的**唯一id标识**
