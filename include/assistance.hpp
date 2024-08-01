#pragma once

using co_id_t = std::size_t;
constexpr co_id_t NOP_ID = 0;

using priority_t = std::size_t;
constexpr priority_t HIGHEST_LEVEL = 0xffffffff'ffffffff;
constexpr priority_t LOWEST_LEVEL = 0x00;
constexpr priority_t MIDDLE_LEVEL = HIGHEST_LEVEL/2;

struct slime_type{
    template<typename T>
    operator T(){
        return T{};
    }
};

template<typename T>
T get_next_iterator(T& it){
    auto temp = it;
    temp++;
    return temp;
}

struct awaiter_assister{
    co_id_t id;
    bool await_ready()noexcept { return false;}

    void await_suspend(std::coroutine_handle<> h)noexcept {
        id = reinterpret_cast<co_id_t>(h.address());
        h.resume();
    }

    uint64_t await_resume()noexcept {
        return id;
    }
};

#define current_corotine_id()  co_await awaiter_assister{};