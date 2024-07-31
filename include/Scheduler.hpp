#pragma once
#include"coroutine_base.hpp"
#include<list>
#include<tuple>
#include<map>
#include<vector>
#include<set>

template<typename CoroutineType = coroutine_states<void>, typename PriorityMode = scheduler_mode::FairScheduling, typename MemoryMode = memmory_mode::LinearQueues, template <typename> class  ContainerType = std::list>
class Scheduler;

template<typename CoroutineType>
requires Concept_CorotineType<CoroutineType>
class Scheduler<CoroutineType, scheduler_mode::FairScheduling, memmory_mode::LinearQueues, std::list>
{
    public:
    using value_type = CoroutineType::value_type;
    using packer_type = typename CoroutineType::packer_type;
    using value_list_t = std::list<value_type>;

    Scheduler(){}

    Scheduler(packer_type ct){
        this->push_coroutine(ct);
    }

    template<typename ...T>
    requires Concept_CoroutinePackerType_or_CorotineTypes<T...>
    Scheduler(T&&... t){
        ([&](){
            this->push_coroutine(packer_type(std::forward<T>(t)));
        }(),...);
    }

    void push_coroutine(packer_type ct){
        this->co_list.push_back(ct);
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

/*
________________________________________________________________________________________________________________________________________________________________________________
#finish_list:
[finished_co]->[finished_co]->[finished_co]->[finished_co]->[finished_co]
###

#co_list:
[coroutine_base]->[coroutine_base]->[coroutine_base]->[coroutine_base]->[coroutine_base]
###

#pri_list_map:
[priority]      [priority]      [priority]
    |                |              |
 [co_list]       [co_list]       [co_list]
 ###
________________________________________________________________________________________________________________________________________________________________________________
*/
template<typename CoroutineType>
requires Concept_CorotineType<CoroutineType>
class Scheduler<CoroutineType, scheduler_mode::PriorityScheduling, memmory_mode::LinearQueues, std::list>
{   
    public:
    using value_type = CoroutineType::value_type;
    using packer_type = typename CoroutineType::packer_type;
    using value_list_t = std::list<value_type>;
    using packer_list_t = std::list<packer_type>;

    public:
    Scheduler(){}

    Scheduler(packer_type packer){
        this->push_coroutine(packer);
    }

    template<typename ...T>
    requires Concept_CoroutinePackerType_or_CorotineTypes<T...>
    Scheduler(T&&... t){
        (this->push_coroutine(packer_type(std::forward<T>(t))),...);
    }

    co_id_t push_coroutine(packer_type packer){
        auto priority = packer.get_priority();
        auto id = packer.get_id();
        this->priority_set.insert(priority);
        this->pri_packerlist_map[priority].push_back(packer);
        this->co_map[id] = (--(this->pri_packerlist_map[priority].end()));
        return id;
    }

    void step(){
        for(auto pri_it = this->priority_set.rbegin(); pri_it!=this->priority_set.rend(); pri_it++){
            auto end_it = get_next_iterator(pri_it);
            for(auto iter_it = this->priority_set.rbegin(); iter_it!=end_it; iter_it++){
                if(!this->pri_packerlist_map.contains(*iter_it)){
                    break;
                }
                auto& current_packerlist =  this->pri_packerlist_map[*iter_it];
                for(auto packerlist_it = current_packerlist.begin(); packerlist_it!=current_packerlist.end(); packerlist_it++){
                    if(*packerlist_it){
                        (*packerlist_it)();
                    }
                    else{
                        this->finished_list.push_back(*packerlist_it);
                        this->co_map[packerlist_it->get_id()] = (--(this->finished_list.end()));
                        current_packerlist.erase(packerlist_it);
                    }
                }
                if(current_packerlist.empty()){
                    this->pri_packerlist_map.erase(*iter_it);
                }
            }
        }
        this->clean_pri_set();
    }

    void continuous(){
        while(!this->priority_set.empty()){
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
        for(auto pri: this->priority_set){
            auto& packer_list = this->pri_packerlist_map[pri];
            for(auto &it: packer_list){
                id_list.push_back(it.get_id());
            }
        }
        return id_list;
    }

    ~Scheduler(){
    }

    private:

    void clean_pri_set(){
        std::set<priority_t> tmp_set;
        for(auto it: this->priority_set){
            if(!this->pri_packerlist_map.contains(it)){
                tmp_set.insert(it);
            }
        }
        for(auto it: tmp_set){
            this->priority_set.erase(it);
        }
    }

    private:
    std::map<co_id_t,  typename packer_list_t::iterator > co_map;
    std::map<priority_t, packer_list_t> pri_packerlist_map;
    std::set<priority_t> priority_set;
    packer_list_t finished_list;
};

template<typename ...T>
Scheduler(T&&...)->Scheduler<std::decay_t<T>...>;

template<>
class Scheduler<coroutine_states<void>, scheduler_mode::FairScheduling, memmory_mode::LinearQueues, std::list>
{
    using CoroutineType = coroutine_states<void>;
    public:
    using outcome_type = CoroutineType::value_type;
    using packer_type = typename CoroutineType::packer_type;

    Scheduler(){}

    Scheduler(packer_type packer){
        this->co_list.push_back(packer);
    }

    template<typename ...T>
    requires Concept_CoroutinePackerType_or_CorotineTypes<T...>
    Scheduler(T&&... t){
        ([&](){
            this->co_list.push_back(packer_type(std::forward<T>(t)));
        }(),...);
    }

    void push_coroutine(packer_type ct){
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

template<>
class Scheduler<coroutine_states<void>, scheduler_mode::PriorityScheduling, memmory_mode::LinearQueues, std::list>
{
    using CoroutineType = coroutine_states<void>;
    public:
    using value_type = CoroutineType::value_type;
    using packer_type = typename CoroutineType::packer_type;
    using packer_list_t = std::list<packer_type>;

    Scheduler(){}

    Scheduler(packer_type packer){
        this->push_coroutine(packer);
    }

    template<typename ...T>
    requires Concept_CoroutinePackerType_or_CorotineTypes<T...>
    Scheduler(T&&... t){
        ([&](){
            this->push_coroutine(packer_type(std::forward<T>(t)));
        }(),...);
    }

    void push_coroutine(packer_type packer){
        auto id = packer.get_id();
        auto pri = packer.get_priority();
        this->priority_set.insert(pri);
        this->pri_packerlist_map[pri].push_back(packer);
    }

    void step(){
        for(auto pri_it = this->priority_set.rbegin(); pri_it!=this->priority_set.rend(); pri_it++){
            auto end_it = get_next_iterator(pri_it);
            for(auto iter_it = this->priority_set.rbegin(); iter_it!=end_it; iter_it++){
                if(!this->pri_packerlist_map.contains(*iter_it)){
                    break;
                }
                auto& current_packerlist =  this->pri_packerlist_map[*iter_it];
                for(auto packerlist_it = current_packerlist.begin(); packerlist_it!=current_packerlist.end(); packerlist_it++){
                    if(*packerlist_it){
                        (*packerlist_it)();
                    }
                    else{
                        current_packerlist.erase(packerlist_it);
                    }
                }
                if(current_packerlist.empty()){
                    this->pri_packerlist_map.erase(*iter_it);
                }
            }
        }
        this->clean_pri_set();
    }

    void continuous(){
        while(!this->priority_set.empty()){
            this->step();
        }
    }

    auto get_running_id_vector(){
        std::vector<co_id_t> id_list;
        for(auto pri: this->priority_set){
            auto& packer_list = this->pri_packerlist_map[pri];
            for(auto &it: packer_list){
                id_list.push_back(it.get_id());
            }
        }
        return id_list;
    }

    ~Scheduler(){
    }

    private:

    void clean_pri_set(){
        std::set<priority_t> tmp_set;
        for(auto it: this->priority_set){
            if(!this->pri_packerlist_map.contains(it)){
                tmp_set.insert(it);
            }
        }
        for(auto it: tmp_set){
            this->priority_set.erase(it);
        }
    }
    private:
    std::map<co_id_t,  typename packer_list_t::iterator > co_map;
    std::map<priority_t, packer_list_t> pri_packerlist_map;
    std::set<priority_t> priority_set;
};