#include"./include/Scheduler.hpp"
#include "./include/coroutine_base.hpp"
#include "assistance.hpp"
#include <string>
#include <thread>

coroutine_states<std::string, int,std::string> func(std::string coro_num){
    auto p = current_corotine_id();
    std::cout<<coro_num<<':'<<"coro_handle: "<<p<<std::endl;
    int i = 0;
    for(; i<1000000; i++){
        // std::cout<<"thread:"<<std::this_thread::get_id()<<' '<<coro_num<<' '<<__FUNCTION__<<" now value:"<<i<<std::endl;
        co_yield {coro_num, i, std::to_string(i + 100)};
    }
    std::cout<<"thread:"<<std::this_thread::get_id()<<' '<<coro_num<<' '<<__FUNCTION__<<" now value:"<<i<<std::endl;
    std::cout<<coro_num<<" done"<<std::endl;
}

coroutine_states<std::string, int,std::string> func2(std::string num){
    auto p = current_corotine_id();
    std::cout<<num<<':'<<"coro_handle:"<<std::hex<<' '<<p<<std::endl;
    for(int i = 0; i<10; i++){
        std::cout<<num<<':'<<__FUNCTION__<<" now value:"<<i<<std::endl;
        co_yield {num, i, std::to_string(i+200)};
    }
}

coroutine_states<void> func3(std::string num){
    auto p = current_corotine_id();
    std::cout<<num<<':'<<"coro_handle: "<<p<<std::endl;
    int i = 0;
    for(; i<1000000; i++){
        // std::cout<<num<<':'<<__FUNCTION__<<" now value:"<<i<<std::endl;
        co_await awaiter_stop{};
    }
    std::cout<<"total num:"<<i<<std::endl;
}

int main(){
    using type = decltype(func("co"));
    auto num = 1;
    std::cout<<"############################################# Scheduler_"<<num<<" #############################################"<<std::endl;
    Scheduler<type> sche(
        func("co"), 
        coroutine_packer(func("co2"), 3),
        coroutine_packer(func("co3"), 1)
        );

    {
        auto f = [&sche](){
            sche.continuous();
        };
        std::thread t(f);
        std::thread y(f);
        std::thread tt(f);
        std::thread yy(f);
        sche.push_coroutine(func("co4"));
        sche.push_coroutine(coroutine_packer(func("co3"), HIGHEST_LEVEL));
        t.join();
        y.join();
        tt.join();
        yy.join();
    }
    
    // Scheduler<> sche2(func3("sche"), func3("sche2"), func3("sche3"));
    // {
    //     auto f = [&sche2](){
    //         sche2.continuous();
    //     };
    //     std::thread t(f);
    //     std::thread y(f);
    //     std::thread tt(f);
    //     std::thread yy(f);
    //     t.join();
    //     y.join();
    //     tt.join();
    //     yy.join();
    // }
    std::cout<<"############################################# Scheduler_"<<num++<<" #############################################\n"<<std::endl;

    std::cout<<"############################################# Scheduler_"<<num<<" #############################################"<<std::endl;
    
    std::cout<<"############################################# Scheduler_"<<num++<<" #############################################\n"<<std::endl;

    std::cout<<"-----------------------------------------------end of all-----------------------------------------------"<<std::endl;
    return 0;
}