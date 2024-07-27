#pragma once
#include<coroutine>
#include<iostream>
#include<format>
#include<memory>
#include<list>
#include"assistance.hpp"
#include"awaiters.hpp"

struct promise_type;

template<typename ...T>
class coroutine_states;

//Contains the various states of the coroutine
template<typename T>
struct handle_packer{
    handle_packer():ref_count(new std::size_t(1)){}
    handle_packer(void* p):pack_handle(std::coroutine_handle<T>::from_address(p)),ref_count(new std::size_t(1)){}
    handle_packer(const handle_packer& h_p):pack_handle(h_p.pack_handle),ref_count(h_p.ref_count){
        ++(*this->ref_count);
    }
    handle_packer(handle_packer&& h_p):pack_handle(h_p.pack_handle),ref_count(h_p.ref_count){
        ++(*this->ref_count);
    }
    void operator=(const handle_packer& h_p){
        this->pack_handle = h_p.pack_handle;
        ++(*this->ref_count);
    }
    ~handle_packer(){
        --(*this->ref_count);   
        if(*this->ref_count==0) 
        {
            delete this->ref_count;
            this->ref_count = nullptr;
            pack_handle.destroy();
        }
    }

    auto& operator()(){
        return this->pack_handle;
    }

    bool handle_valid(){
        return *this->ref_count>0;
    }

    void add_ref_count(std::size_t num){
        ++(*this->ref_count);
    }
    void sub_ref_count(std::size_t num){
        --(*this->ref_count);
    }
    void reset_ref_count(std::size_t num){
        *this->ref_count = num;
    }
    auto get_ref_count(){
        return (*this->ref_count);
    }
    auto address(){
        return this->pack_handle.address();
    }

    private:
    std::coroutine_handle<T> pack_handle;
    std::size_t *ref_count;
};

template<typename CoroutineType = coroutine_states<void>>
class coroutine_packer;

template<typename ...T>
class coroutine_states{
    public:
    using co_packer_com_type = void;
    struct promise_type;
    using value_type = std::tuple<T...>;
    using packer_type = coroutine_packer<coroutine_states<T...>>;

    public:
    struct promise_type{

        auto get_return_object(){
            return coroutine_states(std::coroutine_handle<promise_type>::from_promise(*this));
        }

        awaiter_stop initial_suspend(){
            this->result_ready = false;
            return {};
        }
        awaiter_stop final_suspend()noexcept{
            this->result_ready = false;
            return {};
        }
        awaiter_stop yield_value(value_type value){
            this->result_ready = true;
            this->result = value;
            return {};
        }
        auto await_transform(awaiter_no_value_ready awaiter){
            this->result_ready = false;
            return awaiter;
        }
        awaiter_assister await_transform(awaiter_assister awaiter){
            return awaiter;
        }
        void return_void(){

        }
        ~promise_type(){}

        promise_type():result_ready(false){}

        auto handle_addr(){
            return std::coroutine_handle<promise_type>::from_promise(*this).address();
        }

        void unhandled_exception(){}

        value_type result;
        bool result_ready;

    };

    public:
    using handle_type = handle_packer<promise_type>;
    virtual operator bool(){
        return  !(this->handle().done());
    }

    virtual value_type get_promise_value(){
        this->handle().promise().result_ready = false;
        return this->handle().promise().result;
    }

    virtual void operator ()(){
        this->handle().resume();
    }

    virtual void resume(){
        this->handle().resume();
    }

    virtual bool value_ready(){
        return this->handle().promise().result_ready;
    }

    auto& get_handle(){
        return this->handle;
    }

    auto get_id(){
        return this->coroutine_id;
    }

    coroutine_states(coroutine_states&& co_base):handle(std::move(co_base.handle)),coroutine_id(co_base.coroutine_id){}
    coroutine_states(const coroutine_states& co_base):handle(co_base.handle), coroutine_id(co_base.coroutine_id){}

    void operator=(const coroutine_states& co_base){
        this->handle = co_base.handle;
        this->coroutine_id = co_base.coroutine_id;
    }
    void operator=(coroutine_states&& co_base){
        this->handle = co_base.handle;
        this->coroutine_id = co_base.coroutine_id;
    }
    virtual ~coroutine_states(){
    }

    private:
    handle_type handle;
    co_id_t coroutine_id;
    coroutine_states(std::coroutine_handle<promise_type>&& h):handle(h.address()),coroutine_id(co_id_t(h.address())){}
};

template<>
class coroutine_states<void>{
    public:
    using co_packer_com_type = void;
    struct promise_type;
    using value_type = void;
    using packer_type = coroutine_packer<coroutine_states<void>>;

    public:
    struct promise_type{

        auto get_return_object(){
            return coroutine_states(std::coroutine_handle<promise_type>::from_promise(*this));
        }

        awaiter_stop initial_suspend(){
            return {};
        }
        awaiter_stop final_suspend()noexcept{
            this->result_ready = false;
            return {};
        }
        // template<typename T>
        // awaiter_stop yield_value(T t){
        //     return {};
        // }
        auto await_transform(awaiter_no_value_ready awaiter){
            this->result_ready = false;
            return awaiter;
        }
        awaiter_assister await_transform(awaiter_assister awaiter){
            return awaiter;
        }
        awaiter_stop await_suspend(){
            this->result_ready = false;
            return {};
        }
        void return_void(){

        }
        ~promise_type(){}

        promise_type():result_ready(false){}

        auto handle_addr(){
            return std::coroutine_handle<promise_type>::from_promise(*this).address();
        }

        void unhandled_exception(){}

        bool result_ready;

    };

