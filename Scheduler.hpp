#pragma once
#include"coroutine_base.hpp"
#include<list>
#include<tuple>

template<typename CoroutineType>
concept Concept_CorotineType = requires(CoroutineType ct){
    CoroutineType::promise_type;
    CoroutineType::packer_type;
};

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

template<typename CoroutineType, template <typename> typename  ContainerType = std::list, typename SchedulerMode = scheduler_mode::LinearQueues>
class Scheduler;


template<typename CoroutineType>
class Scheduler<CoroutineType, std::list, scheduler_mode::LinearQueues>
{
    public:
    using outcome_type = std::pair<typename CoroutineType::value_type, std::size_t>;
    using packer_type = typename CoroutineType::packer_type;

    template<typename ...T>
    Scheduler(T... t){
        ([&](){
            auto coro_packer(packer_type(std::forward<T>(t)));
            this->co_list.push_back(coro_packer);
            this->outcome_list.push_back(outcome_type({}, coro_packer.get_id()));
        }(),...);
    }

    void set_mode(){}

    void test(){
        std::cout<<"----------------------at test begin----------------------"<<std::endl;
        for(auto it = this->co_list.begin(); it!=co_list.end(); it++){
            co_list.erase(it);
        }
        std::cout<<"----------------------at test end----------------------"<<std::endl;
    }
    ~Scheduler(){
        std::cout<<"-----------------------------------------------Scheduler distruct-----------------------------------------------"<<std::endl;
    }

    private:
    std::list<packer_type> co_list;
    std::list<outcome_type> outcome_list;
};

template<typename CoroutineType, template<typename> class  ContainerType>
requires Concept_ContainerType<CoroutineType, ContainerType>
class Scheduler<CoroutineType, ContainerType, scheduler_mode::LinearQueues>
{
    public:
    Scheduler(){}
    void set_mode(){}
};