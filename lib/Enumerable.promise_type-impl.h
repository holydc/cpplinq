#pragma once

namespace cpplinq {
template<class T>
constexpr std::suspend_never Enumerable<T>::promise_type::initial_suspend() noexcept {
    return {};
}

template<class T>
constexpr std::suspend_always Enumerable<T>::promise_type::final_suspend() noexcept {
    return {};
}

template<class T>
constexpr void Enumerable<T>::promise_type::return_void() noexcept {
}

template<class T>
void Enumerable<T>::promise_type::unhandled_exception() {
    throw;
}

template<class T>
auto Enumerable<T>::promise_type::get_return_object() -> Enumerable {
    return *this;
}

template<class T>
std::suspend_always Enumerable<T>::promise_type::yield_value(T value) {
    value_ = std::move(value);
    return {};
}

template<class T>
auto Enumerable<T>::promise_type::operator*() const -> reference {
    return *value_;
}
} // namespace cpplinq