    public:
    using handle_type = handle_packer<promise_type>;
    virtual operator bool(){
        return  !(this->handle().done());
    }

    virtual value_type get_promise_value(){
        this->handle().promise().result_ready = false;
        return ;
    }

    virtual void operator ()(){
        this->handle().resume();
    }

    virtual void resume(){
        this->handle().resume();
    }

    virtual bool value_ready(){
        return this->handle().promise().result_ready;
    }

    auto& get_handle(){
        return this->handle;
    }

    auto get_id(){
        return this->coroutine_id;
    }

    coroutine_states(coroutine_states&& co_base):handle(std::move(co_base.handle)),coroutine_id(co_base.coroutine_id){}
    coroutine_states(const coroutine_states& co_base):handle(co_base.handle), coroutine_id(co_base.coroutine_id){}

    void operator=(const coroutine_states& co_base){
        this->handle = co_base.handle;
        this->coroutine_id = co_base.coroutine_id;
    }
    void operator=(coroutine_states&& co_base){
        this->handle = co_base.handle;
        this->coroutine_id = co_base.coroutine_id;
    }
    virtual ~coroutine_states(){
    }


    private:
    handle_type handle;
    co_id_t coroutine_id;
    coroutine_states(std::coroutine_handle<promise_type>&& h):handle(h.address()),coroutine_id(co_id_t(h.address())){}
};

template<typename CoroutineType>
class coroutine_packer{
    public:
    using co_packer_com_type = void;
    using value_type = CoroutineType::value_type;
    using co_base_t = CoroutineType;
    using co_base_handle_t = typename CoroutineType::handle_type;
    using value_list_t = std::list<value_type>;

    coroutine_packer(co_base_t&& coro_base, priority_t pri = LOWEST_LEVEL):
    co_state_ptr(new CoroutineType(std::forward<co_base_t>(coro_base))),
    priority(pri){
        this->check_and_push_result();
    }
    coroutine_packer(const co_base_t& coro_base, priority_t pri = LOWEST_LEVEL):
    co_state_ptr(new CoroutineType(coro_base)),
    priority(pri){
        this->check_and_push_result();
    }

    coroutine_packer(coroutine_packer&& co):
    co_state_ptr(co.co_state_ptr),
    result_list(co.result_list),
    priority(co.priority){
        this->check_and_push_result();
    }
    coroutine_packer(const coroutine_packer& co):
    co_state_ptr(co.co_state_ptr),
    result_list(co.result_list),
    priority(co.priority){
        this->check_and_push_result();
    }

    void operator=(coroutine_packer&& co){
        this->co_state_ptr = co.co_state_ptr;
        this->result_list = co.result_list;
    }
    void operator=(const coroutine_packer& co){
        this->co_state_ptr = co.co_state_ptr;
        this->result_list = co.result_list;
    }

    bool operator==(const coroutine_packer& co){
        return this->co_state_ptr.get() == co.co_state_ptr.get();
    }

    void operator()(){
        this->resume();
    }

    operator bool(){
        return  bool(*this->co_state_ptr);
    }

    void resume(){
        this->co_state_ptr->resume();
        this->check_and_push_result();
        
    }

    void check_and_push_result(){
        if(this->co_state_ptr->value_ready()){
            this->result_list.push_back(this->co_state_ptr->get_promise_value());
        }
    }

    auto get_coroutine_state_ptr(){
        return this->co_state_ptr;
    }

    auto get_id(){
        return this->co_state_ptr->get_id();
    }

    const auto get_priority(){
        return this->priority;
    }

    value_list_t get_value_list(){
        return this->result_list;
    }

    virtual ~coroutine_packer(){
    }


    private:
    std::shared_ptr<CoroutineType> co_state_ptr;
    value_list_t result_list;
    priority_t priority;
};

using void_co_t = coroutine_states<void>;

template<>
class coroutine_packer<coroutine_states<void>>{
    public:
    using co_packer_com_type = void;
    using co_base_t = coroutine_states<void>;
    using co_base_handle_t = typename coroutine_states<void>::handle_type;

    coroutine_packer(co_base_t&& coro_base, priority_t pri = LOWEST_LEVEL):
    co_state_ptr(new coroutine_states<void>(std::forward<co_base_t>(coro_base))),
    priority(pri)
    {}
    coroutine_packer(const co_base_t& coro_base, priority_t pri = LOWEST_LEVEL):
    co_state_ptr(new coroutine_states<void>(coro_base)),
    priority(pri)
    {}

    coroutine_packer(coroutine_packer&& co):
    co_state_ptr(co.co_state_ptr),
    priority(co.priority)
    {}
    coroutine_packer(const coroutine_packer& co):
    co_state_ptr(co.co_state_ptr),
    priority(co.priority)
    {}

    void operator=(coroutine_packer&& co){
        this->co_state_ptr = co.co_state_ptr;
    }
    void operator=(const coroutine_packer& co){
        this->co_state_ptr = co.co_state_ptr;
    }

    bool operator==(const coroutine_packer& co){
       return this->co_state_ptr.get() == co.co_state_ptr.get();
    }

    void operator()(){
        this->co_state_ptr->resume();
    }

    operator bool(){
        return  bool(*this->co_state_ptr);
    }

    void resume(){
        this->co_state_ptr->resume();
    }

    auto get_coroutine_state_ptr(){
        return this->co_state_ptr;
    }

    const auto get_id(){
        return this->co_state_ptr->get_id();
    }

    const auto get_priority(){
        return this->priority;
    }

    virtual ~coroutine_packer(){}
    private:
    std::shared_ptr<coroutine_states<void>> co_state_ptr;
    priority_t priority;
};
