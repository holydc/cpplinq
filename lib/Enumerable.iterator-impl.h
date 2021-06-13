#pragma once

namespace cpplinq {
namespace detail {
template<class T>
class Iterator {
public:
    virtual ~Iterator() = default;

    virtual bool HasNext() const = 0;
    virtual void Next() = 0;
    virtual const T& Value() const = 0;
}; // struct Iterator

template<class T>
class CoroutineIterator : public Iterator<T> {
public:
    explicit CoroutineIterator(typename Enumerable<T>::Coroutine coroutine) : coroutine_{coroutine} {
    }

    bool HasNext() const override {
        return !coroutine_.done();
    }

    void Next() override {
        coroutine_();
    }

    const T& Value() const override {
        return *coroutine_.promise();
    }

private:
    typename Enumerable<T>::Coroutine coroutine_{};
}; // class CoroutineIterator

template<class T>
class ContainerIterator : public Iterator<T> {
public:
    explicit ContainerIterator(const typename Enumerable<T>::Container& container) : begin_{std::begin(container)}, end_{std::end(container)} {
    }

    bool HasNext() const override {
        return begin_ != end_;
    }

    void Next() override {
        ++begin_;
    }

    const T& Value() const override {
        return *begin_;
    }

private:
    typename Enumerable<T>::Container::const_iterator begin_{};
    typename Enumerable<T>::Container::const_iterator end_{};
}; // class ContainerIterator
} // namespace detail

template<class T>
Enumerable<T>::iterator::iterator(const Controller& controller) : controller_{controller} {
    if (controller_.IsCoroutine()) {
        impl_ = std::make_unique<detail::CoroutineIterator<T>>(controller_.GetCoroutine());
    } else if (controller_.IsContainer()) {
        impl_ = std::make_unique<detail::ContainerIterator<T>>(controller_.GetContainer());
    } else {
        controller_.Reset();
    }
}

template<class T>
Enumerable<T>::iterator::iterator(const iterator& rhs) {
    *this = rhs;
}

template<class T>
auto Enumerable<T>::iterator::operator=(const iterator& rhs) -> iterator& {
    if (this != &rhs) {
        controller_ = rhs.controller_;
        controller_.Flush();
        if (controller_.IsContainer()) {
            impl_ = std::make_unique<detail::ContainerIterator<T>>(controller_.GetContainer());
        } else {
            controller_.Reset();
            impl_.reset();
        }
    }
    return *this;
}

template<class T>
bool Enumerable<T>::iterator::operator!=(const iterator& rhs) const {
    return !rhs.controller_ && impl_ && impl_->HasNext();
}

template<class T>
bool Enumerable<T>::iterator::operator==(const iterator& rhs) const {
    return !(*this != rhs);
}

template<class T>
auto Enumerable<T>::iterator::operator++() -> iterator& {
    impl_->Next();
    return *this;
}

template<class T>
auto Enumerable<T>::iterator::operator*() const -> reference {
    return impl_->Value();
}
} // namespace cpplinq
