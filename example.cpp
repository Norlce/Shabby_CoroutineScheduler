#include"./include/Scheduler.hpp"
#include "assistance.hpp"
#include "coroutine_base.hpp"
#include <chrono>
#include <string>
#include <thread>
#include<iostream>

coro<std::string, int,std::string> func(std::string coro_num){
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

coro<std::string, int,std::string> func2(std::string num){
    auto p = current_corotine_id();
    std::cout<<num<<':'<<"coro_handle:"<<std::hex<<' '<<p<<std::endl;
    for(int i = 0; i<10; i++){
        std::cout<<num<<':'<<__FUNCTION__<<" now value:"<<i<<std::endl;
        co_yield {num, i, std::to_string(i+200)};
    }
}

coro<void> func3(std::string num){
    auto p = current_corotine_id();
    std::cout<<num<<':'<<"coro_handle: "<<p<<std::endl;
    int i = 0;
    for(; i<1000000; i++){
        // std::cout<<num<<':'<<__FUNCTION__<<" now value:"<<i<<std::endl;
        co_await awaiter_stop{};
    }
    std::cout<<"thread:"<<std::this_thread::get_id()<<' '<<num<<' '<<__FUNCTION__<<" now value:"<<i<<std::endl;
}

int main(){
    using type = decltype(func("co"));
    auto num = 1;
    std::cout<<"############################################# Scheduler_"<<num<<" #############################################"<<std::endl;
        Scheduler<type> sche(
            func("co"), 
            copacker(func("co2"), 3),
            copacker(func("co3"), 1)
            );
    {
        sche.automatic();
        sche.set_max_threads(4);
        auto running_id = sche.get_running_id_vector();
        std::cout<<"running_id:";
        for(auto id:running_id){
            std::cout<<id<<' ';
        }
        std::cout<<std::endl;
        sche.add_co(func("co4"));
        sche.add_co(copacker(func("co5"), HIGHEST_LEVEL));
        auto finish_id = sche.get_finishied_id_vector();
        
        
        for(auto &id:running_id){
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            std::cout<<id<<std::endl;
            auto values = sche.get_value_list(id);
            // for(auto [co, i, num]:values){
            //     std::cout<<co<<' '<<i<<' '<<num<<std::endl;
            // }
            std::cout<<"finish.size():"<<values.size()<<std::endl;
        }
        
    }
    
    std::cout<<"############################################# Scheduler_"<<num++<<" #############################################\n"<<std::endl;

    std::cout<<"############################################# Scheduler_"<<num<<" #############################################"<<std::endl;
    sche.intterput();
    Scheduler<> sche2(
        func3("sche"),
        func3("sche2"), 
        copacker(func3("sche3"), HIGHEST_LEVEL), 
        func3("sche4"));
    {
        sche2.automatic();
        sche2.set_max_threads(4);
        sche.intterput(false);
        sche2.start();
        sche2.intterput();
    }
    std::cout<<"############################################# Scheduler_"<<num++<<" #############################################\n"<<std::endl;
    sche2.intterput(false);
    std::cout<<"-----------------------------------------------end of all-----------------------------------------------"<<std::endl;
    return 0;
}