#pragma once
#include<coroutine>
#include<iostream>
#include<format>
#include<memory>

struct promise_type;

template<typename ...T>
class coroutine_states;

//Here are some corotine states, you can customize the status also

struct awaiter_stop:std::suspend_always{
    // constexpr bool await_ready() const noexcept { return false;}

    constexpr void await_suspend(std::coroutine_handle<> h) const noexcept {
    }

    constexpr void await_resume() const noexcept {
    }
};

struct awaiter_run:std::suspend_never{
    // constexpr bool await_ready() const noexcept { return true;}

    constexpr void await_suspend(std::coroutine_handle<> h) const noexcept {
    }

    constexpr void await_resume() const noexcept {
    }
};

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
        std::cout<<"ref_count:"<<*this->ref_count<<std::endl;
        if(*this->ref_count==0)
        {
            delete this->ref_count;
            this->ref_count = nullptr;
            pack_handle.destroy();
            std::cout<<"+++++++++++++++++++++++++++++++++++++++handle destroy+++++++++++++++++++++++++++++++++++++++"<<std::endl;
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

    private:
    std::coroutine_handle<T> pack_handle;
    std::size_t *ref_count;
};

template<typename ...T>
class coroutine_packer;

template<typename ...T>
class coroutine_states{
    public:
    struct promise_type;
    using value_type = std::tuple<T...>;
    using packer_type = coroutine_packer<T...>;

    public:
    struct promise_type{

        auto get_return_object(){
            return coroutine_states(std::coroutine_handle<promise_type>::from_promise(*this));
        }

        awaiter_run initial_suspend(){
            return {};
        }
        awaiter_stop final_suspend()noexcept{
            return {};
        }
        awaiter_stop yield_value(value_type value){
            if(!this->result_ready){
                throw "no value";
            }
            else{
                this->result = value;
            }
            return {};
        }
        ~promise_type(){}

        promise_type():result_ready(true){}

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
        return this->handle().promise().result;
    }

    virtual void operator ()(){
        this->handle().resume();
    }

    virtual bool has_result(){
        return this->handle().promise().result_ready;
    }

    virtual void value_ready(){
        this->handle().promise().result_ready = true;
    }

    auto& get_handle(){
        return this->handle;
    }


    coroutine_states(handle_type&& h):handle(std::forward<handle_type>(h)){}
    coroutine_states(const handle_type& h):handle(h){}

    coroutine_states(coroutine_states&& co_base):handle(std::move(co_base.handle)){}
    coroutine_states(const coroutine_states& co_base):handle(co_base.handle){}

    void operator=(const coroutine_states& co_base){
        this->handle = co_base.handle;
    }
    void operator=(coroutine_states&& co_base){
        this->handle = co_base.handle;
    }
    virtual ~coroutine_states(){
    }


    private:
    handle_type handle;
    coroutine_states(std::coroutine_handle<promise_type>&& h):handle(h.address()){}
};

template<typename ...T>
class coroutine_packer{
    public:
    using co_base_t = coroutine_states<T...>;
    using co_base_handle_t = typename coroutine_states<T...>::handle_type;

    coroutine_packer(co_base_t&& coro_base):
    co_state_ptr(new coroutine_states<T...>(std::forward<co_base_t>(coro_base))),
    coroutine_id((std::size_t)this->co_state_ptr.get())
    {}
    coroutine_packer(const co_base_t& coro_base):
    co_state_ptr(new coroutine_states<T...>(coro_base)),
    coroutine_id((std::size_t)this->co_state_ptr.get())
    {}

    coroutine_packer(coroutine_packer&& co):
    co_state_ptr(co.co_state_ptr),
    coroutine_id((std::size_t)this->co_state_ptr.get())
    {}
    coroutine_packer(const coroutine_packer& co):
    co_state_ptr(co.co_state_ptr),
    coroutine_id((std::size_t)this->co_state_ptr.get())
    {}

    void operator=(coroutine_packer&& co){
        this->co_state_ptr = co.co_state_ptr;
        this->coroutine_id = (std::size_t)this->co_state_ptr.get();
    }
    void operator=(const coroutine_packer& co){
        this->co_state_ptr = co.co_state_ptr;
        this->coroutine_id = (std::size_t)this->co_state_ptr.get();
    }

    bool operator==(const coroutine_packer& co){
        return this->coroutine_id == co.coroutine_id;
    }

    auto get_coroutine_state_ptr(){
        return this->co_state_ptr;
    }

    const auto get_id(){
        return this->coroutine_id;
    }

    virtual ~coroutine_packer(){}
    private:
    std::shared_ptr<coroutine_states<T...>> co_state_ptr;
    std::size_t coroutine_id;
};