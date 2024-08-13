# Shabby_CoroutineScheduler
-  基于c++20 coroutine的一个***Head-Only***简易协程调度器
-  请确保你的编译器支持cpp20及以上标准，本项目用于调试的编译器为 ***gcc13.1*** 👉[各家编译器对于c++20的支持情况](https://zh.cppreference.com/w/cpp/20)
# **开始使用**
-  仅需包含***include/*** 目录下的 ***Scheduler.hpp***头文件
##  对于本项目，关于c++20 coroutine一些必要的知识
>  此处仅针对本项目作最低需求的讲述，若想深入了解请移步  👉[cpprefrence/coroutine](https://en.cppreference.com/w/cpp/language/coroutines)
-  关键字 **co_await**、**co_yield**、**co_return**
    -  **co_await**
    >  **暂停**一个协程
    -  **co_yield**
    >  **暂停**一个协程并返回一个值
    -  **co_return**
    >  **终止**一个协程并返回值（与普通函数的return作用大致相同）
-  awaiter
  >用于控制协程的**行为**（**暂停** or **运行**）


  - ### 一个简单的协程函数示例
  ```
  coroutine_states<void> func(std::string num){
    //获取当前协程的id值
    auto p = current_corotine_id();

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
    auto co_ = co;//其实二者共享同一协程
  ```
  >  当 **resume()** 被调用时，协程将从**上次暂停的地方**恢复执行直至下次暂停或结束
  > 
  >  ⚠️当co被复制时，***内部的handle并不会复制，而是共享一份***

   - 协程(coroutine_states)具有如下**成员函数**
  ```
    operator bool();
  ```
  >  返回一个bool，标识该协程是否**可以继续执行**，如果该协程**尚未执行完成**，返回**true**、如果**已经执行完成**，则返回**false**；
  ```
    value_type get_promise_value();
  ```
  >  返回协程最近一次执行后的值（当协程不返回值时，即 ***coroutine_states&lt;void>*** -**空状态协程**，**没有返回值**，即返回***void***）；
  ```
    bool value_ready();
  ```
  >  返回当前协程是否有值更新，有**值更新**则返回**true**，**无值更新**则返回**false**（对于**空状态协程**，**永远返回false**）；
  ```
    void operator ()();
  ```
  >  将调用resume();
  ```
    void resume();
  ```
  >  如上所述，恢复协程执行；
  ```
    get_handle();
  ```
  >  获取协程的底层handle的包装器；
  ```
    get_id();
  ```
  >  获取协程的**唯一id标识**；

  - ### copacker
  >  用于封装并保存每一次coroutine运行后产生的值，对于**空状态协程同样有特化**，**也可作为独立单位使用**
  - copacker具有上述coroutine_states的所有成员函数,它等同于直接操作底层封装的协程对应函数（**除了get_promise_value()和get_handle()**），此外，它还具有如下成员函数：
  ```
    auto get_coroutine_state_ptr();
  ```
  >  获取底层封装的coroutine_state的智能指针（shared_ptr）；
  ```
    const auto get_priority();
  ```
  >  获取被封装的协程的优先级（priority_t）
  > 
  >  协程优先级的数值越大，优先级越高，已定义如下几个优先级
  > 
  >  constexpr priority_t HIGHEST_LEVEL = 0xffffffff'ffffffff;//最高优先级
  > 
  >  constexpr priority_t LOWEST_LEVEL = 0x00;//最低优先级
  > 
  >  constexpr priority_t MIDDLE_LEVEL = HIGHEST_LEVEL/2;//最中间优先级(´▽`ʃ♡ƪ)
  ```
    value_list_t get_value_list();
  ```
  >  获取产生的值列表(std::list,***空状态协程无此成员***)
- ### Scheduler调度器

对于Scheduler,有如下定义：
```
    template<typename CoroutineType = coroutine_states<void> >
    class Scheduler;
```
>其中：
>
> CoroutineType--协程类型，**默认为空状态协程**
- 协程(coroutine_states)具有如下**成员函数**
```
    void step()
```
>  让该协程调度器（不受内部线程调度）单步执行一次（指从调度队列中取出一个协程任务并进行一次调用）；
```
    void continuous()
```
>  让该协程调度器在当前线程持续调度直到内部无任务；
```
    co_id_t add_co(packer_type packer)
```
>  向调度器中添加一个协程任务；
```
    void start()
```
>  启动调度器，并自动开始执行内部线程；
```
    void automatic(bool val = true)
```
>  设置该调度器为内部线程执行模式；
```
    void intterput(bool val = true)
```
>  暂停当前调度器直到取消暂停；
```
    void terminate(bool val = true)
```
>  完全终止该调度器 ***⚠⚠⚠⚠该行为可能引起资源泄露⚠⚠⚠⚠***
```
    void set_max_threads(std::size_t max_thr)
```
>  设置当前调度器最大内部线程数量；
```
    std::vector<co_id_t> get_running_id_vector()
```
>  获取当前调度器正在调度队列中的协程；
```
    std::size_t thread_num()
```
>  获取当前调度器中正在参与调度的线程数量；
```
    bool empty()
```
>  返回当前调度器内部是否有协程正在参与调度；
```
    std::size_t coroutine_num()
```
>  获取当前协程调度器中正在被调度的协程数量；

额外的，对于**非空状态协程**，有如下成员
```
    value_list_t get_value_list(co_id_t co_id)
```
>   获取对应协程id的值列表；
```
    auto get_finishied_id_vector()
```
>  获取当前调度器所有已完成的协程的id；

- #### 一个简单的调度器使用示例
```
    coro<std::string, int,std::string> func(std::string coro_num){
        co_await awaiter_stop{};
        auto p = current_corotine_id();
        co_await awaiter_no_value_ready{};
        std::cout<<coro_num<<':'<<"coro_handle:"<<std::hex<<(uint64_t)p<<std::endl;
        for(int i = 0; i<9; i++){
            std::cout<<coro_num<<':'<<__FUNCTION__<<" now value:"<<i<<std::endl;
            co_yield {coro_num, i, std::to_string(i+100)}; 
        }
    }
    
    coroutine_states<std::string, int,std::string> func2(std::string num){
        auto p = current_corotine_id();
        co_await awaiter_no_value_ready{};
        std::cout<<num<<':'<<"coro_handle:"<<std::hex<<(uint64_t)p<<std::endl;
        for(int i = 0; i<10; i++){
            std::cout<<num<<':'<<__FUNCTION__<<" now value:"<<i<<std::endl;
            co_yield {num, i, std::to_string(i+200)};
        }
    }
```
``` 
    //创建协程
    auto co (func("co"));
    coroutine_states<std::string, int,std::string> co2 = func2("co2");

    using type = decltype(co);

    //可接收任意个数**copacker**或**coroutine_states**的左值或右值
    //⚠⚠⚠ 优先级由couroutine持有，若直接传入coroutine_states，优先级将默认为最低优先级0 ⚠⚠⚠
    Scheduler<type> sche(copacker(co,2)/*带有优先级packer*/,
                    func("co2"),
                    copacker(func("co3"))/*不带有优先级的packer，默认为最低优先级*/,
                    copacker(func("co4"), HIGHEST_LEVEL)
    );
    sche.add_co(func("co5"));

    //单步执行每一个被添加到Scheduler的协程
    sche.step();
    //连续执行所有被添加的协程，直到所有协程被完成
    sche.continuous();

    //获取所有已完成的协程的id的vector
    auto id_vec = sche.get_finishied_id_vector();
    for(auto id: id_vec){
        std::cout<<"id:"<<id<<std::endl;
    }
    std::cout<<std::endl;

    std::vector<decltype(sche.get_value_list(0))> result_vec;
    for(auto id: id_vec){
        std::cout<<"id:"<<id<<std::endl;
        //获取对应id的协程的值列表，若没有值则返回空列表
        result_vec.push_back(sche.get_value_list(id));
    }
    std::cout<<"defore view"<<std::endl;
    for(auto result_list: result_vec){
        std::cout<<"begin---------"<<std::endl;
        for(auto result: result_list){
            auto [co, num, symbol] = result;
            std::cout<<co<<' '<<num<<' '<<symbol<<std::endl;
        }
        std::cout<<"end---------"<<std::endl;
    }
```
