#pragma once

namespace cpplinq {
namespace detail {
template<class T>
class CoroutineHandle {
public:
    using Coroutine = Enumerable<T>::Coroutine;

    constexpr CoroutineHandle() noexcept = default;

    CoroutineHandle(Enumerable<T>::promise_type& promise) : coroutine_{Coroutine::from_promise(promise)} {
    }

    CoroutineHandle(const CoroutineHandle& rhs) = default;
    CoroutineHandle& operator=(const CoroutineHandle& rhs) = default;

    CoroutineHandle(CoroutineHandle&& rhs) = default;
    CoroutineHandle& operator=(CoroutineHandle&& rhs) = default;

    ~CoroutineHandle() {
        if (coroutine_) {
            coroutine_.destroy();
        }
    }

    operator const Coroutine&() const {
        return coroutine_;
    }

private:
    Coroutine coroutine_{};
}; // class CoroutineHandler

template<class T>
T noop_selector(T x) {
    return x;
}

template<class T>
bool noop_predicate(const T& x) {
    (void)x;
    return true;
}

template<class T>
struct ComparerTraits {
    template<class U, class = std::invoke_result_t<std::hash<U>, U>>
    static std::true_type HashHelper(int);

    template<class>
    static std::false_type HashHelper(...);

    template<class U, class = decltype(std::declval<U>() < std::declval<U>())>
    static std::true_type LessHelper(int);

    template<class>
    static std::false_type LessHelper(...);

    template<class U, class = decltype(std::declval<U>() == std::declval<U>())>
    static std::true_type EqualHelper(int);

    template<class>
    static std::false_type EqualHelper(...);

    static constexpr bool IsDefaultHashable = decltype(HashHelper<T>(0))::value;
    static constexpr bool IsDefaultLessable = decltype(LessHelper<T>(0))::value;
    static constexpr bool IsDefaultEqualable = decltype(EqualHelper<T>(0))::value;
}; // struct AlgorithmSelector

template<class T>
inline constexpr bool is_default_hashable_v = ComparerTraits<T>::IsDefaultHashable;

template<class T>
inline constexpr bool is_default_lessable_v = ComparerTraits<T>::IsDefaultLessable;

template<class T>
inline constexpr bool is_default_equalable_v = ComparerTraits<T>::IsDefaultEqualable;
} // namespace detail

template<class T>
class Enumerable<T>::Controller {
public:
    constexpr Controller() noexcept = default;

    explicit Controller(promise_type& promise) : variant_{std::make_shared<Variant>(promise)} {
    }

    explicit Controller(const Container& container) : variant_{std::make_shared<Variant>(container)} {
    }

    Controller(const Controller& rhs) noexcept = default;
    Controller& operator=(const Controller& rhs) noexcept = default;

    Controller(Controller&& rhs) noexcept = default;
    Controller& operator=(Controller&& rhs) noexcept = default;

    ~Controller() = default;

    bool operator!() const noexcept {
        return !variant_;
    }

    bool IsCoroutine() const {
        return variant_ && (variant_->index() == kCoroutineIndex);
    }

    const Coroutine& GetCoroutine() const {
        return std::get<kCoroutineIndex>(*variant_);
    }

    bool IsContainer() const {
        return variant_ && (variant_->index() == kContainerIndex);
    }

    const Container& GetContainer() const {
        return std::get<kContainerIndex>(*variant_);
    }

    void Flush() const {
        if (!IsCoroutine()) {
            return;
        }

        Container container{};
        for (auto coroutine = GetCoroutine(); coroutine && !coroutine.done(); coroutine()) {
            container.push_back(*coroutine.promise());
        }

        *variant_ = std::move(container);
    }

    void Reset() {
        variant_.reset();
    }

private:
    using Variant = std::variant<detail::CoroutineHandle<T>, Container>;

    enum Index {
        kCoroutineIndex = 0,
        kContainerIndex = 1,
    };

