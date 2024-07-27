#include"Scheduler.hpp"
#include<list>
#include<any>
#include<variant>
#include<map>

auto test(){
    std::cout<<__PRETTY_FUNCTION__<<std::endl;
    return std::make_tuple(1,"fuck", 2.3);
}

coroutine_states<std::string, int,std::string> func(std::string coro_num){
    co_await awaiter_stop{};
    uint64_t p;
    current_corotine_id(p);
    co_await awaiter_no_value_ready{};
    std::cout<<"coro_handle:"<<std::hex<<(uint64_t)p<<std::endl;
    for(int i = 0; i<9; i++){
        std::cout<<coro_num<<':'<<__FUNCTION__<<" now value:"<<i<<std::endl;
        co_yield {coro_num, i, std::to_string(i+100)}; 
    }
}

coroutine_states<std::string, int,std::string> func2(std::string num){
    for(int i = 0; i<10; i++){
        std::cout<<num<<':'<<__FUNCTION__<<" now value:"<<i<<std::endl;
        co_yield {num, i, std::to_string(i+200)};
    }
}

coroutine_states<void> func3(std::string num){
    for(int i = 0; i<30; i++){
        std::cout<<num<<':'<<__FUNCTION__<<" now value:"<<i<<std::endl;
        co_await awaiter_stop{};
    }
}

int main(){
    // // fun(1,2,3,4,5,6);
    auto co (func("co"));
    // coroutine_states<std::string, int,std::string> co2 = func2("co2");
    using type = decltype(co);
    // Scheduler<type> sche(coroutine_packer(co,2), func("co2"));

    // sche.push_coroutine(func("co3"));
    // sche.continuous();

    // auto id_vec = sche.get_finishied_id_vector();
    // std::cout<<(id_vec.size()==0?"-------------------is fucking void":"-------------------all right")<<std::endl;
    // for(auto id: id_vec){
    //     std::cout<<"id:"<<id<<std::endl;
    // }
    // std::cout<<std::endl;
    // std::vector<decltype(sche.get_value_list(0))> result_vec;
    // for(auto id: id_vec){
    //     std::cout<<"id:"<<id<<std::endl;
    //     result_vec.push_back(sche.get_value_list(id));
    // }
    // std::cout<<"defore view"<<std::endl;
    // for(auto result_list: result_vec){
    //     std::cout<<"begin---------"<<std::endl;
    //     for(auto result: result_list){
    //         auto [co, num, symbol] = result;
    //         std::cout<<co<<' '<<num<<' '<<symbol<<std::endl;
    //     }
    //     std::cout<<"end---------"<<std::endl;
    // }
    // Scheduler<> sche2(coroutine_packer(func3("co3"),3), func3("co4"));
    // std::cout<<(typeid(coroutine_states<void>)==typeid(coroutine_states<>)?"==":"!=")<<std::endl;
    // sche2.continuous();
    // Scheduler<type, scheduler_mode::PriorityScheduling> sche3(coroutine_packer(co,2), func2("co2"), coroutine_packer(func2("co3"),3));
    // sche3.push_coroutine(coroutine_packer(func("co4"), 99));
    // auto vec = sche3.get_running_id_vector();
    // std::cout<<"running id:";
    // for(auto id: vec){
    //     std::cout<<std::hex<<id<<' ';
    // }
    // std::cout<<std::endl;
    // sche3.continuous();
    Scheduler<void_co_t, scheduler_mode::PriorityScheduling> sche4(coroutine_packer(func3("co3"),3), func3("co4"));
    sche4.push_coroutine(coroutine_packer(func3("co_max"), HIGHEST_LEVEL));
    auto vec = sche4.get_running_id_vector();
    std::cout<<"running id:";
    for(auto id: vec){
        std::cout<<std::hex<<id<<' ';
    }
    std::cout<<std::endl;
    sche4.continuous();

    std::cout<<"-----------------------------------------------end of all-----------------------------------------------"<<std::endl;
    return 0;
}