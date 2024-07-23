#pragma once

using co_id_t = std::size_t;

struct slime_type{
    template<typename T>
    operator T(){
        return T{};
    }
};


struct awaiter_assister{
    uint64_t *pp;
    bool await_ready()noexcept { *pp=0; return false;}

    void await_suspend(std::coroutine_handle<> h)noexcept {
        *pp = (uint64_t)h.address();
        h.resume();
    }

    void await_resume()noexcept {
    }
};

auto get_co_addr(uint64_t &p){
    
    return awaiter_assister{&p};
}

#define current_corotine_id(p) {co_await get_co_addr(p);}