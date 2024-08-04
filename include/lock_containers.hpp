#pragma once
#include <utility>
#include <mutex>

template<typename ReturnType, typename ClassType, typename ...OtherType>
constexpr auto get_member_function_addr(ReturnType( ClassType::* pointer)(OtherType...)){
    union{
        decltype(pointer) p;
        void* vp;
    }u;
    u.p = pointer;
    return reinterpret_cast<ReturnType(*)(ClassType*, OtherType...)>(u.vp);
}

template <typename ContainerType>
struct lock_container{
    public:
    template<typename Func, typename ...ParamType>
    auto complete_lock_call(Func f, ParamType&&... params){
        std::lock_guard<std::mutex> lk(complete_mutex);
        auto func_p = get_member_function_addr(f);
        return func_p(&this->contain, std::forward<ParamType>(params)...);
    }
    ContainerType contain;
    private:
    std::mutex complete_mutex;
};