    std::shared_ptr<Variant> variant_{};
}; // class Enumerable::Controller

#pragma region Enumerable

#pragma region constructors

template<class T>
Enumerable<T>::Enumerable(promise_type& promise) : controller_{promise} {
}

template<class T>
template<class TIterator>
Enumerable<T>::Enumerable(TIterator begin, TIterator end) : controller_{Container(begin, end)} {
}

template<class T>
template<class TEnumerable>
Enumerable<T>::Enumerable(const TEnumerable& enumerable) : Enumerable{std::begin(enumerable), std::end(enumerable)} {
}

template<class T>
Enumerable<T>::Enumerable(std::initializer_list<T> initializer) : Enumerable{std::begin(initializer), std::end(initializer)} {
}

template<class T>
Enumerable<T>::Enumerable(const Container& container) : Enumerable{std::begin(container), std::end(container)} {
}

template<class T>
Enumerable<T>::Enumerable(const Enumerable& rhs) {
    *this = rhs;
}

template<class T>
auto Enumerable<T>::operator=(const Enumerable& rhs) -> Enumerable& {
    if (this != &rhs) {
        controller_ = rhs.controller_;
        controller_.Flush();
        if (!controller_.IsContainer()) {
            controller_.Reset();
        }
    }
    return *this;
}

#pragma endregion constructors

#pragma region common

template<class T>
constexpr auto Enumerable<T>::end() noexcept -> iterator {
    return {};
}

template<class T>
auto Enumerable<T>::Empty() -> Enumerable {
    return {Container{}};
}

template<class T>
auto Enumerable<T>::Range(value_type start, int count) -> Enumerable {
    for (; count > 0; ++start, --count) {
        co_yield start;
    }
}

template<class T>
auto Enumerable<T>::Repeat(value_type element, int count) -> Enumerable {
    for (; count > 0; --count) {
        co_yield element;
    }
}

#pragma endregion common

#pragma region linq

template<class T>
auto Enumerable<T>::begin() && -> iterator {
    return iterator{controller_};
}

template<class T>
auto Enumerable<T>::begin() const & -> iterator {
    controller_.Flush();
    return std::move(*const_cast<Enumerable*>(this)).begin();
}

template<class T>
template<class TAccumulate, class TAggregator, class TResultSelector>
auto Enumerable<T>::Aggregate(
        TAccumulate seed,
        TAggregator aggregator,
        TResultSelector selector) && -> std::invoke_result_t<TResultSelector, TAccumulate> {
    for (auto i = std::move(*this).begin(), j = end(); i != j; ++i) {
        auto&& source = *i;
        seed = aggregator(std::move(seed), source);
    }
    return selector(std::move(seed));
}

template<class T>
template<class TAccumulate, class TAggregator, class TResultSelector>
auto Enumerable<T>::Aggregate(
        TAccumulate seed,
        TAggregator aggregator,
        TResultSelector selector) const & -> std::invoke_result_t<TResultSelector, TAccumulate> {
    controller_.Flush();
    return std::move(*const_cast<Enumerable*>(this)).Aggregate(std::move(seed), aggregator, selector);
}

template<class T>
template<class TAccumulate, class TAggregator>
TAccumulate Enumerable<T>::Aggregate(
        TAccumulate seed,
        TAggregator aggregator) && {
    return std::move(*this).Aggregate(std::move(seed), aggregator, &detail::noop_selector<TAccumulate>);
}

template<class T>
template<class TAccumulate, class TAggregator>
TAccumulate Enumerable<T>::Aggregate(
        TAccumulate seed,
        TAggregator aggregator) const & {
    controller_.Flush();
    return std::move(*const_cast<Enumerable*>(this)).Aggregate(std::move(seed), aggregator);
}

template<class T>
template<class TPredicate>
bool Enumerable<T>::All(TPredicate predicate) && {
    for (auto i = std::move(*this).begin(), j = end(); i != j; ++i) {
        auto&& source = *i;
        if (!predicate(source)) {
            return false;
        }
    }
    return true;
}

template<class T>
template<class TPredicate>
bool Enumerable<T>::All(TPredicate predicate) const & {
    controller_.Flush();
    return std::move(*const_cast<Enumerable*>(this)).All(predicate);
}

template<class T>
template<class TPredicate>
bool Enumerable<T>::Any(TPredicate predicate) && {
    for (auto i = std::move(*this).begin(), j = end(); i != j; ++i) {
        auto&& source = *i;
        if (predicate(source)) {
            return true;
        }
    }
    return false;
}

template<class T>
template<class TPredicate>
bool Enumerable<T>::Any(TPredicate predicate) const & {
    controller_.Flush();
    return std::move(*const_cast<Enumerable*>(this)).Any(predicate);
}

template<class T>
bool Enumerable<T>::Any() && {
    return std::move(*this).Any(&detail::noop_predicate<value_type>);
}

template<class T>
bool Enumerable<T>::Any() const & {
    controller_.Flush();
    return std::move(*const_cast<Enumerable*>(this)).Any();
}

template<class T>
auto Enumerable<T>::Append(value_type element) && -> Enumerable {
    for (auto i = std::move(*this).begin(), j = end(); i != j; ++i) {
        auto&& source = *i;
        co_yield source;
    }
    co_yield element;
}

template<class T>
auto Enumerable<T>::Append(value_type element) const & -> Enumerable {
    controller_.Flush();
    return std::move(*const_cast<Enumerable*>(this)).Append(std::move(element));
}

template<class T>
auto Enumerable<T>::Concat(const Enumerable& other) && -> Enumerable {
    auto otherBegin = other.begin(), otherEnd = other.end();
    for (auto i = std::move(*this).begin(), j = end(); i != j; ++i) {
        auto&& source = *i;
        co_yield source;
    }
    for (; otherBegin != otherEnd; ++otherBegin) {
        auto&& element = *otherBegin;
        co_yield element;
    }
}

template<class T>
auto Enumerable<T>::Concat(const Enumerable& other) const & -> Enumerable {
    controller_.Flush();
    return std::move(*const_cast<Enumerable*>(this)).Concat(other);
}

template<class T>
template<class TEqual>
bool Enumerable<T>::Contains(reference value, TEqual comparer) && {
    for (auto i = std::move(*this).begin(), j = end(); i != j; ++i) {
        auto&& source = *i;
        if (comparer(source, value)) {
            return true;
        }
    }
    return false;
}

template<class T>
template<class TEqual>
bool Enumerable<T>::Contains(reference value, TEqual comparer) const & {
    controller_.Flush();
    return std::move(*const_cast<Enumerable*>(this)).Contains(value, comparer);
}

template<class T>
bool Enumerable<T>::Contains(reference value) && {
    return std::move(*this).Contains(value, std::equal_to<T>{});
}

template<class T>
bool Enumerable<T>::Contains(reference value) const & {
    controller_.Flush();
    return std::move(*const_cast<Enumerable*>(this)).Contains(value);
}

template<class T>
template<class TPredicate>
auto Enumerable<T>::Count(TPredicate predicate) && -> size_type {
    size_type n{};
    for (auto i = std::move(*this).begin(), j = end(); i != j; ++i) {
        auto&& source = *i;
        if (predicate(source)) {
            ++n;
        }
    }
    return n;
}

template<class T>
template<class TPredicate>
auto Enumerable<T>::Count(TPredicate predicate) const & -> size_type {
    controller_.Flush();
    return std::move(*const_cast<Enumerable*>(this)).Count(predicate);
}

template<class T>
auto Enumerable<T>::Count() && -> size_type {
    if (controller_.IsCoroutine()) {
        return std::distance(std::move(*this).begin(), end());
    }
    if (controller_.IsContainer()) {
        return std::size(controller_.GetContainer());
    }
    return {};
}

template<class T>
auto Enumerable<T>::Count() const & -> size_type {
    controller_.Flush();
    return std::move(*const_cast<Enumerable*>(this)).Count();
}

template<class T>
auto Enumerable<T>::DefaultIfEmpty(value_type defaultValue) && -> Enumerable {
    if (!std::move(*this).Any()) {
        return {std::move(defaultValue)};
    }
    return *this;
}

template<class T>
auto Enumerable<T>::DefaultIfEmpty(value_type defaultValue) const & -> Enumerable {
    controller_.Flush();
    return std::move(*const_cast<Enumerable*>(this)).DefaultIfEmpty(std::move(defaultValue));
}

template<class T>
template<class THash>
auto Enumerable<T>::DistinctHash() && -> Enumerable {
    std::unordered_set<value_type, THash> values{};
    for (auto i = std::move(*this).begin(), j = end(); i != j; ++i) {
        auto&& source = *i;
        values.emplace(source);
    }
    for (auto&& value : values) {
        co_yield value;
    }
}

template<class T>
template<class THash>
auto Enumerable<T>::DistinctHash() const & -> Enumerable {
    controller_.Flush();
    return std::move(*const_cast<Enumerable*>(this)).template DistinctHash<THash>();
}

template<class T>
template<class TLess>
auto Enumerable<T>::DistinctLess() && -> Enumerable {
    std::set<value_type, TLess> values{};
    for (auto i = std::move(*this).begin(), j = end(); i != j; ++i) {
        auto&& source = *i;
        values.emplace(source);
    }
    for (auto&& value : values) {
        co_yield value;
    }
}

template<class T>
template<class TLess>
auto Enumerable<T>::DistinctLess() const & -> Enumerable {
    controller_.Flush();
    return std::move(*const_cast<Enumerable*>(this)).template DistinctLess<TLess>();
}

template<class T>
template<class TEqual>
auto Enumerable<T>::DistinctEqual() && -> Enumerable {
    std::vector<value_type> values{};
    TEqual equal{};
    for (auto i = std::move(*this).begin(), j = end(); i != j; ++i) {
        auto&& source = *i;
        auto itr = std::find_if(std::begin(values), std::end(values), [&] (auto&& value) { return equal(source, value); });
        if (itr == std::end(values)) {
            values.emplace_back(source);
        }
    }
    for (auto&& value : values) {
        co_yield value;
    }
}

template<class T>
template<class TEqual>
auto Enumerable<T>::DistinctEqual() const & -> Enumerable {
    controller_.Flush();
    return std::move(*const_cast<Enumerable*>(this)).template DistinctEqual<TEqual>();
}

template<class T>
auto Enumerable<T>::Distinct() && -> Enumerable {
    if constexpr (detail::is_default_hashable_v<value_type>) {
        return std::move(*this).template DistinctHash<std::hash<value_type>>();
    } else if constexpr (detail::is_default_lessable_v<value_type>) {
        return std::move(*this).template DistinctLess<std::less<value_type>>();
    } else if constexpr (detail::is_default_equalable_v<value_type>) {
        return std::move(*this).template DistinctEqual<std::equal_to<value_type>>();
    } else {
        //static_assert(false, "This type doesn't support std::hash, std::less, and std::equal_to. Please call DistinctHash/DistinctLess/DistinctEqual with specific comparer.");
    }
}

template<class T>
auto Enumerable<T>::Distinct() const & -> Enumerable {
    controller_.Flush();
    return std::move(*const_cast<Enumerable*>(this)).Distinct();
}

template<class T>
auto Enumerable<T>::ElementAt(int index, value_type defaultValue) && -> value_type {
    auto i = std::move(*this).begin(), j = end();
    for (; (index > 0) && (i != j); --index, ++i) {
    }
    if (i == j) {
        return defaultValue;
    }
    return *i;
}

template<class T>
auto Enumerable<T>::ElementAt(int index, value_type defaultValue) const & -> value_type {
    controller_.Flush();
    return std::move(*const_cast<Enumerable*>(this)).ElementAt(index, std::move(defaultValue));
}

template<class T>
template<class THash>
auto Enumerable<T>::ExceptHash(const Enumerable& other) && -> Enumerable {
    std::unordered_set<value_type, THash> values{std::begin(other), std::end(other)};
    for (auto i = std::move(*this).template DistinctHash<THash>().begin(), j = end(); i != j; ++i) {
        auto&& source = *i;
        if (values.find(source) == std::end(values)) {
            co_yield source;
        }
    }
}

template<class T>
template<class THash>
auto Enumerable<T>::ExceptHash(const Enumerable& other) const & -> Enumerable {
    controller_.Flush();
    return std::move(*const_cast<Enumerable*>(this)).template ExceptHash<THash>(other);
}

template<class T>
template<class TLess>
auto Enumerable<T>::ExceptLess(const Enumerable& other) && -> Enumerable {
    std::set<value_type, TLess> values{std::begin(other), std::end(other)};
    for (auto i = std::move(*this).template DistinctLess<TLess>().begin(), j = end(); i != j; ++i) {
        auto&& source = *i;
        if (values.find(source) == std::end(values)) {
            co_yield source;
        }
    }
}

template<class T>
template<class TLess>
auto Enumerable<T>::ExceptLess(const Enumerable& other) const & -> Enumerable {
    controller_.Flush();
    return std::move(*const_cast<Enumerable*>(this)).template ExceptLess<TLess>(other);
}

template<class T>
template<class TEqual>
auto Enumerable<T>::ExceptEqual(const Enumerable& other) && -> Enumerable {
    std::vector<value_type> values{std::begin(other), std::end(other)};
    TEqual equal{};
    for (auto i = std::move(*this).template DistinctEqual<TEqual>().begin(), j = end(); i != j; ++i) {
        auto&& source = *i;
        if (std::find_if(std::begin(values), std::end(values), [&] (auto&& value) { return equal(source, value); }) == std::end(values)) {
            co_yield source;
        }
    }
}

template<class T>
template<class TEqual>
auto Enumerable<T>::ExceptEqual(const Enumerable& other) const & -> Enumerable {
    controller_.Flush();
    return std::move(*const_cast<Enumerable*>(this)).template ExceptEqual<TEqual>(other);
}

template<class T>
auto Enumerable<T>::Except(const Enumerable& other) && -> Enumerable {
    if constexpr (detail::is_default_hashable_v<value_type>) {
        return std::move(*this).template ExceptHash<std::hash<value_type>>(other);
    } else if constexpr (detail::is_default_lessable_v<value_type>) {
        return std::move(*this).template ExceptLess<std::less<value_type>>(other);
    } else if constexpr (detail::is_default_equalable_v<value_type>) {
        return std::move(*this).template ExceptEqual<std::equal_to<value_type>>(other);
    } else {
        //static_assert(false, "This type doesn't support std::hash, std::less, and std::equal_to. Please call ExceptHash/ExceptLess/ExceptEqual with specific comparer.");
    }
}

template<class T>
auto Enumerable<T>::Except(const Enumerable& other) const & -> Enumerable {
    controller_.Flush();
    return std::move(*const_cast<Enumerable*>(this)).Except(other);
}

template<class T>
template<class TPredicate>
auto Enumerable<T>::First(TPredicate predicate, value_type defaultValue) && -> value_type {
    for (auto i = std::move(*this).begin(), j = end(); i != j; ++i) {
        auto&& source = *i;
        if (predicate(source)) {
            return source;
        }
    }
    return defaultValue;
}

template<class T>
template<class TPredicate>
auto Enumerable<T>::First(TPredicate predicate, value_type defaultValue) const & -> value_type {
    controller_.Flush();
    return std::move(*const_cast<Enumerable*>(this)).First(predicate, std::move(defaultValue));
}

template<class T>
auto Enumerable<T>::First(value_type defaultValue) && -> value_type {
    return std::move(*this).First(&detail::noop_predicate<value_type>, std::move(defaultValue));
}

template<class T>
auto Enumerable<T>::First(value_type defaultValue) const & -> value_type {
    controller_.Flush();
    return std::move(*const_cast<Enumerable*>(this)).First(std::move(defaultValue));
}

template<class T>
template<class THash, class TKeySelector, class TElementSelector, class TResultSelector>
auto Enumerable<T>::GroupByHash(
        TKeySelector keySelector,
        TElementSelector elementSelector,
        TResultSelector resultSelector) && -> Enumerable<std::invoke_result_t<TResultSelector, std::invoke_result_t<TKeySelector, reference>, Enumerable<std::invoke_result_t<TElementSelector, reference>>>> {
    std::unordered_map<std::invoke_result_t<TKeySelector, reference>, std::vector<std::invoke_result_t<TElementSelector, reference>>, THash> values{};
    for (auto i = std::move(*this).begin(), j = end(); i != j; ++i) {
        auto&& source = *i;
        values[keySelector(source)].emplace_back(elementSelector(source));
    }
    for (auto&& [key, elements] : values) {
        co_yield resultSelector(key, elements);
    }
}

template<class T>
template<class THash, class TKeySelector, class TElementSelector, class TResultSelector>
auto Enumerable<T>::GroupByHash(
        TKeySelector keySelector,
        TElementSelector elementSelector,
        TResultSelector resultSelector) const & -> Enumerable<std::invoke_result_t<TResultSelector, std::invoke_result_t<TKeySelector, reference>, Enumerable<std::invoke_result_t<TElementSelector, reference>>>> {
    controller_.Flush();
    return std::move(*const_cast<Enumerable*>(this)).template GroupByHash<THash>(keySelector, elementSelector, resultSelector);
}

template<class T>
template<class TLess, class TKeySelector, class TElementSelector, class TResultSelector>
auto Enumerable<T>::GroupByLess(
        TKeySelector keySelector,
        TElementSelector elementSelector,
        TResultSelector resultSelector) && -> Enumerable<std::invoke_result_t<TResultSelector, std::invoke_result_t<TKeySelector, reference>, Enumerable<std::invoke_result_t<TElementSelector, reference>>>> {
    std::map<std::invoke_result_t<TKeySelector, reference>, std::vector<std::invoke_result_t<TElementSelector, reference>>, TLess> values{};
    for (auto i = std::move(*this).begin(), j = end(); i != j; ++i) {
        auto&& source = *i;
        values[keySelector(source)].emplace_back(elementSelector(source));
    }
    for (auto&& [key, elements] : values) {
        co_yield resultSelector(key, elements);
    }
}

template<class T>
template<class TLess, class TKeySelector, class TElementSelector, class TResultSelector>
auto Enumerable<T>::GroupByLess(
        TKeySelector keySelector,
        TElementSelector elementSelector,
        TResultSelector resultSelector) const & -> Enumerable<std::invoke_result_t<TResultSelector, std::invoke_result_t<TKeySelector, reference>, Enumerable<std::invoke_result_t<TElementSelector, reference>>>> {
    controller_.Flush();
    return std::move(*const_cast<Enumerable*>(this)).template GroupByLess<TLess>(keySelector, elementSelector, resultSelector);
}

template<class T>
template<class TEqual, class TKeySelector, class TElementSelector, class TResultSelector>
auto Enumerable<T>::GroupByEqual(
        TKeySelector keySelector,
        TElementSelector elementSelector,
        TResultSelector resultSelector) && -> Enumerable<std::invoke_result_t<TResultSelector, std::invoke_result_t<TKeySelector, reference>, Enumerable<std::invoke_result_t<TElementSelector, reference>>>> {
    std::vector<std::pair<std::invoke_result_t<TKeySelector, reference>, std::vector<std::invoke_result_t<TElementSelector, reference>>>> values{};
    TEqual equal{};
    for (auto i = std::move(*this).begin(), j = end(); i != j; ++i) {
        auto&& source = *i;
        auto key = keySelector(source);
        auto element = elementSelector(source);
        auto itr = std::find_if(std::begin(values), std::end(values), [&] (auto&& value) { return value.first == key; });
        if (itr == std::end(values)) {
            values.push_back({key, {element}});
        } else {
            itr->second.emplace_back(element);
        }
    }
    for (auto&& [key, elements] : values) {
        co_yield resultSelector(key, elements);
    }
}

template<class T>
template<class TEqual, class TKeySelector, class TElementSelector, class TResultSelector>
auto Enumerable<T>::GroupByEqual(
        TKeySelector keySelector,
        TElementSelector elementSelector,
        TResultSelector resultSelector) const & -> Enumerable<std::invoke_result_t<TResultSelector, std::invoke_result_t<TKeySelector, reference>, Enumerable<std::invoke_result_t<TElementSelector, reference>>>> {
    controller_.Flush();
    return std::move(*const_cast<Enumerable*>(this)).template GroupByEqual<TEqual>(keySelector, elementSelector, resultSelector);
}

template<class T>
template<class TKeySelector, class TElementSelector, class TResultSelector>
auto Enumerable<T>::GroupBy(
        TKeySelector keySelector,
        TElementSelector elementSelector,
        TResultSelector resultSelector) && -> Enumerable<std::invoke_result_t<TResultSelector, std::invoke_result_t<TKeySelector, reference>, Enumerable<std::invoke_result_t<TElementSelector, reference>>>> {
    using Key = std::invoke_result_t<TKeySelector, reference>;
    if constexpr (detail::is_default_hashable_v<Key>) {
        return std::move(*this).template GroupByHash<std::hash<Key>>(keySelector, elementSelector, resultSelector);
    } else if constexpr (detail::is_default_lessable_v<Key>) {
        return std::move(*this).template GroupByLess<std::less<Key>>(keySelector, elementSelector, resultSelector);
    } else if constexpr (detail::is_default_equalable_v<Key>) {
        return std::move(*this).template GroupByEqual<std::equal_to<Key>>(keySelector, elementSelector, resultSelector);
    } else {
        //static_assert(false, "This type doesn't support std::hash, std::less, and std::equal_to. Please call GroupByHash/GroupByLess/GroupByEqual with specific comparer.");
    }
}

template<class T>
template<class TKeySelector, class TElementSelector, class TResultSelector>
auto Enumerable<T>::GroupBy(
        TKeySelector keySelector,
        TElementSelector elementSelector,
        TResultSelector resultSelector) const & -> Enumerable<std::invoke_result_t<TResultSelector, std::invoke_result_t<TKeySelector, reference>, Enumerable<std::invoke_result_t<TElementSelector, reference>>>> {
    controller_.Flush();
    return std::move(*const_cast<Enumerable*>(this)).GroupBy(keySelector, elementSelector, resultSelector);
}

template<class T>
template<class TKeySelector, class TElementSelector>
auto Enumerable<T>::GroupBy(
        TKeySelector keySelector,
        TElementSelector elementSelector) && -> Enumerable<Grouping<std::invoke_result_t<TKeySelector, reference>, std::invoke_result_t<TElementSelector, reference>>> {
    return std::move(*this).GroupBy(keySelector, elementSelector, [] (auto&& key, auto&& elements) { return Grouping{key, elements}; });
}

template<class T>
template<class TKeySelector, class TElementSelector>
auto Enumerable<T>::GroupBy(
        TKeySelector keySelector,
        TElementSelector elementSelector) const & -> Enumerable<Grouping<std::invoke_result_t<TKeySelector, reference>, std::invoke_result_t<TElementSelector, reference>>> {
    controller_.Flush();
    return std::move(*const_cast<Enumerable*>(this)).GroupBy(keySelector, elementSelector);
}

template<class T>
template<class THash, class TEnumerable, class TOuterKeySelector, class TInnerKeySelector, class TResultSelector>
auto Enumerable<T>::GroupJoinHash(
        const TEnumerable& inner,
        TOuterKeySelector outerKeySelector,
        TInnerKeySelector innerKeySelector,
        TResultSelector resultSelector) && -> Enumerable<std::invoke_result_t<TResultSelector, reference, const Enumerable<std::decay_t<decltype(*std::begin(inner))>>&>> {
    std::unordered_multimap<std::invoke_result_t<TOuterKeySelector, reference>, std::pair<value_type, std::vector<std::decay_t<decltype(*std::begin(inner))>>>, THash> values{};
    for (auto i = std::move(*this).begin(), j = end(); i != j; ++i) {
        auto&& source = *i;
        values.insert({outerKeySelector(source), {source, {}}});
    }
    for (auto&& element : inner) {
        for (auto&& [i, j] = values.equal_range(innerKeySelector(element)); i != j; ++i) {
            i->second.second.emplace_back(element);
        }
    }
    for (auto&& value : values) {
        co_yield resultSelector(value.second.first, value.second.second);
    }
}

template<class T>
template<class THash, class U, class TOuterKeySelector, class TInnerKeySelector, class TResultSelector>
auto Enumerable<T>::GroupJoinHash(
        std::initializer_list<U> inner,
        TOuterKeySelector outerKeySelector,
        TInnerKeySelector innerKeySelector,
        TResultSelector resultSelector) && -> Enumerable<std::invoke_result_t<TResultSelector, reference, const Enumerable<U>&>> {
    return std::move(*this).template GroupJoinHash<THash, std::initializer_list<U>>(inner, outerKeySelector, innerKeySelector, resultSelector);
}

template<class T>
template<class THash, class TEnumerable, class TOuterKeySelector, class TInnerKeySelector, class TResultSelector>
auto Enumerable<T>::GroupJoinHash(
        const TEnumerable& inner,
        TOuterKeySelector outerKeySelector,
        TInnerKeySelector innerKeySelector,
        TResultSelector resultSelector) const & -> Enumerable<std::invoke_result_t<TResultSelector, reference, const Enumerable<std::decay_t<decltype(*std::begin(inner))>>&>> {
    controller_.Flush();
    return std::move(*const_cast<Enumerable*>(this)).template GroupJoinHash<THash>(inner, outerKeySelector, innerKeySelector, resultSelector);
}

template<class T>
template<class THash, class U, class TOuterKeySelector, class TInnerKeySelector, class TResultSelector>
auto Enumerable<T>::GroupJoinHash(
        std::initializer_list<U> inner,
        TOuterKeySelector outerKeySelector,
        TInnerKeySelector innerKeySelector,
        TResultSelector resultSelector) const & -> Enumerable<std::invoke_result_t<TResultSelector, reference, const Enumerable<U>&>> {
    return GroupJoinHash<THash, std::initializer_list<U>>(inner, outerKeySelector, innerKeySelector, resultSelector);
}

template<class T>
template<class TLess, class TEnumerable, class TOuterKeySelector, class TInnerKeySelector, class TResultSelector>
auto Enumerable<T>::GroupJoinLess(
        const TEnumerable& inner,
        TOuterKeySelector outerKeySelector,
        TInnerKeySelector innerKeySelector,
        TResultSelector resultSelector) && -> Enumerable<std::invoke_result_t<TResultSelector, reference, const Enumerable<std::decay_t<decltype(*std::begin(inner))>>&>> {
    std::multimap<std::invoke_result_t<TOuterKeySelector, reference>, std::pair<value_type, std::vector<std::decay_t<decltype(*std::begin(inner))>>>, TLess> values{};
    for (auto i = std::move(*this).begin(), j = end(); i != j; ++i) {
        auto&& source = *i;
        values.insert({outerKeySelector(source), {source, {}}});
    }
    for (auto&& element : inner) {
        for (auto&& [i, j] = values.equal_range(innerKeySelector(element)); i != j; ++i) {
            i->second.second.emplace_back(element);
        }
    }
    for (auto&& value : values) {
        co_yield resultSelector(value.second.first, value.second.second);
    }
}

template<class T>
template<class TLess, class U, class TOuterKeySelector, class TInnerKeySelector, class TResultSelector>
auto Enumerable<T>::GroupJoinLess(
        std::initializer_list<U> inner,
        TOuterKeySelector outerKeySelector,
        TInnerKeySelector innerKeySelector,
        TResultSelector resultSelector) && -> Enumerable<std::invoke_result_t<TResultSelector, reference, const Enumerable<U>&>> {
    return std::move(*this).template GroupJoinLess<TLess, std::initializer_list<U>>(inner, outerKeySelector, innerKeySelector, resultSelector);
}

template<class T>
template<class TLess, class TEnumerable, class TOuterKeySelector, class TInnerKeySelector, class TResultSelector>
auto Enumerable<T>::GroupJoinLess(
        const TEnumerable& inner,
        TOuterKeySelector outerKeySelector,
        TInnerKeySelector innerKeySelector,
        TResultSelector resultSelector) const & -> Enumerable<std::invoke_result_t<TResultSelector, reference, const Enumerable<std::decay_t<decltype(*std::begin(inner))>>&>> {
    controller_.Flush();
    return std::move(*const_cast<Enumerable*>(this)).template GroupJoinLess<TLess>(inner, outerKeySelector, innerKeySelector, resultSelector);
}

template<class T>
template<class TLess, class U, class TOuterKeySelector, class TInnerKeySelector, class TResultSelector>
auto Enumerable<T>::GroupJoinLess(
        std::initializer_list<U> inner,
        TOuterKeySelector outerKeySelector,
        TInnerKeySelector innerKeySelector,
        TResultSelector resultSelector) const & -> Enumerable<std::invoke_result_t<TResultSelector, reference, const Enumerable<U>&>> {
    return GroupJoinLess<TLess, std::initializer_list<U>>(inner, outerKeySelector, innerKeySelector, resultSelector);
}

template<class T>
template<class TEqual, class TEnumerable, class TOuterKeySelector, class TInnerKeySelector, class TResultSelector>
auto Enumerable<T>::GroupJoinEqual(
        const TEnumerable& inner,
        TOuterKeySelector outerKeySelector,
        TInnerKeySelector innerKeySelector,
        TResultSelector resultSelector) && -> Enumerable<std::invoke_result_t<TResultSelector, reference, const Enumerable<std::decay_t<decltype(*std::begin(inner))>>&>> {
    std::vector<std::pair<std::invoke_result_t<TOuterKeySelector, reference>, std::pair<value_type, std::vector<std::decay_t<decltype(*std::begin(inner))>>>>> values{};
    TEqual equal{};
    for (auto i = std::move(*this).begin(), j = end(); i != j; ++i) {
        auto&& source = *i;
        values.push_back({outerKeySelector(source), {source, {}}});
    }
    for (auto&& element : inner) {
        auto innerKey = innerKeySelector(element);
        for (auto&& value : values) {
            if (equal(innerKey, value.first)) {
                value.second.second.emplace_back(element);
            }
        }
    }
    for (auto&& value : values) {
        co_yield resultSelector(value.second.first, value.second.second);
    }
}

template<class T>
template<class TEqual, class U, class TOuterKeySelector, class TInnerKeySelector, class TResultSelector>
auto Enumerable<T>::GroupJoinEqual(
        std::initializer_list<U> inner,
        TOuterKeySelector outerKeySelector,
        TInnerKeySelector innerKeySelector,
        TResultSelector resultSelector) && -> Enumerable<std::invoke_result_t<TResultSelector, reference, const Enumerable<U>&>> {
    return std::move(*this).template GroupJoinEqual<TEqual, std::initializer_list<U>>(inner, outerKeySelector, innerKeySelector, resultSelector);
}

template<class T>
template<class TEqual, class TEnumerable, class TOuterKeySelector, class TInnerKeySelector, class TResultSelector>
auto Enumerable<T>::GroupJoinEqual(
        const TEnumerable& inner,
        TOuterKeySelector outerKeySelector,
        TInnerKeySelector innerKeySelector,
        TResultSelector resultSelector) const & -> Enumerable<std::invoke_result_t<TResultSelector, reference, const Enumerable<std::decay_t<decltype(*std::begin(inner))>>&>> {
    controller_.Flush();
    return std::move(*const_cast<Enumerable*>(this)).template GroupJoinEqual<TEqual>(inner, outerKeySelector, innerKeySelector, resultSelector);
}

template<class T>
template<class TEqual, class U, class TOuterKeySelector, class TInnerKeySelector, class TResultSelector>
auto Enumerable<T>::GroupJoinEqual(
        std::initializer_list<U> inner,
        TOuterKeySelector outerKeySelector,
        TInnerKeySelector innerKeySelector,
        TResultSelector resultSelector) const & -> Enumerable<std::invoke_result_t<TResultSelector, reference, const Enumerable<U>&>> {
    return GroupJoinEqual<TEqual, std::initializer_list<U>>(inner, outerKeySelector, innerKeySelector, resultSelector);
}

template<class T>
template<class TEnumerable, class TOuterKeySelector, class TInnerKeySelector, class TResultSelector>
auto Enumerable<T>::GroupJoin(
        const TEnumerable& inner,
        TOuterKeySelector outerKeySelector,
        TInnerKeySelector innerKeySelector,
        TResultSelector resultSelector) && -> Enumerable<std::invoke_result_t<TResultSelector, reference, const Enumerable<std::decay_t<decltype(*std::begin(inner))>>&>> {
    using Key = std::invoke_result_t<TOuterKeySelector, reference>;
    if constexpr (detail::is_default_hashable_v<Key>) {
        return std::move(*this).template GroupJoinHash<std::hash<Key>>(inner, outerKeySelector, innerKeySelector, resultSelector);
    } else if constexpr (detail::is_default_lessable_v<Key>) {
        return std::move(*this).template GroupJoinLess<std::less<Key>>(inner, outerKeySelector, innerKeySelector, resultSelector);
    } else if constexpr (detail::is_default_equalable_v<Key>) {
        return std::move(*this).template GroupJoinEqual<std::equal_to<Key>>(inner, outerKeySelector, innerKeySelector, resultSelector);
    } else {
        //static_assert(false, "This type doesn't support std::hash, std::less, and std::equal_to. Please call GroupJoinHash/GroupJoinLess/GroupJoinEqual with specific comparer.");
    }
}

template<class T>
template<class U, class TOuterKeySelector, class TInnerKeySelector, class TResultSelector>
auto Enumerable<T>::GroupJoin(
        std::initializer_list<U> inner,
        TOuterKeySelector outerKeySelector,
        TInnerKeySelector innerKeySelector,
        TResultSelector resultSelector) && -> Enumerable<std::invoke_result_t<TResultSelector, reference, const Enumerable<U>&>> {
    return std::move(*this).template GroupJoin<std::initializer_list<U>>(inner, outerKeySelector, innerKeySelector, resultSelector);
}

template<class T>
template<class TEnumerable, class TOuterKeySelector, class TInnerKeySelector, class TResultSelector>
auto Enumerable<T>::GroupJoin(
        const TEnumerable& inner,
        TOuterKeySelector outerKeySelector,
        TInnerKeySelector innerKeySelector,
        TResultSelector resultSelector) const & -> Enumerable<std::invoke_result_t<TResultSelector, reference, const Enumerable<std::decay_t<decltype(*std::begin(inner))>>&>> {
    controller_.Flush();
    return std::move(*const_cast<Enumerable*>(this)).GroupJoin(inner, outerKeySelector, innerKeySelector, resultSelector);
}

template<class T>
template<class U, class TOuterKeySelector, class TInnerKeySelector, class TResultSelector>
auto Enumerable<T>::GroupJoin(
        std::initializer_list<U> inner,
        TOuterKeySelector outerKeySelector,
        TInnerKeySelector innerKeySelector,
        TResultSelector resultSelector) const & -> Enumerable<std::invoke_result_t<TResultSelector, reference, const Enumerable<U>&>> {
    return GroupJoin<std::initializer_list<U>>(inner, outerKeySelector, innerKeySelector, resultSelector);
}

template<class T>
template<class THash>
auto Enumerable<T>::IntersectHash(const Enumerable& other) && -> Enumerable {
    std::unordered_set<value_type, THash> values{std::begin(other), std::end(other)};
    for (auto i = std::move(*this).template DistinctHash<THash>().begin(), j = end(); i != j; ++i) {
        auto&& source = *i;
        if (values.find(source) != values.end()) {
            co_yield source;
        }
    }
}

template<class T>
template<class THash>
auto Enumerable<T>::IntersectHash(const Enumerable& other) const & -> Enumerable {
    controller_.Flush();
    return std::move(*const_cast<Enumerable*>(this)).template IntersectHash<THash>(other);
}

template<class T>
template<class TLess>
auto Enumerable<T>::IntersectLess(const Enumerable& other) && -> Enumerable {
    std::set<value_type, TLess> values{std::begin(other), std::end(other)};
    for (auto i = std::move(*this).template DistinctLess<TLess>().begin(), j = end(); i != j; ++i) {
        auto&& source = *i;
        if (values.find(source) != values.end()) {
            co_yield source;
        }
    }
}

template<class T>
template<class TLess>
auto Enumerable<T>::IntersectLess(const Enumerable& other) const & -> Enumerable {
    controller_.Flush();
    return std::move(*const_cast<Enumerable*>(this)).template IntersectLess<TLess>(other);
}

template<class T>
template<class TEqual>
auto Enumerable<T>::IntersectEqual(const Enumerable& other) && -> Enumerable {
    std::vector<value_type> values{std::begin(other), std::end(other)};
    TEqual equal{};
    for (auto i = std::move(*this).template DistinctEqual<TEqual>().begin(), j = end(); i != j; ++i) {
        auto&& source = *i;
        if (std::find_if(std::begin(values), std::end(values), [&] (auto&& value) { return equal(source, value); }) != std::end(values)) {
            co_yield source;
        }
    }
}

template<class T>
template<class TEqual>
auto Enumerable<T>::IntersectEqual(const Enumerable& other) const & -> Enumerable {
    controller_.Flush();
    return std::move(*const_cast<Enumerable*>(this)).template IntersectEqual<TEqual>(other);
}

template<class T>
auto Enumerable<T>::Intersect(const Enumerable& other) && -> Enumerable {
    if constexpr (detail::is_default_hashable_v<value_type>) {
        return std::move(*this).template IntersectHash<std::hash<value_type>>(other);
    } else if constexpr (detail::is_default_lessable_v<value_type>) {
        return std::move(*this).template IntersectLess<std::less<value_type>>(other);
    } else if constexpr (detail::is_default_equalable_v<value_type>) {
        return std::move(*this).template IntersectEqual<std::equal_to<value_type>>(other);
    } else {
        //static_assert(false, "This type doesn't support std::hash, std::less, and std::equal_to. Please call IntersectHash/IntersectLess/IntersectEqual with specific comparer.");
    }
}

template<class T>
auto Enumerable<T>::Intersect(const Enumerable& other) const & -> Enumerable {
    controller_.Flush();
    return std::move(*const_cast<Enumerable*>(this)).Intersect(other);
}

template<class T>
template<class THash, class TEnumerable, class TOuterKeySelector, class TInnerKeySelector, class TResultSelector>
auto Enumerable<T>::JoinHash(
        const TEnumerable& inner,
        TOuterKeySelector outerKeySelector,
        TInnerKeySelector innerKeySelector,
        TResultSelector resultSelector) && -> Enumerable<std::invoke_result_t<TResultSelector, reference, decltype(*std::begin(inner))>> {
    auto values = std::accumulate(
        std::begin(inner),
        std::end(inner),
        std::unordered_multimap<std::invoke_result_t<TInnerKeySelector, decltype(*std::begin(inner))>, std::decay_t<decltype(*std::begin(inner))>, THash>{},
        [&] (auto&& acc, auto&& val) {
            acc.insert({innerKeySelector(val), val});
            return std::move(acc);
        });
    for (auto i = std::move(*this).begin(), j = end(); i != j; ++i) {
        auto&& source = *i;
        for (auto&& [l, r] = values.equal_range(outerKeySelector(source)); l != r; ++l) {
            co_yield resultSelector(source, l->second);
        }
    }
}

template<class T>
template<class THash, class U, class TOuterKeySelector, class TInnerKeySelector, class TResultSelector>
auto Enumerable<T>::JoinHash(
        std::initializer_list<U> inner,
        TOuterKeySelector outerKeySelector,
        TInnerKeySelector innerKeySelector,
        TResultSelector resultSelector) && -> Enumerable<std::invoke_result_t<TResultSelector, reference, const U&>> {
    return std::move(*this).template JoinHash<THash, std::initializer_list<U>>(inner, outerKeySelector, innerKeySelector, resultSelector);
}

template<class T>
template<class THash, class TEnumerable, class TOuterKeySelector, class TInnerKeySelector, class TResultSelector>
auto Enumerable<T>::JoinHash(
        const TEnumerable& inner,
        TOuterKeySelector outerKeySelector,
        TInnerKeySelector innerKeySelector,
        TResultSelector resultSelector) const & -> Enumerable<std::invoke_result_t<TResultSelector, reference, decltype(*std::begin(inner))>> {
    controller_.Flush();
    return std::move(*const_cast<Enumerable*>(this)).template JoinHash<THash>(inner, outerKeySelector, innerKeySelector, resultSelector);
}

template<class T>
template<class THash, class U, class TOuterKeySelector, class TInnerKeySelector, class TResultSelector>
auto Enumerable<T>::JoinHash(
        std::initializer_list<U> inner,
        TOuterKeySelector outerKeySelector,
        TInnerKeySelector innerKeySelector,
        TResultSelector resultSelector) const & -> Enumerable<std::invoke_result_t<TResultSelector, reference, const U&>> {
    return JoinHash<THash, std::initializer_list<U>>(inner, outerKeySelector, innerKeySelector, resultSelector);
}

template<class T>
template<class TLess, class TEnumerable, class TOuterKeySelector, class TInnerKeySelector, class TResultSelector>
auto Enumerable<T>::JoinLess(
        const TEnumerable& inner,
        TOuterKeySelector outerKeySelector,
        TInnerKeySelector innerKeySelector,
        TResultSelector resultSelector) && -> Enumerable<std::invoke_result_t<TResultSelector, reference, decltype(*std::begin(inner))>> {
    auto values = std::accumulate(
        std::begin(inner),
        std::end(inner),
        std::multimap<std::invoke_result_t<TInnerKeySelector, decltype(*std::begin(inner))>, std::decay_t<decltype(*std::begin(inner))>, TLess>{},
        [&] (auto&& acc, auto&& val) {
            acc.insert({innerKeySelector(val), val});
            return std::move(acc);
        });
    for (auto i = std::move(*this).begin(), j = end(); i != j; ++i) {
        auto&& source = *i;
        for (auto&& [l, r] = values.equal_range(outerKeySelector(source)); l != r; ++l) {
            co_yield resultSelector(source, l->second);
        }
    }
}

template<class T>
template<class TLess, class U, class TOuterKeySelector, class TInnerKeySelector, class TResultSelector>
auto Enumerable<T>::JoinLess(
        std::initializer_list<U> inner,
        TOuterKeySelector outerKeySelector,
        TInnerKeySelector innerKeySelector,
        TResultSelector resultSelector) && -> Enumerable<std::invoke_result_t<TResultSelector, reference, const U&>> {
    return std::move(*this).template JoinLess<TLess, std::initializer_list<U>>(inner, outerKeySelector, innerKeySelector, resultSelector);
}

template<class T>
template<class TLess, class TEnumerable, class TOuterKeySelector, class TInnerKeySelector, class TResultSelector>
auto Enumerable<T>::JoinLess(
        const TEnumerable& inner,
        TOuterKeySelector outerKeySelector,
        TInnerKeySelector innerKeySelector,
        TResultSelector resultSelector) const & -> Enumerable<std::invoke_result_t<TResultSelector, reference, decltype(*std::begin(inner))>> {
    controller_.Flush();
    return std::move(*const_cast<Enumerable*>(this)).template JoinLess<TLess>(inner, outerKeySelector, innerKeySelector, resultSelector);
}

template<class T>
template<class TLess, class U, class TOuterKeySelector, class TInnerKeySelector, class TResultSelector>
auto Enumerable<T>::JoinLess(
        std::initializer_list<U> inner,
        TOuterKeySelector outerKeySelector,
        TInnerKeySelector innerKeySelector,
        TResultSelector resultSelector) const & -> Enumerable<std::invoke_result_t<TResultSelector, reference, const U&>> {
    return JoinLess<TLess, std::initializer_list<U>>(inner, outerKeySelector, innerKeySelector, resultSelector);
}

template<class T>
template<class TEqual, class TEnumerable, class TOuterKeySelector, class TInnerKeySelector, class TResultSelector>
auto Enumerable<T>::JoinEqual(
        const TEnumerable& inner,
        TOuterKeySelector outerKeySelector,
        TInnerKeySelector innerKeySelector,
        TResultSelector resultSelector) && -> Enumerable<std::invoke_result_t<TResultSelector, reference, decltype(*std::begin(inner))>> {
    auto values = std::accumulate(
        std::begin(inner),
        std::end(inner),
        std::vector<std::pair<std::invoke_result_t<TInnerKeySelector, decltype(*std::begin(inner))>, std::decay_t<decltype(*std::begin(inner))>>>{},
        [&] (auto&& acc, auto&& val) {
            acc.push_back({innerKeySelector(val), val});
            return std::move(acc);
        });
    TEqual equal{};
    for (auto i = std::move(*this).begin(), j = end(); i != j; ++i) {
        auto&& source = *i;
        for (auto&& value : values) {
            if (equal(outerKeySelector(source), value.first)) {
                co_yield resultSelector(source, value.second);
            }
        }
    }
}

template<class T>
template<class TEqual, class U, class TOuterKeySelector, class TInnerKeySelector, class TResultSelector>
auto Enumerable<T>::JoinEqual(
        std::initializer_list<U> inner,
        TOuterKeySelector outerKeySelector,
        TInnerKeySelector innerKeySelector,
        TResultSelector resultSelector) && -> Enumerable<std::invoke_result_t<TResultSelector, reference, const U&>> {
    return std::move(*this).template JoinEqual<TEqual, std::initializer_list<U>>(inner, outerKeySelector, innerKeySelector, resultSelector);
}

template<class T>
template<class TEqual, class TEnumerable, class TOuterKeySelector, class TInnerKeySelector, class TResultSelector>
auto Enumerable<T>::JoinEqual(
        const TEnumerable& inner,
        TOuterKeySelector outerKeySelector,
        TInnerKeySelector innerKeySelector,
        TResultSelector resultSelector) const& -> Enumerable<std::invoke_result_t<TResultSelector, reference, decltype(*std::begin(inner))>> {
    controller_.Flush();
    return std::move(*const_cast<Enumerable*>(this)).template JoinEqual<TEqual>(inner, outerKeySelector, innerKeySelector, resultSelector);
}

template<class T>
template<class TEqual, class U, class TOuterKeySelector, class TInnerKeySelector, class TResultSelector>
auto Enumerable<T>::JoinEqual(
        std::initializer_list<U> inner,
        TOuterKeySelector outerKeySelector,
        TInnerKeySelector innerKeySelector,
        TResultSelector resultSelector) const & -> Enumerable<std::invoke_result_t<TResultSelector, reference, const U&>> {
    return JoinEqual<TEqual, std::initializer_list<U>>(inner, outerKeySelector, innerKeySelector, resultSelector);
}

template<class T>
template<class TEnumerable, class TOuterKeySelector, class TInnerKeySelector, class TResultSelector>
auto Enumerable<T>::Join(
        const TEnumerable& inner,
        TOuterKeySelector outerKeySelector,
        TInnerKeySelector innerKeySelector,
        TResultSelector resultSelector) && -> Enumerable<std::invoke_result_t<TResultSelector, reference, decltype(*std::begin(inner))>> {
    using Key = std::invoke_result_t<TOuterKeySelector, reference>;
    if constexpr (detail::is_default_hashable_v<Key>) {
        return std::move(*this).template JoinHash<std::hash<Key>>(inner, outerKeySelector, innerKeySelector, resultSelector);
    } else if constexpr (detail::is_default_lessable_v<Key>) {
        return std::move(*this).template JoinLess<std::less<Key>>(inner, outerKeySelector, innerKeySelector, resultSelector);
    } else if constexpr (detail::is_default_equalable_v<Key>) {
        return std::move(*this).template JoinEqual<std::equal_to<Key>>(inner, outerKeySelector, innerKeySelector, resultSelector);
    } else {
        //static_assert(false, "This type doesn't support std::hash, std::less, and std::equal_to. Please call JoinHash/JoinLess/JoinEqual with specific comparer.");
    }
}

template<class T>
template<class U, class TOuterKeySelector, class TInnerKeySelector, class TResultSelector>
auto Enumerable<T>::Join(
        std::initializer_list<U> inner,
        TOuterKeySelector outerKeySelector,
        TInnerKeySelector innerKeySelector,
        TResultSelector resultSelector) && -> Enumerable<std::invoke_result_t<TResultSelector, reference, const U&>> {
    return std::move(*this).template Join<std::initializer_list<U>>(inner, outerKeySelector, innerKeySelector, resultSelector);
}

template<class T>
template<class TEnumerable, class TOuterKeySelector, class TInnerKeySelector, class TResultSelector>
auto Enumerable<T>::Join(
        const TEnumerable& inner,
        TOuterKeySelector outerKeySelector,
        TInnerKeySelector innerKeySelector,
        TResultSelector resultSelector) const & -> Enumerable<std::invoke_result_t<TResultSelector, reference, decltype(*std::begin(inner))>> {
    controller_.Flush();
    return std::move(*const_cast<Enumerable*>(this)).Join(inner, outerKeySelector, innerKeySelector, resultSelector);
}

template<class T>
template<class U, class TOuterKeySelector, class TInnerKeySelector, class TResultSelector>
auto Enumerable<T>::Join(
        std::initializer_list<U> inner,
        TOuterKeySelector outerKeySelector,
        TInnerKeySelector innerKeySelector,
        TResultSelector resultSelector) const & -> Enumerable<std::invoke_result_t<TResultSelector, reference, const U&>> {
    return Join<std::initializer_list<U>>(inner, outerKeySelector, innerKeySelector, resultSelector);
}

template<class T>
template<class TPredicate>
auto Enumerable<T>::Last(TPredicate predicate, value_type defaultValue) && -> value_type {
    pointer pval = nullptr;
    for (auto i = std::move(*this).begin(), j = end(); i != j; ++i) {
        auto&& source = *i;
        if (predicate(source)) {
            pval = &source;
        }
    }
    if (!pval) {
        return defaultValue;
    }
    return *pval;
}

template<class T>
template<class TPredicate>
auto Enumerable<T>::Last(TPredicate predicate, value_type defaultValue) const & -> value_type {
    controller_.Flush();
    return std::move(*const_cast<Enumerable*>(this)).Last(predicate, std::move(defaultValue));
}

template<class T>
auto Enumerable<T>::Last(value_type defaultValue) && -> value_type {
    return std::move(*this).Last(&detail::noop_predicate<value_type>, std::move(defaultValue));
}

template<class T>
auto Enumerable<T>::Last(value_type defaultValue) const & -> value_type {
    controller_.Flush();
    return std::move(*const_cast<Enumerable*>(this)).Last(std::move(defaultValue));
}

template<class T>
template<class TKeySelector, class TComparer>
auto Enumerable<T>::OrderBy(TKeySelector keySelector, TComparer comparer) && -> Enumerable {
    std::vector<value_type> values{};
    for (auto i = std::move(*this).begin(), j = end(); i != j; ++i) {
        auto&& source = *i;
        values.push_back(source);
    }
    std::sort(std::begin(values), std::end(values), [&] (auto&& lhs, auto&& rhs) { return comparer(keySelector(lhs), keySelector(rhs)); });
    for (auto&& value : values) {
        co_yield value;
    }
}

template<class T>
template<class TKeySelector, class TComparer>
auto Enumerable<T>::OrderBy(TKeySelector keySelector, TComparer comparer) const & -> Enumerable {
    controller_.Flush();
    return std::move(*const_cast<Enumerable*>(this)).OrderBy(keySelector, comparer);
}

template<class T>
template<class TKeySelector>
auto Enumerable<T>::OrderBy(TKeySelector keySelector) && -> Enumerable {
    return std::move(*this).OrderBy(keySelector, std::less<std::invoke_result_t<TKeySelector, reference>>{});
}

template<class T>
template<class TKeySelector>
auto Enumerable<T>::OrderBy(TKeySelector keySelector) const & -> Enumerable {
    controller_.Flush();
    return std::move(*const_cast<Enumerable*>(this)).OrderBy(keySelector);
}

template<class T>
auto Enumerable<T>::OrderBy() && -> Enumerable {
    return std::move(*this).OrderBy(&detail::noop_selector<value_type>, std::less<value_type>{});
}

template<class T>
auto Enumerable<T>::OrderBy() const & -> Enumerable {
    controller_.Flush();
    return std::move(*const_cast<Enumerable*>(this)).OrderBy();
}

template<class T>
template<class TKeySelector>
auto Enumerable<T>::OrderByDescending(TKeySelector keySelector) && -> Enumerable {
    return std::move(*this).OrderBy(keySelector, std::greater<std::invoke_result_t<TKeySelector, reference>>{});
}

template<class T>
template<class TKeySelector>
auto Enumerable<T>::OrderByDescending(TKeySelector keySelector) const & -> Enumerable {
    controller_.Flush();
    return std::move(*const_cast<Enumerable*>(this)).OrderByDescending(keySelector);
}

template<class T>
auto Enumerable<T>::OrderByDescending() && -> Enumerable {
    return std::move(*this).OrderByDescending(&detail::noop_selector<value_type>);
}

template<class T>
auto Enumerable<T>::OrderByDescending() const & -> Enumerable {
    controller_.Flush();
    return std::move(*const_cast<Enumerable*>(this)).OrderByDescending();
}

template<class T>
auto Enumerable<T>::Prepend(value_type element) && -> Enumerable {
    auto i = std::move(*this).begin(), j = end();
    co_yield element;
    for (; i != j; ++i) {
        auto&& source = *i;
        co_yield source;
    }
}

template<class T>
auto Enumerable<T>::Prepend(value_type element) const & -> Enumerable {
    controller_.Flush();
    return std::move(*const_cast<Enumerable*>(this)).Prepend(std::move(element));
}

template<class T>
auto Enumerable<T>::Reverse() && -> Enumerable {
    std::vector<value_type> values{};
    for (auto i = std::move(*this).begin(), j = end(); i != j; ++i) {
        auto&& source = *i;
        values.push_back(source);
    }
    for (auto i = std::rbegin(values), j = std::rend(values); i != j; ++i) {
        auto&& value = *i;
        co_yield value;
    }
}

template<class T>
auto Enumerable<T>::Reverse() const & -> Enumerable {
    controller_.Flush();
    return std::move(*const_cast<Enumerable*>(this)).Reverse();
}

template<class T>
template<class TSelector>
auto Enumerable<T>::Select(TSelector selector) && -> Enumerable<std::invoke_result_t<TSelector, reference>> {
    for (auto i = std::move(*this).begin(), j = end(); i != j; ++i) {
        auto&& source = *i;
        co_yield selector(source);
    }
}

template<class T>
template<class TSelector>
auto Enumerable<T>::Select(TSelector selector) const & -> Enumerable<std::invoke_result_t<TSelector, reference>> {
    controller_.Flush();
    return std::move(*const_cast<Enumerable*>(this)).Select(selector);
}

template<class T>
template<class TSelector>
auto Enumerable<T>::SelectWithIndex(TSelector selector) && -> Enumerable<std::invoke_result_t<TSelector, reference, int>> {
    int index = 0;
    for (auto i = std::move(*this).begin(), j = end(); i != j; ++i, ++index) {
        auto&& source = *i;
        co_yield selector(source, index);
    }
}

template<class T>
template<class TSelector>
auto Enumerable<T>::SelectWithIndex(TSelector selector) const& -> Enumerable<std::invoke_result_t<TSelector, reference, int>> {
    controller_.Flush();
    return std::move(*const_cast<Enumerable*>(this)).SelectWithIndex(selector);
}

template<class T>
template<class TCollectionSelector, class TResultSelector>
auto Enumerable<T>::SelectMany(TCollectionSelector collectionSelector, TResultSelector resultSelector) &&
        -> Enumerable<std::invoke_result_t<TResultSelector, reference, std::decay_t<decltype(*std::begin(std::declval<std::invoke_result_t<TCollectionSelector, reference>>()))>>> {
    for (auto i = std::move(*this).begin(), j = end(); i != j; ++i) {
        auto&& source = *i;
        auto collection = collectionSelector(source);
        for (auto&& element : collection) {
            co_yield resultSelector(source, element);
        }
    }
}

template<class T>
template<class TCollectionSelector, class TResultSelector>
auto Enumerable<T>::SelectMany(TCollectionSelector collectionSelector, TResultSelector resultSelector) const &
        -> Enumerable<std::invoke_result_t<TResultSelector, reference, std::decay_t<decltype(*std::begin(std::declval<std::invoke_result_t<TCollectionSelector, reference>>()))>>> {
    controller_.Flush();
    return std::move(*const_cast<Enumerable*>(this)).SelectMany(collectionSelector, resultSelector);
}

template<class T>
template<class TCollectionSelector>
auto Enumerable<T>::SelectMany(TCollectionSelector collectionSelector) &&
        -> Enumerable<std::decay_t<decltype(*std::begin(std::declval<std::invoke_result_t<TCollectionSelector, reference>>()))>> {
    return std::move(*this).SelectMany(collectionSelector, [] (reference source, auto&& element) { return element; });
}

template<class T>
template<class TCollectionSelector>
auto Enumerable<T>::SelectMany(TCollectionSelector collectionSelector) const &
        -> Enumerable<std::decay_t<decltype(*std::begin(std::declval<std::invoke_result_t<TCollectionSelector, reference>>()))>> {
    controller_.Flush();
    return std::move(*const_cast<Enumerable*>(this)).SelectMany(collectionSelector);
}

template<class T>
template<class TCollectionSelector, class TResultSelector>
auto Enumerable<T>::SelectManyWithIndex(TCollectionSelector collectionSelector, TResultSelector resultSelector) &&
        -> Enumerable<std::invoke_result_t<TResultSelector, reference, std::decay_t<decltype(*std::begin(std::declval<std::invoke_result_t<TCollectionSelector, reference, int>>()))>>> {
    int index = 0;
    for (auto i = std::move(*this).begin(), j = end(); i != j; ++i, ++index) {
        auto&& source = *i;
        auto collection = collectionSelector(source, index);
        for (auto&& element : collection) {
            co_yield resultSelector(source, element);
        }
    }
}

template<class T>
template<class TCollectionSelector, class TResultSelector>
auto Enumerable<T>::SelectManyWithIndex(TCollectionSelector collectionSelector, TResultSelector resultSelector) const &
        -> Enumerable<std::invoke_result_t<TResultSelector, reference, std::decay_t<decltype(*std::begin(std::declval<std::invoke_result_t<TCollectionSelector, reference, int>>()))>>> {
    controller_.Flush();
    return std::move(*const_cast<Enumerable*>(this)).SelectManyWithIndex(collectionSelector, resultSelector);
}

template<class T>
template<class TCollectionSelector>
auto Enumerable<T>::SelectManyWithIndex(TCollectionSelector collectionSelector) &&
        -> Enumerable<std::decay_t<decltype(*std::begin(std::declval<std::invoke_result_t<TCollectionSelector, reference, int>>()))>> {
    return std::move(*this).SelectManyWithIndex(collectionSelector, [] (reference source, auto&& element) { return element; });
}

template<class T>
template<class TCollectionSelector>
auto Enumerable<T>::SelectManyWithIndex(TCollectionSelector collectionSelector) const &
        -> Enumerable<std::decay_t<decltype(*std::begin(std::declval<std::invoke_result_t<TCollectionSelector, reference, int>>()))>> {
    controller_.Flush();
    return std::move(*const_cast<Enumerable*>(this)).SelectManyWithIndex(collectionSelector);
}

template<class T>
template<class TEqual>
bool Enumerable<T>::SequenceEqual(const Enumerable& other, TEqual comparer) && {
    auto i1 = std::move(*this).begin(), j1 = end();
    auto i2 = std::begin(other), j2 = std::end(other);
    for (; (i1 != j1) && (i2 != j2); ++i1, ++i2) {
        auto&& first = *i1;
        auto&& second = *i2;
        if (!comparer(first, second)) {
            return false;
        }
    }
    return (i1 == j1) && (i2 == j2);
}

template<class T>
template<class TEqual>
bool Enumerable<T>::SequenceEqual(const Enumerable& other, TEqual comparer) const & {
    controller_.Flush();
    return std::move(*const_cast<Enumerable*>(this)).SequenceEqual(other, comparer);
}

template<class T>
bool Enumerable<T>::SequenceEqual(const Enumerable& other) && {
    return std::move(*this).SequenceEqual(other, std::equal_to<value_type>{});
}

template<class T>
bool Enumerable<T>::SequenceEqual(const Enumerable& other) const & {
    controller_.Flush();
    return std::move(*const_cast<Enumerable*>(this)).SequenceEqual(other);
}

template<class T>
template<class TPredicate>
auto Enumerable<T>::Single(value_type defaultValue, TPredicate predicate) && -> value_type {
    auto i = std::move(*this).Where(predicate).begin(), j = end();
    if (i == j) {
        return defaultValue;
    }
    auto value = *i;
    ++i;
    if (i != j) {
        return defaultValue;
    }
    return value;
}

template<class T>
template<class TPredicate>
auto Enumerable<T>::Single(value_type defaultValue, TPredicate predicate) const & -> value_type {
    controller_.Flush();
    return std::move(*const_cast<Enumerable*>(this)).Single(std::move(defaultValue), predicate);
}

template<class T>
auto Enumerable<T>::Single(value_type defaultValue) && -> value_type {
    return std::move(*this).Single(std::move(defaultValue), &detail::noop_predicate<value_type>);
}

template<class T>
auto Enumerable<T>::Single(value_type defaultValue) const & -> value_type {
    controller_.Flush();
    return std::move(*const_cast<Enumerable*>(this)).Single(std::move(defaultValue));
}

template<class T>
auto Enumerable<T>::Skip(int count) && -> Enumerable {
    auto i = std::move(*this).begin(), j = end();
    for (; (count > 0) && (i != j); --count, ++i) {
    }
    for (; i != j; ++i) {
        auto&& source = *i;
        co_yield source;
    }
}

template<class T>
auto Enumerable<T>::Skip(int count) const & -> Enumerable {
    controller_.Flush();
    return std::move(*const_cast<Enumerable*>(this)).Skip(count);
}

template<class T>
auto Enumerable<T>::SkipLast(int count) && -> Enumerable {
    return Take(static_cast<int>(Count()) - count); // must flush
}

template<class T>
auto Enumerable<T>::SkipLast(int count) const & -> Enumerable {
    controller_.Flush();
    return std::move(*const_cast<Enumerable*>(this)).SkipLast(count);
}

template<class T>
template<class TPredicate>
auto Enumerable<T>::SkipWhile(TPredicate predicate) && -> Enumerable {
    auto i = std::move(*this).begin(), j = end();
    for (; (i != j) && predicate(*i); ++i) {
    }
    for (; i != j; ++i) {
        auto&& source = *i;
        co_yield source;
    }
}

template<class T>
template<class TPredicate>
auto Enumerable<T>::SkipWhile(TPredicate predicate) const & -> Enumerable {
    controller_.Flush();
    return std::move(*const_cast<Enumerable*>(this)).SkipWhile(predicate);
}

template<class T>
template<class TPredicate>
auto Enumerable<T>::SkipWhileWithIndex(TPredicate predicate) && -> Enumerable {
    int index = 0;
    auto i = std::move(*this).begin(), j = end();
    for (; (i != j) && predicate(*i, index); ++i, ++index) {
    }
    for (; i != j; ++i) {
        auto&& source = *i;
        co_yield source;
    }
}

template<class T>
template<class TPredicate>
auto Enumerable<T>::SkipWhileWithIndex(TPredicate predicate) const & -> Enumerable {
    controller_.Flush();
    return std::move(*const_cast<Enumerable*>(this)).SkipWhileWithIndex(predicate);
}

template<class T>
auto Enumerable<T>::Take(int count) && -> Enumerable {
    for (auto i = std::move(*this).begin(), j = end(); (count > 0) && (i != j); --count, ++i) {
        auto&& source = *i;
        co_yield source;
    }
}

template<class T>
auto Enumerable<T>::Take(int count) const & -> Enumerable {
    controller_.Flush();
    return std::move(*const_cast<Enumerable*>(this)).Take(count);
}

template<class T>
auto Enumerable<T>::TakeLast(int count) && -> Enumerable {
    return Skip(static_cast<int>(Count()) - count); // must flush
}

template<class T>
auto Enumerable<T>::TakeLast(int count) const & -> Enumerable {
    controller_.Flush();
    return std::move(*const_cast<Enumerable*>(this)).TakeLast(count);
}

template<class T>
template<class TPredicate>
auto Enumerable<T>::TakeWhile(TPredicate predicate) && -> Enumerable {
    for (auto i = std::move(*this).begin(), j = end(); (i != j) && predicate(*i); ++i) {
        auto&& source = *i;
        co_yield source;
    }
}

template<class T>
template<class TPredicate>
auto Enumerable<T>::TakeWhile(TPredicate predicate) const & -> Enumerable {
    controller_.Flush();
    return std::move(*const_cast<Enumerable*>(this)).TakeWhile(predicate);
}

template<class T>
template<class TPredicate>
auto Enumerable<T>::TakeWhileWithIndex(TPredicate predicate) && -> Enumerable {
    int index = 0;
    for (auto i = std::move(*this).begin(), j = end(); (i != j) && predicate(*i, index); ++i, ++index) {
        auto&& source = *i;
        co_yield source;
    }
}

template<class T>
template<class TPredicate>
auto Enumerable<T>::TakeWhileWithIndex(TPredicate predicate) const & -> Enumerable {
    controller_.Flush();
    return std::move(*const_cast<Enumerable*>(this)).TakeWhileWithIndex(predicate);
}

template<class T>
template<template<class, class...> class TContainer, class... U>
auto Enumerable<T>::ToContainer() && -> TContainer<value_type, U...> {
    return TContainer<value_type, U...>{std::move(*this).begin(), end()};
}

template<class T>
template<template<class, class...> class TContainer, class... U>
auto Enumerable<T>::ToContainer() const & -> TContainer<value_type, U...> {
    controller_.Flush();
    return std::move(*const_cast<Enumerable*>(this)).template ToContainer<TContainer, U...>();
}

template<class T>
auto Enumerable<T>::ToContainer() && -> Container {
    return Container{std::move(*this).begin(), end()};
}

template<class T>
auto Enumerable<T>::ToContainer() const & -> Container {
    controller_.Flush();
    return std::move(*const_cast<Enumerable*>(this)).ToContainer();
}

template<class T>
template<class THash>
auto Enumerable<T>::UnionHash(const Enumerable& other) && -> Enumerable {
    std::unordered_set<value_type, THash> values{std::begin(other), std::end(other)};
    for (auto i = std::move(*this).begin(), j = end(); i != j; ++i) {
        auto&& source = *i;
        values.insert(source);
    }
    for (auto&& value : values) {
        co_yield value;
    }
}

template<class T>
template<class THash>
auto Enumerable<T>::UnionHash(const Enumerable& other) const & -> Enumerable {
    controller_.Flush();
    return std::move(*const_cast<Enumerable*>(this)).template UnionHash<THash>(other);
}

template<class T>
template<class TLess>
auto Enumerable<T>::UnionLess(const Enumerable& other) && -> Enumerable {
    std::set<value_type, TLess> values{std::begin(other), std::end(other)};
    for (auto i = std::move(*this).begin(), j = end(); i != j; ++i) {
        auto&& source = *i;
        values.insert(source);
    }
    for (auto&& value : values) {
        co_yield value;
    }
}

template<class T>
template<class TLess>
auto Enumerable<T>::UnionLess(const Enumerable& other) const & -> Enumerable {
    controller_.Flush();
    return std::move(*const_cast<Enumerable*>(this)).template UnionLess<TLess>(other);
}

template<class T>
template<class TEqual>
auto Enumerable<T>::UnionEqual(const Enumerable& other) && -> Enumerable {
    std::vector<value_type> values{std::begin(other.DistinctEqual<TEqual>()), std::end(other)};
    TEqual equal{};
    for (auto i = std::move(*this).template DistinctEqual<TEqual>().begin(), j = end(); i != j; ++i) {
        auto&& source = *i;
        if (std::find_if(std::begin(values), std::end(values), [&] (auto&& value) { return equal(source, value); }) == std::end(values)) {
            values.push_back(source);
        }
    }
    for (auto&& value : values) {
        co_yield value;
    }
}

template<class T>
template<class TEqual>
auto Enumerable<T>::UnionEqual(const Enumerable& other) const & -> Enumerable {
    controller_.Flush();
    return std::move(*const_cast<Enumerable*>(this)).template UnionEqual<TEqual>(other);
}

template<class T>
auto Enumerable<T>::Union(const Enumerable& other) && -> Enumerable {
    if constexpr (detail::is_default_hashable_v<value_type>) {
        return std::move(*this).template UnionHash<std::hash<value_type>>(other);
    } else if constexpr (detail::is_default_lessable_v<value_type>) {
        return std::move(*this).template UnionLess<std::less<value_type>>(other);
    } else if constexpr (detail::is_default_equalable_v<value_type>) {
        return std::move(*this).template UnionEqual<std::equal_to<value_type>>(other);
    } else {
        //static_assert(false, "This type doesn't support std::hash, std::less, and std::equal_to. Please call UnionHash/UnionLess/UnionEqual with specific comparer.");
    }
}

template<class T>
auto Enumerable<T>::Union(const Enumerable& other) const & -> Enumerable {
    controller_.Flush();
    return std::move(*const_cast<Enumerable*>(this)).Union(other);
}

template<class T>
template<class TPredicate>
auto Enumerable<T>::Where(TPredicate predicate) && -> Enumerable {
    for (auto i = std::move(*this).begin(), j = end(); i != j; ++i) {
        auto&& source = *i;
        if (predicate(source)) {
            co_yield source;
        }
    }
}

template<class T>
template<class TPredicate>
auto Enumerable<T>::Where(TPredicate predicate) const & -> Enumerable {
    controller_.Flush();
    return std::move(*const_cast<Enumerable*>(this)).Where(predicate);
}

template<class T>
template<class TPredicate>
auto Enumerable<T>::WhereWithIndex(TPredicate predicate) && -> Enumerable {
    int index = 0;
    for (auto i = std::move(*this).begin(), j = end(); i != j; ++i, ++index) {
        auto&& source = *i;
        if (predicate(source, index)) {
            co_yield source;
        }
    }
}

template<class T>
template<class TPredicate>
auto Enumerable<T>::WhereWithIndex(TPredicate predicate) const & -> Enumerable {
    controller_.Flush();
    return std::move(*const_cast<Enumerable*>(this)).WhereWithIndex(predicate);
}

template<class T>
template<class TEnumerable, class TResultSelector>
auto Enumerable<T>::Zip(
        const TEnumerable& other,
        TResultSelector resultSelector) && -> Enumerable<std::invoke_result_t<TResultSelector, reference, decltype(*std::begin(other))>> {
    auto i1 = std::move(*this).begin(), j1 = end();
    auto i2 = std::begin(other), j2 = std::end(other);
    for (; (i1 != j1) && (i2 != j2); ++i1, ++i2) {
        auto&& source1 = *i1;
        auto&& source2 = *i2;
        co_yield resultSelector(source1, source2);
    }
}

template<class T>
template<class U, class TResultSelector>
auto Enumerable<T>::Zip(
        std::initializer_list<U> other,
        TResultSelector resultSelector) && -> Enumerable<std::invoke_result_t<TResultSelector, reference, decltype(*std::begin(other))>> {
    return std::move(*this).template Zip<std::initializer_list<U>>(other, resultSelector);
}

template<class T>
template<class TEnumerable, class TResultSelector>
auto Enumerable<T>::Zip(
        const TEnumerable& other,
        TResultSelector resultSelector) const & -> Enumerable<std::invoke_result_t<TResultSelector, reference, decltype(*std::begin(other))>> {
    controller_.Flush();
    return std::move(*const_cast<Enumerable*>(this)).Zip(other, resultSelector);
}

template<class T>
template<class U, class TResultSelector>
auto Enumerable<T>::Zip(
        std::initializer_list<U> other,
        TResultSelector resultSelector) const & -> Enumerable<std::invoke_result_t<TResultSelector, reference, decltype(*std::begin(other))>> {
    return Zip<std::initializer_list<U>>(other, resultSelector);
}

template<class T>
template<class TEnumerable>
auto Enumerable<T>::Zip(const TEnumerable& other) && -> Enumerable<std::pair<value_type, std::decay_t<decltype(*std::begin(other))>>> {
    return std::move(*this).Zip(other, [] (auto&& source1, auto&& source2) { return std::pair{source1, source2}; });
}

template<class T>
template<class U>
auto Enumerable<T>::Zip(std::initializer_list<U> other) && -> Enumerable<std::pair<value_type, std::decay_t<decltype(*std::begin(other))>>> {
    return std::move(*this).template Zip<std::initializer_list<U>>(other);
}

template<class T>
template<class TEnumerable>
auto Enumerable<T>::Zip(const TEnumerable& other) const & -> Enumerable<std::pair<value_type, std::decay_t<decltype(*std::begin(other))>>> {
    controller_.Flush();
    return std::move(*const_cast<Enumerable*>(this)).Zip(other);
}

template<class T>
template<class U>
auto Enumerable<T>::Zip(std::initializer_list<U> other) const& -> Enumerable<std::pair<value_type, std::decay_t<decltype(*std::begin(other))>>> {
    return Zip<std::initializer_list<U>>(other);
}

#pragma endregion linq

#pragma endregion Enumerable

#pragma region Grouping

template<class TKey, class TElement>
template<class TEnumerable>
Grouping<TKey, TElement>::Grouping(TKey key, const TEnumerable& enumerable) : Enumerable<TElement>{enumerable}, key_{std::move(key)} {
}

template<class TKey, class TElement>
const TKey& Grouping<TKey, TElement>::Key() const {
    return key_;
}

#pragma endregion Grouping

} // namespace cpplinq
