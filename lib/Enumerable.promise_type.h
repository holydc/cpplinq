#pragma once

namespace cpplinq {
template<class T>
class Enumerable<T>::promise_type {
public:
    static constexpr std::suspend_never initial_suspend() noexcept;
    static constexpr std::suspend_always final_suspend() noexcept;
    static constexpr void return_void() noexcept;
    static void unhandled_exception();
    Enumerable get_return_object();
    std::suspend_always yield_value(T value);

    reference operator*() const;

    void await_transform() = delete;

private:
    std::optional<T> value_{};
}; /// class Enumerable::promise_type
} // namespace cpplinq
