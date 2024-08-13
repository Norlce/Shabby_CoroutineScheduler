#pragma once
#include <chrono>
#include <functional>
#include<list>
#include<map>
#include <thread>
#include<unordered_map>
#include <utility>
#include<vector>
#include<set>
#include<atomic>
#include<mutex>
#include "awaiters.hpp"
#include "coroutine_base.hpp"
#include "concepts.hpp"


/*
________________________________________________________________________________________________________________________________________________________________________________
#finish_set:
[finished_id]->[finished_id]->[finished_id]->[finished_id]->[finished_id]
###

#co_id_set:
[coroutine_id]->[coroutine_id]->[coroutine_id]->[coroutine_id]->[coroutine_id]
###

#co_set:
[coroutine]->[coroutine]->[coroutine]->[coroutine]->[coroutine]
###

#pri_map:
[priority]      [priority]      [priority]
    |                |              |
 [co_list]       [co_list]       [co_list]
 ###
________________________________________________________________________________________________________________________________________________________________________________
*/
template<typename CoroutineType = coro<void>>
requires Concept_CorotineType<CoroutineType>
class Scheduler_base{
    public:
    using value_type = CoroutineType::value_type;
    using packer_type = typename CoroutineType::packer_type;
    using value_list_t = std::list<value_type>;
    using packer_list_t = std::list<packer_type>;
    public:
    Scheduler_base():
    intterput_(false),
    terminate_(false),
    automatic_(false),
    max_thread_(0),
    thread_num_(0)
    {}

    template<typename ...T>
    Scheduler_base(T&&... t):
    intterput_(false),
    terminate_(false),
    automatic_(false),
    max_thread_(0),
    thread_num_(0){
        (this->push_coroutine(packer_type(std::forward<T>(t))),...);
    }

    public:
    virtual void step() = 0;
    virtual void continuous(){
        while(!this->id_set.empty()&&!this->terminate_){
            while(this->intterput_){
                std::this_thread::yield();
            }
            this->step();
        }
    }

    virtual co_id_t add_co(packer_type packer){
        auto id = this->push_coroutine(packer);
        if(this->automatic_){
            this->start();
        }
        return id;
    }
    virtual void start(){
        auto mission = [&](){
            this->continuous();
            std::unique_lock lk(this->thread_num_mutex);
            this->thread_num_--;
        };
        std::unique_lock lk(this->thread_num_mutex);
        if(this->thread_num_.load()<this->max_thread_.load()){
            auto sub_num = this->max_thread_.load() - this->thread_num_.load();
            this->thread_num_+=sub_num;
            for(auto i = 0; i< sub_num; i++){
                std::thread t(mission);
                t.detach();
            }
        }
    }
    virtual void automatic(bool val = true){
        this->automatic_ = val;
    }
    virtual void intterput(bool val = true){
        this->intterput_.store(val, std::memory_order_release);
    }
    virtual void terminate(bool val = true){
        this->terminate_ = val;
    }
    virtual void set_max_threads(std::size_t max_thr){
        this->max_thread_ = max_thr;
    }
    virtual std::vector<co_id_t> get_running_id_vector(){
        std::lock_guard lk(this->id_set_mutex);
        std::vector<co_id_t> vec;
        for(auto id: this->id_set){
            vec.push_back(id);
        }
        return vec;
    }
    virtual std::size_t thread_num(){
        return this->thread_num_;
    }
    virtual bool empty(){
        return this->id_set.empty();
    }
    virtual std::size_t coroutine_num(){
        return this->co_map.size();
    }
    virtual ~Scheduler_base(){
    }

    protected:
    virtual co_id_t push_coroutine(packer_type packer){
        std::scoped_lock lk(this->co_map_mutex, this->id_set_mutex, this->pri_map_mutex);
        auto priority = packer.get_priority();
        auto id = packer.get_id();
        this->co_map.insert({id, packer});
        this->id_set.insert(id);
        this->pri_map[priority].insert(id);
        return id;
    }

    virtual coro<co_id_t> coroutine_next_co(){
        while(!this->terminate_){
            for(auto pri_map_it = this->pri_map.begin(); pri_map_it!=this->pri_map.end(); pri_map_it++){
                auto end_it = get_next_iterator(pri_map_it);
                for(auto it = this->pri_map.begin(); it!=end_it&&it!=this->pri_map.end(); it++){
                    auto& maped_id_set = it->second;
                    for(auto maped_set_it = maped_id_set.begin(); maped_set_it!=maped_id_set.end(); maped_set_it++){
                        auto id = *maped_set_it;
                        co_yield id;
                        auto &co = this->co_map[id];
                        if(!co){
                            this->id_set.erase(id);
                            maped_set_it = maped_id_set.erase(maped_set_it);
                            this->co_map.erase(id);
                            if(maped_set_it == maped_id_set.end()) break;
                        }
                    }
                    if(maped_id_set.empty()){
                        if(pri_map_it==it){
                            it = this->pri_map.erase(it);
                            pri_map_it = it;
                        }
                        else{
                            it = this->pri_map.erase(it);
                        }
                    }
                    if(it==this->pri_map.end()) break;
                }
                if(pri_map_it==this->pri_map.end()) break;
            }
            co_await awaiter_no_value_ready{};
        }
    }

    protected:
    std::unordered_map<co_id_t, packer_type> co_map;
    std::mutex co_map_mutex;

    std::set<co_id_t> id_set;
    std::mutex id_set_mutex;

    std::map<priority_t, std::set<co_id_t>, std::greater<co_id_t>> pri_map;
    std::mutex pri_map_mutex;

    std::mutex running_mutex;

