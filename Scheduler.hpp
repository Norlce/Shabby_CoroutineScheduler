#pragma once
#include"coroutine_base.hpp"
#include<list>
#include<tuple>
#include<map>
#include<vector>

template<typename CoroutineType>
concept Concept_CorotineType = requires(CoroutineType ct){
    typename CoroutineType::promise_type;
    typename CoroutineType::packer_type;
    typename CoroutineType::value_type;
    
};

template<typename ...Types>
concept Concept_CorotineTypes = (Concept_CorotineType<Types>,...);

template<typename CoroutineType, template<typename> class  ContainerType>
concept Concept_ContainerType = requires(ContainerType<CoroutineType>){
    ContainerType<CoroutineType>::push_back(CoroutineType{});
    ContainerType<CoroutineType>::push_fornt(CoroutineType{});
    ContainerType<CoroutineType>::pop_back(CoroutineType{});
    ContainerType<CoroutineType>::pop_fornt(CoroutineType{});
};

namespace scheduler_mode{
    struct LinearQueues{  LinearQueues(){}  };
};

template<typename CoroutineType = coroutine_states<void>, template <typename> class  ContainerType = std::list, typename SchedulerMode = scheduler_mode::LinearQueues>
class Scheduler;


template<typename CoroutineType>
requires Concept_CorotineType<CoroutineType>
class Scheduler<CoroutineType, std::list, scheduler_mode::LinearQueues>
{
    public:
    using value_type = CoroutineType::value_type;
    using packer_type = typename CoroutineType::packer_type;
    using value_list_t = std::list<value_type>;

    Scheduler(){}

    Scheduler(CoroutineType ct){
        this->co_list.push_back(ct);
        this->co_map[ct.get_id()] = (--this->co_list.end());
    }

    template<typename ...T>
    requires Concept_CorotineTypes<T...>
    Scheduler(T&&... t){
        ([&](){
            this->co_list.push_back(std::forward<T>(t));
            this->co_map[t.get_id()] = (--this->co_list.end());
        }(),...);
    }

    void test(){
        std::cout<<"----------------------at test begin----------------------"<<std::endl;
        for(auto i = 0; i<5; i++){
            for(auto it = this->co_list.begin(); it!=co_list.end(); it++){
                auto [coro_num, val, str] = it->get_promise_value();
                std::cout<<"coro_id:"<<it->get_id()<<" "<<coro_num<<": now value:"<<val<<' '<<str<<std::endl;
                (*it)();
            }
        }
        std::cout<<"----------------------at test end----------------------"<<std::endl;
    }

    void push_coroutine(CoroutineType ct){
        auto coro_packer(ct);
        this->co_list.push_back(coro_packer);
        this->co_map[ct.get_id()] = (--this->co_list.end());
    }

    void step(){
        for(auto it = this->co_list.begin(); it!=this->co_list.end(); it++){
            if(*it){
                (*it)();
            }
            else{
                this->finished_list.push_back(*it);
                this->co_map[it->get_id()] = (--this->finished_list.end());
                this->co_list.erase(it);
            }
        }
    }

    void continuous(){
        while(!this->co_list.empty()){
            this->step();
        }
    }

    value_list_t get_value_list(co_id_t co_id){
        if(!this->co_map.contains(co_id)){
            return {};
        }
        auto co_iterator = this->co_map[co_id];
        auto value_list = co_iterator->get_value_list();
        if(!(*co_iterator)){
            this->co_map.erase(co_id);
            this->finished_list.erase(co_iterator);
        }
        return value_list;
    }

    auto get_finishied_id_vector(){
        std::vector<co_id_t> vec;
        for(auto co: this->finished_list){
            vec.push_back(co.get_id());
        }
        return vec;
    }

    auto get_running_id_vector(){
        std::vector<co_id_t> id_list;
        for(auto &co: this->co_list){
            id_list.push_back(co.get_id());
        }
        return id_list;
    }

    ~Scheduler(){
    }

    private:
    std::list<packer_type> co_list;
    std::map<co_id_t,  typename std::list<packer_type>::iterator > co_map;
    std::list<packer_type> finished_list;
};

// template<typename CoroutineType>
// template<typename ...T>
// Scheduler<CoroutineType, std::list, scheduler_mode::LinearQueues>::Scheduler<T...>;

template<typename ...T>
Scheduler(T...)->Scheduler<std::decay_t<T>...>;

template<typename CoroutineType, template<typename> class  ContainerType>
requires Concept_ContainerType<CoroutineType, ContainerType>
class Scheduler<CoroutineType, ContainerType, scheduler_mode::LinearQueues>
{
    public:
    Scheduler(){}
    void set_mode(){}
};

template<>
class Scheduler<coroutine_states<void>, std::list, scheduler_mode::LinearQueues>
{
    using CoroutineType = coroutine_states<void>;
    public:
    using outcome_type = CoroutineType::value_type;
    using packer_type = typename CoroutineType::packer_type;

    Scheduler(){}

    template<typename ...T>
    requires Concept_CorotineTypes<T...>
    Scheduler(T&&... t){
        ([&](){
            this->co_list.push_back(packer_type(std::forward<T>(t)));
        }(),...);
    }

    void push_coroutine(CoroutineType ct){
        this->co_list.push_back(ct);
    }

    void step(){
        for(auto it = this->co_list.begin(); it!=this->co_list.end(); it++){
            if(*it){
                (*it)();
            }
            else{
                this->co_list.erase(it);
            }
        }
    }

    void continuous(){
        while(!this->co_list.empty()){
            this->step();
        }
    }

    auto get_running_id_vector(){
        std::vector<co_id_t> id_list;
        for(auto &co: this->co_list){
            id_list.push_back(co.get_id());
        }
        return id_list;
    }

    ~Scheduler(){
    }

    private:
    std::list<packer_type> co_list;
};