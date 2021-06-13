#pragma once

namespace cpplinq {
namespace detail {
template<class T> class Iterator;
} // namespace detail

template<class T>
class Enumerable<T>::iterator {
public:
    using value_type = typename Enumerable::value_type;
    using reference = typename Enumerable::reference;
    using pointer = typename Enumerable::pointer;
    using difference_type = typename Enumerable::difference_type;
    using iterator_category = std::input_iterator_tag;

    constexpr iterator() noexcept = default;

    explicit iterator(const Controller& controller);

    iterator(const iterator& rhs);
    iterator& operator=(const iterator& rhs);

    iterator(iterator&& rhs) noexcept = default;
    iterator& operator=(iterator&& rhs) noexcept = default;

    ~iterator() = default;

    bool operator!=(const iterator& rhs) const;
    bool operator==(const iterator& rhs) const;
    iterator& operator++();
    reference operator*() const;

private:
    Controller controller_{};
    std::unique_ptr<detail::Iterator<T>> impl_{};
}; // class Enumerable::iterator
} // namespace cpplinq
