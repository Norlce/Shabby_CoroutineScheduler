#pragma once
#include<coroutine>

//Here are some corotine states, you can customize the status also

struct awaiter_stop{
    constexpr bool await_ready() const noexcept { return false;}

    constexpr void await_suspend(std::coroutine_handle<> h) const noexcept {
    }

    constexpr void await_resume() const noexcept {
    }
};

struct awaiter_run{
    constexpr bool await_ready() const noexcept { return true;}

    constexpr void await_suspend(std::coroutine_handle<> h) const noexcept {
    }

    constexpr void await_resume() const noexcept {
    }
};

using awaiter_no_value_ready = awaiter_stop;