    std::atomic_bool intterput_;
    std::atomic_bool terminate_;
    std::atomic_bool automatic_;

    std::atomic<std::size_t> max_thread_;
    std::atomic<std::size_t> thread_num_;
    std::mutex thread_num_mutex;
};

template<typename CoroutineType = coro<void>>
class Scheduler;

template<typename CoroutineType>
requires Concept_CorotineType<CoroutineType>
class Scheduler<CoroutineType>:public Scheduler_base<CoroutineType>
{   
    public:
    using value_type = CoroutineType::value_type;
    using packer_type = typename CoroutineType::packer_type;
    using value_list_t = std::list<value_type>;
    using packer_list_t = std::list<packer_type>;

    public:
    Scheduler():
    next_co_packer(this->coroutine_next_co()){}

    template<typename ...T>
    requires Concept_CoroutinePackerType_or_CorotineTypes<T...>
    Scheduler(T&&... t):
    Scheduler_base<CoroutineType>(std::forward<T>(t)...),
    next_co_packer(this->coroutine_next_co()){}

    public:
    void step(){
        std::unique_lock lock_co_map(this->co_map_mutex);
        co_id_t co_id = NOP_ID;
        {
            std::scoped_lock lk(this->running_mutex, this->id_set_mutex, this->finished_id_set_mutex, this->pri_map_mutex);
            if(this->id_set.empty()) return ;
            next_co_packer();
            if(next_co_packer.value_ready()){
                auto [id] = next_co_packer.get_promise_value();
                co_id = id;
            }
            else{
                return;
            }
        }
        if(!this->co_map.contains(co_id)) return;
        auto co = this->co_map[co_id];
        lock_co_map.unlock();
        co();
    }

    value_list_t get_value_list(co_id_t co_id){
        std::scoped_lock lk(this->co_map_mutex, this->finished_id_set_mutex);
        if(!this->co_map.contains(co_id)){
            return {};
        }
        auto& co = this->co_map[co_id];
        auto value_list = co.get_value_list();
        if(!co){
            this->co_map.erase(co_id);
            this->finished_id_set.erase(co_id);
        }
        return value_list;
    }

    auto get_finishied_id_vector(){
        std::lock_guard lk(this->finished_id_set_mutex);
        std::vector<co_id_t> vec;
        for(auto id: this->finished_id_set){
            vec.push_back(id);
        }
        return vec;
    }

    ~Scheduler(){
        while(this->thread_num_.load(std::memory_order_acquire)>0){
            std::this_thread::yield();
        }
    }

    protected:
    coro<co_id_t> coroutine_next_co(){
        while(!this->terminate_){
            for(auto pri_map_it = this->pri_map.begin(); pri_map_it!=this->pri_map.end(); pri_map_it++){
                auto end_it = get_next_iterator(pri_map_it);
                for(auto it = this->pri_map.begin(); it!=end_it&&it!=this->pri_map.end(); it++){
                    auto& maped_id_set = it->second;
                    for(auto maped_set_it = it->second.begin(); maped_set_it!=it->second.end(); maped_set_it++){
                        auto id = *maped_set_it;
                        co_yield id;
                        if(!this->co_map.contains(id))  break;
                        auto &co = this->co_map[id];
                        if(!co){
                            this->finished_id_set.insert(id);
                            this->id_set.erase(id);
                            maped_set_it = maped_id_set.erase(maped_set_it);
                            if(maped_set_it == maped_id_set.end()) break;
                        }
                    }
                    if(maped_id_set.empty()){
                        if(pri_map_it==it){
                            it = this->pri_map.erase(it);
                            pri_map_it = it;
                        }
                        else{
                            it = this->pri_map.erase(it);
                        }
                    }
                    if(it==this->pri_map.end()) break;
                }
                if(pri_map_it==this->pri_map.end()) break;
            }
            co_await awaiter_stop{};
        }
    }

    private:
    std::set<co_id_t> finished_id_set;
    std::mutex finished_id_set_mutex;

    coro<co_id_t> next_co_packer;
};

template<>
class Scheduler<coro<void>>:public Scheduler_base<coro<void>>
{   
    public:
    using CoroutineType = coro<void>;
    using value_type = CoroutineType::value_type;
    using packer_type = typename CoroutineType::packer_type;
    using value_list_t = std::list<value_type>;
    using packer_list_t = std::list<packer_type>;

    public:
    Scheduler():
    next_co_packer(this->coroutine_next_co()){}

    template<typename ...T>
    requires Concept_CoroutinePackerType_or_CorotineTypes<T...>
    Scheduler(T&&... t):
    Scheduler_base<coro<void>>(std::forward<T>(t)...),
    next_co_packer(this->coroutine_next_co()){}

    public:
    void step(){
        std::unique_lock lock_co_map(this->co_map_mutex);
        co_id_t co_id = NOP_ID;
        {
            std::scoped_lock lk(this->running_mutex, this->id_set_mutex, this->pri_map_mutex);
            if(this->id_set.empty()) return ;
            next_co_packer();
            if(next_co_packer.value_ready()){
                auto [id] = next_co_packer.get_promise_value();
                co_id = id;
            }
            else{
                return;
            }
        }
        if(!this->co_map.contains(co_id)) return;
        auto co = this->co_map[co_id];
        lock_co_map.unlock();
        co();
    }

    ~Scheduler(){
        while(this->thread_num_.load(std::memory_order_acquire)>0){
            std::this_thread::yield();
        }
    }

    private:
    coro<co_id_t> next_co_packer;
};

template<typename ...T>
Scheduler(T&&...)->Scheduler<std::decay_t<T>...>;

template<typename ...T>
Scheduler_base(T&&...)->Scheduler_base<std::decay_t<T>...>;