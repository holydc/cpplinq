#pragma once

#include <algorithm>
#include <coroutine>
#include <functional>
#include <initializer_list>
#include <iterator>
#include <map>
#include <memory>
#include <numeric>
#include <optional>
#include <set>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

namespace cpplinq {

#pragma region Enumerable

template<class TKey, class TElement>
class Grouping;

template<class T>
class Enumerable {
public:

#pragma region types

    class promise_type;
    class iterator;

    using value_type = T;
    using reference = const value_type&;
    using const_reference = reference;
    using pointer = const value_type*;
    using const_pointer = pointer;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using const_iterator = iterator;

    using Coroutine = std::coroutine_handle<promise_type>;
    using Container = std::vector<value_type>;

#pragma endregion types

#pragma region constructors

    template<class TIterator>
    Enumerable(TIterator begin, TIterator end);

    template<class TEnumerable>
    Enumerable(const TEnumerable& enumerable);

    Enumerable(std::initializer_list<T> initializer);

    Enumerable(const Container& container);

    Enumerable(const Enumerable& rhs);
    Enumerable& operator=(const Enumerable& rhs);

    Enumerable(Enumerable&& rhs) noexcept = default;
    Enumerable& operator=(Enumerable&& rhs) noexcept = default;

    ~Enumerable() = default;

#pragma endregion constructors

#pragma region common

    static constexpr iterator end() noexcept;

    //! Returns an empty Enumerable<T> that has the specified type argument.
    //!
    //! @returns An empty Enumerable<T> whose type argument is T.
    //!
    //! @see https://docs.microsoft.com/en-us/dotnet/api/system.linq.enumerable.empty?view=net-5.0
    static Enumerable Empty();

    static Enumerable Range(value_type start, int count);

    static Enumerable Repeat(value_type element, int count);

#pragma endregion common

#pragma region linq

    iterator begin() &&;

    iterator begin() const &;

    //! Applies an accumulator function over a sequence. The specified seed value is used as the initial accumulator value, and the specified function is used to select the result value.
    //!
    //! @tparam TAccumulate The type of the accumulator value.
    //! @tparam TResult The type of the resulting value. Return type of TResultSelector.
    //!
    //! @tparam TAggregator function<TAccumulate(TAccumulate, const T&)>.
    //! @tparam TResultSelector function<TResult(const TAccumulate&)>.
    //!
    //! @param seed The initial accumulator value.
    //! @param aggregator An accumulator function to be invoked on each element.
    //! @param selector A function to transform the final accumulator value into the result value.
    //!
    //! @returns The transformed final accumulator value.
    //!
    //! @see https://docs.microsoft.com/en-us/dotnet/api/system.linq.enumerable.aggregate?view=net-5.0#System_Linq_Enumerable_Aggregate__3_System_Collections_Generic_IEnumerable___0____1_System_Func___1___0___1__System_Func___1___2__
    template<class TAccumulate, class TAggregator, class TResultSelector>
    auto Aggregate(
        TAccumulate seed,
        TAggregator aggregator,
        TResultSelector selector) && -> std::invoke_result_t<TResultSelector, TAccumulate>;

    template<class TAccumulate, class TAggregator, class TResultSelector>
    auto Aggregate(
        TAccumulate seed,
        TAggregator aggregator,
        TResultSelector selector) const & -> std::invoke_result_t<TResultSelector, TAccumulate>;

    //! Applies an accumulator function over a sequence. The specified seed value is used as the initial accumulator value.
    //!
    //! @tparam TAccumulate The type of the accumulator value.
    //!
    //! @tparam TAggregator function<TAccumulate(TAccumulate, const T&)>.
    //!
    //! @param seed The initial accumulator value.
    //! @param aggregator An accumulator function to be invoked on each element.
    //!
    //! @returns The final accumulator value.
    //!
    //! @see https://docs.microsoft.com/en-us/dotnet/api/system.linq.enumerable.aggregate?view=net-5.0#System_Linq_Enumerable_Aggregate__2_System_Collections_Generic_IEnumerable___0____1_System_Func___1___0___1__
    template<class TAccumulate, class TAggregator>
    TAccumulate Aggregate(
        TAccumulate seed,
        TAggregator aggregator) &&;

    template<class TAccumulate, class TAggregator>
    TAccumulate Aggregate(
        TAccumulate seed,
        TAggregator aggregator) const &;

    //! Determines whether all elements of a sequence satisfy a condition.
    //!
    //! @tparam TPredicate function<bool(const T&)>.
    //!
    //! @param predicate A function to test each element for a condition.
    //!
    //! @returns true if every element of the source sequence passes the test in the specified predicate, or if the sequence is empty; otherwise, false.
    //!
    //! @see https://docs.microsoft.com/en-us/dotnet/api/system.linq.enumerable.all?view=net-5.0
    template<class TPredicate>
    bool All(TPredicate predicate) &&;

    template<class TPredicate>
    bool All(TPredicate predicate) const &;

    //! Determines whether any element of a sequence satisfies a condition.
    //!
    //! @tparam TPredicate function<bool(const T&)>.
    //!
    //! @param predicate A function to test each element for a condition.
    //!
    //! @returns true if the source sequence is not empty and at least one of its elements passes the test in the specified predicate; otherwise, false.
    //!
    //! @see https://docs.microsoft.com/en-us/dotnet/api/system.linq.enumerable.any?view=net-5.0#System_Linq_Enumerable_Any__1_System_Collections_Generic_IEnumerable___0__System_Func___0_System_Boolean__
    template<class TPredicate>
    bool Any(TPredicate predicate) &&;

    template<class TPredicate>
    bool Any(TPredicate predicate) const &;

    //! Determines whether a sequence contains any elements.
    //!
    //! @returns true if the source sequence contains any elements; otherwise, false.
    //!
    //! @see https://docs.microsoft.com/en-us/dotnet/api/system.linq.enumerable.any?view=net-5.0#System_Linq_Enumerable_Any__1_System_Collections_Generic_IEnumerable___0__
    bool Any() &&;

    bool Any() const &;

    //! Appends a value to the end of the sequence.
    //!
    //! @param element The value to append to source.
    //!
    //! @returns A new sequence that ends with element.
    //!
    //! @see https://docs.microsoft.com/en-us/dotnet/api/system.linq.enumerable.append?view=net-5.0
    Enumerable Append(value_type element) &&;

    Enumerable Append(value_type element) const &;

    //! Concatenates two sequences.
    //!
    //! @param other The sequence to concatenate to the first sequence.
    //!
    //! @returns An Enumerable<T> that contains the concatenated elements of the two input sequences.
    //!
    //! @see https://docs.microsoft.com/en-us/dotnet/api/system.linq.enumerable.concat?view=net-5.0
    Enumerable Concat(const Enumerable& other) &&;

    Enumerable Concat(const Enumerable& other) const &;

    //! Determines whether a sequence contains a specified element by using a specified TEqual.
    //!
    //! @tparam TEqual function<bool(const T&, const T&)>.
    //!
    //! @param value The value to locate in the sequence.
    //! @param comparer An equality comparer to compare values.
    //!
    //! @returns true if the source sequence contains an element that has the specified value; otherwise, false.
    //!
    //! @see https://docs.microsoft.com/en-us/dotnet/api/system.linq.enumerable.contains?view=net-5.0#System_Linq_Enumerable_Contains__1_System_Collections_Generic_IEnumerable___0____0_System_Collections_Generic_IEqualityComparer___0__
    template<class TEqual>
    bool Contains(reference value, TEqual comparer) &&;

    template<class TEqual>
    bool Contains(reference value, TEqual comparer) const &;

    //! Determines whether a sequence contains a specified element by using the default equality comparer.
    //!
    //! @param value The value to locate in the sequence.
    //!
    //! @returns true if the source sequence contains an element that has the specified value; otherwise, false.
    //!
    //! @see https://docs.microsoft.com/en-us/dotnet/api/system.linq.enumerable.contains?view=net-5.0#System_Linq_Enumerable_Contains__1_System_Collections_Generic_IEnumerable___0____0_
    bool Contains(reference value) &&;

    bool Contains(reference value) const &;

    //! Returns a number that represents how many elements in the specified sequence satisfy a condition.
    //!
    //! @tparam TPredicate function<bool(const T&)>.
    //!
    //! @param predicate A function to test each element for a condition.
    //!
    //! @returns A number that represents how many elements in the sequence satisfy the condition in the predicate function.
    //!
    //! @see https://docs.microsoft.com/en-us/dotnet/api/system.linq.enumerable.count?view=net-5.0#System_Linq_Enumerable_Count__1_System_Collections_Generic_IEnumerable___0__System_Func___0_System_Boolean__
    template<class TPredicate>
    size_type Count(TPredicate predicate) &&;

    template<class TPredicate>
    size_type Count(TPredicate predicate) const &;

    //! Returns the number of elements in a sequence.
    //!
    //! @returns The number of elements in the input sequence.
    //!
    //! @see https://docs.microsoft.com/en-us/dotnet/api/system.linq.enumerable.count?view=net-5.0#System_Linq_Enumerable_Count__1_System_Collections_Generic_IEnumerable___0__
    size_type Count() &&;

    size_type Count() const &;

    //! Returns the elements of the specified sequence or the specified value in a singleton collection if the sequence is empty.
    //!
    //! @param defaultValue The value to return if the sequence is empty.
    //!
    //! @returns An Enumerable<T> that contains defaultValue if source is empty; otherwise, source.
    //!
    //! @see https://docs.microsoft.com/en-us/dotnet/api/system.linq.enumerable.defaultifempty?view=net-5.0#System_Linq_Enumerable_DefaultIfEmpty__1_System_Collections_Generic_IEnumerable___0____0_
    Enumerable DefaultIfEmpty(value_type defaultValue) &&;

    Enumerable DefaultIfEmpty(value_type defaultValue) const &;

    template<class THash>
    Enumerable DistinctHash() &&;

    template<class THash>
    Enumerable DistinctHash() const &;

    template<class TLess>
    Enumerable DistinctLess() &&;

    template<class TLess>
    Enumerable DistinctLess() const &;

    template<class TEqual>
    Enumerable DistinctEqual() &&;

    template<class TEqual>
    Enumerable DistinctEqual() const &;

    //! Returns distinct elements from a sequence by using the default equality comparer to compare values.
    //!
    //! @returns An Enumerable<T> that contains distinct elements from the source sequence.
    //!
    //! @see https://docs.microsoft.com/en-us/dotnet/api/system.linq.enumerable.distinct?view=net-5.0#System_Linq_Enumerable_Distinct__1_System_Collections_Generic_IEnumerable___0__
    Enumerable Distinct() &&;

    Enumerable Distinct() const &;

    //! Returns the element at a specified index in a sequence or a default value if the index is out of range.
    //!
    //! @param index The zero-based index of the element to retrieve.
    //! @param defaultValue The value to return if the index is out of range.
    //!
    //! @returns defaultValue if the index is outside the bounds of the source sequence; otherwise, the element at the specified position in the source sequence.
    //!
    //! @see https://docs.microsoft.com/en-us/dotnet/api/system.linq.enumerable.elementatordefault?view=net-5.0
    value_type ElementAt(int index, value_type defaultValue) &&;

    value_type ElementAt(int index, value_type defaultValue) const &;

    template<class THash>
    Enumerable ExceptHash(const Enumerable& other) &&;

    template<class THash>
    Enumerable ExceptHash(const Enumerable& other) const &;

    template<class TLess>
    Enumerable ExceptLess(const Enumerable& other) &&;

    template<class TLess>
    Enumerable ExceptLess(const Enumerable& other) const &;

    template<class TEqual>
    Enumerable ExceptEqual(const Enumerable& other) &&;

    template<class TEqual>
    Enumerable ExceptEqual(const Enumerable& other) const &;

    //! Produces the set difference of two sequences by using the default equality comparer to compare values.
    //!
    //! @param other An Enumerable<T> whose elements that also occur in the first sequence will cause those elements to be removed from the returned sequence.
    //!
    //! @returns A sequence that contains the set difference of the elements of two sequences.
    //!
    //! @see https://docs.microsoft.com/en-us/dotnet/api/system.linq.enumerable.except?view=net-5.0#System_Linq_Enumerable_Except__1_System_Collections_Generic_IEnumerable___0__System_Collections_Generic_IEnumerable___0__
    Enumerable Except(const Enumerable& other) &&;

    Enumerable Except(const Enumerable& other) const &;

    //! Returns the first element of the sequence that satisfies a condition or a default value if no such element is found.
    //!
    //! @tparam TPredicate function<bool(const T&)>.
    //!
    //! @param predicate A function to test each element for a condition.
    //! @param defaultValue The value to return if no such element is found.
    //!
    //! @returns defaultValue if source is empty or if no element passes the test specified by predicate; otherwise, the first element in source that passes the test specified by predicate.
    //!
    //! @see https://docs.microsoft.com/en-us/dotnet/api/system.linq.enumerable.firstordefault?view=net-5.0#System_Linq_Enumerable_FirstOrDefault__1_System_Collections_Generic_IEnumerable___0__System_Func___0_System_Boolean__
    template<class TPredicate>
    value_type First(TPredicate predicate, value_type defaultValue) &&;

    template<class TPredicate>
    value_type First(TPredicate predicate, value_type defaultValue) const &;

    //! Returns the first element of a sequence, or a default value if the sequence contains no elements.
    //!
    //! @param defaultValue The value to return if the sequence contains no elements.
    //!
    //! @returns defaultValue if source is empty; otherwise, the first element in source.
    //!
    //! @see https://docs.microsoft.com/en-us/dotnet/api/system.linq.enumerable.firstordefault?view=net-5.0#System_Linq_Enumerable_FirstOrDefault__1_System_Collections_Generic_IEnumerable___0__
    value_type First(value_type defaultValue) &&;

    value_type First(value_type defaultValue) const &;

    template<class THash, class TKeySelector, class TElementSelector, class TResultSelector>
    auto GroupByHash(
        TKeySelector keySelector,
        TElementSelector elementSelector,
        TResultSelector resultSelector) && -> Enumerable<std::invoke_result_t<TResultSelector, std::invoke_result_t<TKeySelector, reference>, Enumerable<std::invoke_result_t<TElementSelector, reference>>>>;

    template<class THash, class TKeySelector, class TElementSelector, class TResultSelector>
    auto GroupByHash(
        TKeySelector keySelector,
        TElementSelector elementSelector,
        TResultSelector resultSelector) const & -> Enumerable<std::invoke_result_t<TResultSelector, std::invoke_result_t<TKeySelector, reference>, Enumerable<std::invoke_result_t<TElementSelector, reference>>>>;

    template<class TLess, class TKeySelector, class TElementSelector, class TResultSelector>
    auto GroupByLess(
        TKeySelector keySelector,
        TElementSelector elementSelector,
        TResultSelector resultSelector) && -> Enumerable<std::invoke_result_t<TResultSelector, std::invoke_result_t<TKeySelector, reference>, Enumerable<std::invoke_result_t<TElementSelector, reference>>>>;

    template<class TLess, class TKeySelector, class TElementSelector, class TResultSelector>
    auto GroupByLess(
        TKeySelector keySelector,
        TElementSelector elementSelector,
        TResultSelector resultSelector) const & -> Enumerable<std::invoke_result_t<TResultSelector, std::invoke_result_t<TKeySelector, reference>, Enumerable<std::invoke_result_t<TElementSelector, reference>>>>;

    template<class TEqual, class TKeySelector, class TElementSelector, class TResultSelector>
    auto GroupByEqual(
        TKeySelector keySelector,
        TElementSelector elementSelector,
        TResultSelector resultSelector) && -> Enumerable<std::invoke_result_t<TResultSelector, std::invoke_result_t<TKeySelector, reference>, Enumerable<std::invoke_result_t<TElementSelector, reference>>>>;

    template<class TEqual, class TKeySelector, class TElementSelector, class TResultSelector>
    auto GroupByEqual(
        TKeySelector keySelector,
        TElementSelector elementSelector,
        TResultSelector resultSelector) const & -> Enumerable<std::invoke_result_t<TResultSelector, std::invoke_result_t<TKeySelector, reference>, Enumerable<std::invoke_result_t<TElementSelector, reference>>>>;

    //! Groups the elements of a sequence according to a specified key selector function and creates a result value from each group and its key. The elements of each group are projected by using a specified function.
    //!
    //! @tparam TKey The type of the key returned by keySelector. Return type of TKeySelector.
    //! @tparam TElement The type of the elements in each Grouping<TKey,TElement>. Return type of TElementSelector.
    //! @tparam TResult The type of the result value returned by resultSelector. Return type of TResultSelector.
    //!
    //! @tparam TKeySelector function<TKey(const T&)>.
    //! @tparam TElementSelector function<TElement(const T&)>.
    //! @tparam TResultSelector function<TResult(const TKey&, const Enumerable<TElement>&)>.
    //!
    //! @param keySelector A function to extract the key for each element.
    //! @param elementSelector A function to map each source element to an element in an Grouping<TKey,TElement>.
    //! @param resultSelector A function to create a result value from each group.
    //!
    //! @returns A collection of elements of type TResult where each element represents a projection over a group and its key.
    //!
    //! @see https://docs.microsoft.com/en-us/dotnet/api/system.linq.enumerable.groupby?view=net-5.0#System_Linq_Enumerable_GroupBy__4_System_Collections_Generic_IEnumerable___0__System_Func___0___1__System_Func___0___2__System_Func___1_System_Collections_Generic_IEnumerable___2____3__
    template<class TKeySelector, class TElementSelector, class TResultSelector>
    auto GroupBy(
        TKeySelector keySelector,
        TElementSelector elementSelector,
        TResultSelector resultSelector) && -> Enumerable<std::invoke_result_t<TResultSelector, std::invoke_result_t<TKeySelector, reference>, Enumerable<std::invoke_result_t<TElementSelector, reference>>>>;

    template<class TKeySelector, class TElementSelector, class TResultSelector>
    auto GroupBy(
        TKeySelector keySelector,
        TElementSelector elementSelector,
        TResultSelector resultSelector) const & -> Enumerable<std::invoke_result_t<TResultSelector, std::invoke_result_t<TKeySelector, reference>, Enumerable<std::invoke_result_t<TElementSelector, reference>>>>;

    //! Groups the elements of a sequence according to a specified key selector function and projects the elements for each group by using a specified function.
    //!
    //! @tparam TKey The type of the key returned by keySelector. Return type of TKeySelector.
    //! @tparam TElement The type of the elements in each Grouping<TKey,TElement>. Return type of TElementSelector.
    //!
    //! @tparam TKeySelector function<TKey(const T&)>.
    //! @tparam TElementSelector function<TElement(const T&)>.
    //!
    //! @param keySelector A function to extract the key for each element.
    //! @param elementSelector A function to map each source element to an element in an Grouping<TKey,TElement>.
    //!
    //! @returns An Enumerable<Grouping<TKey, TElement>> where each Grouping<TKey,TElement> object contains a collection of objects of type TElement and a key.
    //!
    //! @see https://docs.microsoft.com/en-us/dotnet/api/system.linq.enumerable.groupby?view=net-5.0#System_Linq_Enumerable_GroupBy__3_System_Collections_Generic_IEnumerable___0__System_Func___0___1__System_Func___0___2__
    template<class TKeySelector, class TElementSelector>
    auto GroupBy(
        TKeySelector keySelector,
        TElementSelector elementSelector) && -> Enumerable<Grouping<std::invoke_result_t<TKeySelector, reference>, std::invoke_result_t<TElementSelector, reference>>>;

    template<class TKeySelector, class TElementSelector>
    auto GroupBy(
        TKeySelector keySelector,
        TElementSelector elementSelector) const & -> Enumerable<Grouping<std::invoke_result_t<TKeySelector, reference>, std::invoke_result_t<TElementSelector, reference>>>;

    template<class THash, class TEnumerable, class TOuterKeySelector, class TInnerKeySelector, class TResultSelector>
    auto GroupJoinHash(
        const TEnumerable& inner,
        TOuterKeySelector outerKeySelector,
        TInnerKeySelector innerKeySelector,
        TResultSelector resultSelector) && -> Enumerable<std::invoke_result_t<TResultSelector, reference, const Enumerable<std::decay_t<decltype(*std::begin(inner))>>&>>;

    template<class THash, class U, class TOuterKeySelector, class TInnerKeySelector, class TResultSelector>
    auto GroupJoinHash(
        std::initializer_list<U> inner,
        TOuterKeySelector outerKeySelector,
        TInnerKeySelector innerKeySelector,
        TResultSelector resultSelector) && -> Enumerable<std::invoke_result_t<TResultSelector, reference, const Enumerable<U>&>>;

    template<class THash, class TEnumerable, class TOuterKeySelector, class TInnerKeySelector, class TResultSelector>
    auto GroupJoinHash(
        const TEnumerable& inner,
        TOuterKeySelector outerKeySelector,
        TInnerKeySelector innerKeySelector,
        TResultSelector resultSelector) const & -> Enumerable<std::invoke_result_t<TResultSelector, reference, const Enumerable<std::decay_t<decltype(*std::begin(inner))>>&>>;

    template<class THash, class U, class TOuterKeySelector, class TInnerKeySelector, class TResultSelector>
    auto GroupJoinHash(
        std::initializer_list<U> inner,
        TOuterKeySelector outerKeySelector,
        TInnerKeySelector innerKeySelector,
        TResultSelector resultSelector) const & -> Enumerable<std::invoke_result_t<TResultSelector, reference, const Enumerable<U>&>>;

    template<class TLess, class TEnumerable, class TOuterKeySelector, class TInnerKeySelector, class TResultSelector>
    auto GroupJoinLess(
        const TEnumerable& inner,
        TOuterKeySelector outerKeySelector,
        TInnerKeySelector innerKeySelector,
        TResultSelector resultSelector) && -> Enumerable<std::invoke_result_t<TResultSelector, reference, const Enumerable<std::decay_t<decltype(*std::begin(inner))>>&>>;

    template<class TLess, class U, class TOuterKeySelector, class TInnerKeySelector, class TResultSelector>
    auto GroupJoinLess(
        std::initializer_list<U> inner,
        TOuterKeySelector outerKeySelector,
        TInnerKeySelector innerKeySelector,
        TResultSelector resultSelector) && -> Enumerable<std::invoke_result_t<TResultSelector, reference, const Enumerable<U>&>>;

    template<class TLess, class TEnumerable, class TOuterKeySelector, class TInnerKeySelector, class TResultSelector>
    auto GroupJoinLess(
        const TEnumerable& inner,
        TOuterKeySelector outerKeySelector,
        TInnerKeySelector innerKeySelector,
        TResultSelector resultSelector) const & -> Enumerable<std::invoke_result_t<TResultSelector, reference, const Enumerable<std::decay_t<decltype(*std::begin(inner))>>&>>;

    template<class TLess, class U, class TOuterKeySelector, class TInnerKeySelector, class TResultSelector>
    auto GroupJoinLess(
        std::initializer_list<U> inner,
        TOuterKeySelector outerKeySelector,
        TInnerKeySelector innerKeySelector,
        TResultSelector resultSelector) const & -> Enumerable<std::invoke_result_t<TResultSelector, reference, const Enumerable<U>&>>;

    template<class TEqual, class TEnumerable, class TOuterKeySelector, class TInnerKeySelector, class TResultSelector>
    auto GroupJoinEqual(
        const TEnumerable& inner,
        TOuterKeySelector outerKeySelector,
        TInnerKeySelector innerKeySelector,
        TResultSelector resultSelector) && -> Enumerable<std::invoke_result_t<TResultSelector, reference, const Enumerable<std::decay_t<decltype(*std::begin(inner))>>&>>;

    template<class TEqual, class U, class TOuterKeySelector, class TInnerKeySelector, class TResultSelector>
    auto GroupJoinEqual(
        std::initializer_list<U> inner,
        TOuterKeySelector outerKeySelector,
        TInnerKeySelector innerKeySelector,
        TResultSelector resultSelector) && -> Enumerable<std::invoke_result_t<TResultSelector, reference, const Enumerable<U>&>>;

    template<class TEqual, class TEnumerable, class TOuterKeySelector, class TInnerKeySelector, class TResultSelector>
    auto GroupJoinEqual(
        const TEnumerable& inner,
        TOuterKeySelector outerKeySelector,
        TInnerKeySelector innerKeySelector,
        TResultSelector resultSelector) const & -> Enumerable<std::invoke_result_t<TResultSelector, reference, const Enumerable<std::decay_t<decltype(*std::begin(inner))>>&>>;

    template<class TEqual, class U, class TOuterKeySelector, class TInnerKeySelector, class TResultSelector>
    auto GroupJoinEqual(
        std::initializer_list<U> inner,
        TOuterKeySelector outerKeySelector,
        TInnerKeySelector innerKeySelector,
        TResultSelector resultSelector) const & -> Enumerable<std::invoke_result_t<TResultSelector, reference, const Enumerable<U>&>>;

    //! Correlates the elements of two sequences based on equality of keys and groups the results. The default equality comparer is used to compare keys.
    //!
    //! @tparam TInner The type of the elements of the second sequence. Value type of TEnumerable.
    //! @tparam TKey The type of the keys returned by the key selector functions. Return type of TOuterKeySelector and TInnerKeySelector.
    //! @tparam TResult The type of the result elements. Return type of TResultSelector.
    //!
    //! @tparam TEnumerable The type of the sequence to join to the first sequence.
    //! @tparam TOuterKeySelector function<TKey(const T&)>.
    //! @tparam TInnerKeySelector function<TKey(const TInner&)>.
    //! @tparam TResultSelector function<TResult(const TKey&, const Enumerable<TInner>&)>.
    //!
    //! @param inner The sequence to join to the first sequence.
    //! @param outerKeySelector A function to extract the join key from each element of the first sequence.
    //! @param innerKeySelector A function to extract the join key from each element of the second sequence.
    //! @param resultSelector A function to create a result element from an element from the first sequence and a collection of matching elements from the second sequence.
    //!
    //! @returns An Enumerable<T> that contains elements of type TResult that are obtained by performing a grouped join on two sequences.
    //!
    //! @see https://docs.microsoft.com/en-us/dotnet/api/system.linq.enumerable.groupjoin?view=net-5.0#System_Linq_Enumerable_GroupJoin__4_System_Collections_Generic_IEnumerable___0__System_Collections_Generic_IEnumerable___1__System_Func___0___2__System_Func___1___2__System_Func___0_System_Collections_Generic_IEnumerable___1____3__
    template<class TEnumerable, class TOuterKeySelector, class TInnerKeySelector, class TResultSelector>
    auto GroupJoin(
        const TEnumerable& inner,
        TOuterKeySelector outerKeySelector,
        TInnerKeySelector innerKeySelector,
        TResultSelector resultSelector) && -> Enumerable<std::invoke_result_t<TResultSelector, reference, const Enumerable<std::decay_t<decltype(*std::begin(inner))>>&>>;

    template<class U, class TOuterKeySelector, class TInnerKeySelector, class TResultSelector>
    auto GroupJoin(
        std::initializer_list<U> inner,
        TOuterKeySelector outerKeySelector,
        TInnerKeySelector innerKeySelector,
        TResultSelector resultSelector) && -> Enumerable<std::invoke_result_t<TResultSelector, reference, const Enumerable<U>&>>;

    template<class TEnumerable, class TOuterKeySelector, class TInnerKeySelector, class TResultSelector>
    auto GroupJoin(
        const TEnumerable& inner,
        TOuterKeySelector outerKeySelector,
        TInnerKeySelector innerKeySelector,
        TResultSelector resultSelector) const & -> Enumerable<std::invoke_result_t<TResultSelector, reference, const Enumerable<std::decay_t<decltype(*std::begin(inner))>>&>>;

    template<class U, class TOuterKeySelector, class TInnerKeySelector, class TResultSelector>
    auto GroupJoin(
        std::initializer_list<U> inner,
        TOuterKeySelector outerKeySelector,
        TInnerKeySelector innerKeySelector,
        TResultSelector resultSelector) const & -> Enumerable<std::invoke_result_t<TResultSelector, reference, const Enumerable<U>&>>;

    template<class THash>
    Enumerable IntersectHash(const Enumerable& other) &&;

    template<class THash>
    Enumerable IntersectHash(const Enumerable& other) const &;

    template<class TLess>
    Enumerable IntersectLess(const Enumerable& other) &&;

    template<class TLess>
    Enumerable IntersectLess(const Enumerable& other) const &;

    template<class TEqual>
    Enumerable IntersectEqual(const Enumerable& other) &&;

    template<class TEqual>
    Enumerable IntersectEqual(const Enumerable& other) const &;

    //! Produces the set intersection of two sequences by using the default equality comparer to compare values.
    //!
    //! @param other An Enumerable<T> whose distinct elements that also appear in the first sequence will be returned.
    //!
    //! @returns A sequence that contains the elements that form the set intersection of two sequences.
    //!
    //! @see https://docs.microsoft.com/en-us/dotnet/api/system.linq.enumerable.intersect?view=net-5.0#System_Linq_Enumerable_Intersect__1_System_Collections_Generic_IEnumerable___0__System_Collections_Generic_IEnumerable___0__
    Enumerable Intersect(const Enumerable& other) &&;

    Enumerable Intersect(const Enumerable& other) const &;

    template<class THash, class TEnumerable, class TOuterKeySelector, class TInnerKeySelector, class TResultSelector>
    auto JoinHash(
        const TEnumerable& inner,
        TOuterKeySelector outerKeySelector,
        TInnerKeySelector innerKeySelector,
        TResultSelector resultSelector) && -> Enumerable<std::invoke_result_t<TResultSelector, reference, decltype(*std::begin(inner))>>;

    template<class THash, class U, class TOuterKeySelector, class TInnerKeySelector, class TResultSelector>
    auto JoinHash(
        std::initializer_list<U> inner,
        TOuterKeySelector outerKeySelector,
        TInnerKeySelector innerKeySelector,
        TResultSelector resultSelector) && -> Enumerable<std::invoke_result_t<TResultSelector, reference, const U&>>;

    template<class THash, class TEnumerable, class TOuterKeySelector, class TInnerKeySelector, class TResultSelector>
    auto JoinHash(
        const TEnumerable& inner,
        TOuterKeySelector outerKeySelector,
        TInnerKeySelector innerKeySelector,
        TResultSelector resultSelector) const & -> Enumerable<std::invoke_result_t<TResultSelector, reference, decltype(*std::begin(inner))>>;

    template<class THash, class U, class TOuterKeySelector, class TInnerKeySelector, class TResultSelector>
    auto JoinHash(
        std::initializer_list<U> inner,
        TOuterKeySelector outerKeySelector,
        TInnerKeySelector innerKeySelector,
        TResultSelector resultSelector) const & -> Enumerable<std::invoke_result_t<TResultSelector, reference, const U&>>;

    template<class TLess, class TEnumerable, class TOuterKeySelector, class TInnerKeySelector, class TResultSelector>
    auto JoinLess(
        const TEnumerable& inner,
        TOuterKeySelector outerKeySelector,
        TInnerKeySelector innerKeySelector,
        TResultSelector resultSelector) && -> Enumerable<std::invoke_result_t<TResultSelector, reference, decltype(*std::begin(inner))>>;

    template<class TLess, class U, class TOuterKeySelector, class TInnerKeySelector, class TResultSelector>
    auto JoinLess(
        std::initializer_list<U> inner,
        TOuterKeySelector outerKeySelector,
        TInnerKeySelector innerKeySelector,
        TResultSelector resultSelector) && -> Enumerable<std::invoke_result_t<TResultSelector, reference, const U&>>;

    template<class TLess, class TEnumerable, class TOuterKeySelector, class TInnerKeySelector, class TResultSelector>
    auto JoinLess(
        const TEnumerable& inner,
        TOuterKeySelector outerKeySelector,
        TInnerKeySelector innerKeySelector,
        TResultSelector resultSelector) const & -> Enumerable<std::invoke_result_t<TResultSelector, reference, decltype(*std::begin(inner))>>;

    template<class TLess, class U, class TOuterKeySelector, class TInnerKeySelector, class TResultSelector>
    auto JoinLess(
        std::initializer_list<U> inner,
        TOuterKeySelector outerKeySelector,
        TInnerKeySelector innerKeySelector,
        TResultSelector resultSelector) const & -> Enumerable<std::invoke_result_t<TResultSelector, reference, const U&>>;

    template<class TEqual, class TEnumerable, class TOuterKeySelector, class TInnerKeySelector, class TResultSelector>
    auto JoinEqual(
        const TEnumerable& inner,
        TOuterKeySelector outerKeySelector,
        TInnerKeySelector innerKeySelector,
        TResultSelector resultSelector) && -> Enumerable<std::invoke_result_t<TResultSelector, reference, decltype(*std::begin(inner))>>;

    template<class TEqual, class U, class TOuterKeySelector, class TInnerKeySelector, class TResultSelector>
    auto JoinEqual(
        std::initializer_list<U> inner,
        TOuterKeySelector outerKeySelector,
        TInnerKeySelector innerKeySelector,
        TResultSelector resultSelector) && -> Enumerable<std::invoke_result_t<TResultSelector, reference, const U&>>;

    template<class TEqual, class TEnumerable, class TOuterKeySelector, class TInnerKeySelector, class TResultSelector>
    auto JoinEqual(
        const TEnumerable& inner,
        TOuterKeySelector outerKeySelector,
        TInnerKeySelector innerKeySelector,
        TResultSelector resultSelector) const & -> Enumerable<std::invoke_result_t<TResultSelector, reference, decltype(*std::begin(inner))>>;

    template<class TEqual, class U, class TOuterKeySelector, class TInnerKeySelector, class TResultSelector>
    auto JoinEqual(
        std::initializer_list<U> inner,
        TOuterKeySelector outerKeySelector,
        TInnerKeySelector innerKeySelector,
        TResultSelector resultSelector) const & -> Enumerable<std::invoke_result_t<TResultSelector, reference, const U&>>;

    //! Correlates the elements of two sequences based on matching keys. The default equality comparer is used to compare keys.
    //!
    //! @tparam TInner The type of the elements of the second sequence. Value type of TEnumerable.
    //! @tparam TKey The type of the keys returned by the key selector functions. Return type of TOuterKeySelector and TInnerKeySelector.
    //! @tparam TResult The type of the result elements. Return type of TResultSelector.
    //!
    //! @tparam TEnumerable The type of the sequence to join to the first sequence.
    //! @tparam TOuterKeySelector function<TKey(const T&)>.
    //! @tparam TInnerKeySelector function<TKey(const TInner&)>.
    //! @tparam TResultSelector function<TResult(const T&, const TInner&)>.
    //!
    //! @param inner The sequence to join to the first sequence.
    //! @param outerKeySelector A function to extract the join key from each element of the first sequence.
    //! @param innerKeySelector A function to extract the join key from each element of the second sequence.
    //! @param resultSelector A function to create a result element from two matching elements.
    //!
    //! @returns An Enumerable<T> that has elements of type TResult that are obtained by performing an inner join on two sequences.
    //!
    //! @see https://docs.microsoft.com/en-us/dotnet/api/system.linq.enumerable.join?view=net-5.0#System_Linq_Enumerable_Join__4_System_Collections_Generic_IEnumerable___0__System_Collections_Generic_IEnumerable___1__System_Func___0___2__System_Func___1___2__System_Func___0___1___3__
    template<class TEnumerable, class TOuterKeySelector, class TInnerKeySelector, class TResultSelector>
    auto Join(
        const TEnumerable& inner,
        TOuterKeySelector outerKeySelector,
        TInnerKeySelector innerKeySelector,
        TResultSelector resultSelector) && -> Enumerable<std::invoke_result_t<TResultSelector, reference, decltype(*std::begin(inner))>>;

    template<class U, class TOuterKeySelector, class TInnerKeySelector, class TResultSelector>
    auto Join(
        std::initializer_list<U> inner,
        TOuterKeySelector outerKeySelector,
        TInnerKeySelector innerKeySelector,
        TResultSelector resultSelector) && -> Enumerable<std::invoke_result_t<TResultSelector, reference, const U&>>;

    template<class TEnumerable, class TOuterKeySelector, class TInnerKeySelector, class TResultSelector>
    auto Join(
        const TEnumerable& inner,
        TOuterKeySelector outerKeySelector,
        TInnerKeySelector innerKeySelector,
        TResultSelector resultSelector) const & -> Enumerable<std::invoke_result_t<TResultSelector, reference, decltype(*std::begin(inner))>>;

    template<class U, class TOuterKeySelector, class TInnerKeySelector, class TResultSelector>
    auto Join(
        std::initializer_list<U> inner,
        TOuterKeySelector outerKeySelector,
        TInnerKeySelector innerKeySelector,
        TResultSelector resultSelector) const & -> Enumerable<std::invoke_result_t<TResultSelector, reference, const U&>>;

    //! Returns the last element of a sequence that satisfies a specified condition.
    //!
    //! @tparam TPredicate function<bool(const T&)>.
    //!
    //! @param predicate A function to test each element for a condition.
    //! @param defaultValue The value to return if the sequence is empty or if no elements pass the test in the predicate function.
    //!
    //! @returns defaultValue if the sequence is empty or if no elements pass the test in the predicate function; otherwise, the last element that passes the test in the predicate function.
    //!
    //! @see https://docs.microsoft.com/en-us/dotnet/api/system.linq.enumerable.lastordefault?view=net-5.0#System_Linq_Enumerable_LastOrDefault__1_System_Collections_Generic_IEnumerable___0__System_Func___0_System_Boolean__
    template<class TPredicate>
    value_type Last(TPredicate predicate, value_type defaultValue) &&;

    template<class TPredicate>
    value_type Last(TPredicate predicate, value_type defaultValue) const &;

    //! Returns the last element of a sequence, or a default value if the sequence contains no elements.
    //!
    //! @param defaultValue The value to return if the sequence is empty.
    //!
    //! @returns defaultValue if the source sequence is empty; otherwise, the last element in the IEnumerable<T>.
    //!
    //! @see https://docs.microsoft.com/en-us/dotnet/api/system.linq.enumerable.lastordefault?view=net-5.0#System_Linq_Enumerable_LastOrDefault__1_System_Collections_Generic_IEnumerable___0__
    value_type Last(value_type defaultValue) &&;

    value_type Last(value_type defaultValue) const &;

    //! Sorts the elements of a sequence by using a specified comparer.
    //!
    //! @tparam TKey The type of the key returned by keySelector. Return type of TKeySelector.
    //!
    //! @tparam TKeySelector function<TKey(const T&)>.
    //! @tparam TComparer function<bool(const TKey&, const TKey&)>.
    //!
    //! @param keySelector A function to extract a key from an element.
    //! @param comparer A function to compare keys.
    //!
    //! @returns An Enumerable<T> whose elements are sorted according to a key.
    //!
    //! @see https://docs.microsoft.com/en-us/dotnet/api/system.linq.enumerable.orderby?view=net-5.0#System_Linq_Enumerable_OrderBy__2_System_Collections_Generic_IEnumerable___0__System_Func___0___1__System_Collections_Generic_IComparer___1__
    template<class TKeySelector, class TComparer>
    Enumerable OrderBy(TKeySelector keySelector, TComparer comparer) &&;

    template<class TKeySelector, class TComparer>
    Enumerable OrderBy(TKeySelector keySelector, TComparer comparer) const &;

    //! Sorts the elements of a sequence in ascending order according to a key.
    //!
    //! @tparam TKey The type of the key returned by keySelector. Return type of TKeySelector.
    //!
    //! @tparam TKeySelector function<TKey(const T&)>.
    //!
    //! @param keySelector A function to extract a key from an element.
    //!
    //! @returns An Enumerable<T> whose elements are sorted in ascending order according to a key.
    //!
    //! @see https://docs.microsoft.com/en-us/dotnet/api/system.linq.enumerable.orderby?view=net-5.0#System_Linq_Enumerable_OrderBy__2_System_Collections_Generic_IEnumerable___0__System_Func___0___1__
    template<class TKeySelector>
    Enumerable OrderBy(TKeySelector keySelector) &&;

    template<class TKeySelector>
    Enumerable OrderBy(TKeySelector keySelector) const &;

    //! Sorts the elements of a sequence in ascending order.
    //!
    //! @returns An Enumerable<T> whose elements are sorted in ascending order.
    Enumerable OrderBy() &&;

    Enumerable OrderBy() const &;

    //! Sorts the elements of a sequence in descending order according to a key.
    //!
    //! @tparam TKey The type of the key returned by keySelector. Return type of TKeySelector.
    //!
    //! @tparam TKeySelector function<TKey(const T&)>.
    //!
    //! @param keySelector A function to extract a key from an element.
    //!
    //! @returns An Enumerable<T> whose elements are sorted in descending order according to a key.
    //!
    //! @see https://docs.microsoft.com/en-us/dotnet/api/system.linq.enumerable.orderbydescending?view=net-5.0#System_Linq_Enumerable_OrderByDescending__2_System_Collections_Generic_IEnumerable___0__System_Func___0___1__
    template<class TKeySelector>
    Enumerable OrderByDescending(TKeySelector keySelector) &&;

    template<class TKeySelector>
    Enumerable OrderByDescending(TKeySelector keySelector) const &;

    //! Sorts the elements of a sequence in descending order.
    //!
    //! @returns An Enumerable<T> whose elements are sorted in descending order.
    Enumerable OrderByDescending() &&;

    Enumerable OrderByDescending() const &;

    //! Adds a value to the beginning of the sequence.
    //!
    //! @param The value to prepend to source.
    //!
    //! @returns A new sequence that begins with element.
    //!
    //! @see https://docs.microsoft.com/en-us/dotnet/api/system.linq.enumerable.prepend?view=net-5.0
    Enumerable Prepend(value_type element) &&;

    Enumerable Prepend(value_type element) const &;

    //! Inverts the order of the elements in a sequence.
    //!
    //! @returns A sequence whose elements correspond to those of the input sequence in reverse order.
    //!
    //! @see https://docs.microsoft.com/en-us/dotnet/api/system.linq.enumerable.reverse?view=net-5.0
    Enumerable Reverse() &&;

    Enumerable Reverse() const &;

    //! Projects each element of a sequence into a new form.
    //!
    //! @tparam TResult The type of the value returned by selector. Return type of TSelector.
    //!
    //! @tparam TSelector function<TResult(const T&)>.
    //!
    //! @param selector A transform function to apply to each element.
    //!
    //! @returns An Enumerable<T> whose elements are the result of invoking the transform function on each element of source.
    //!
    //! @see https://docs.microsoft.com/en-us/dotnet/api/system.linq.enumerable.select?view=net-5.0#System_Linq_Enumerable_Select__2_System_Collections_Generic_IEnumerable___0__System_Func___0___1__
    template<class TSelector>
    auto Select(TSelector selector) && -> Enumerable<std::invoke_result_t<TSelector, reference>>;

    template<class TSelector>
    auto Select(TSelector selector) const & -> Enumerable<std::invoke_result_t<TSelector, reference>>;

    //! Projects each element of a sequence into a new form by incorporating the element's index.
    //!
    //! @tparam TResult The type of the value returned by selector. Return type of TSelector.
    //!
    //! @tparam TSelector function<TResult(const T&, int)>.
    //!
    //! @param selector A transform function to apply to each source element; the second parameter of the function represents the index of the source element.
    //!
    //! @returns An Enumerable<T> whose elements are the result of invoking the transform function on each element of source.
    //!
    //! @see https://docs.microsoft.com/en-us/dotnet/api/system.linq.enumerable.select?view=net-5.0#System_Linq_Enumerable_Select__2_System_Collections_Generic_IEnumerable___0__System_Func___0_System_Int32___1__
    template<class TSelector>
    auto SelectWithIndex(TSelector selector) && -> Enumerable<std::invoke_result_t<TSelector, reference, int>>;

    template<class TSelector>
    auto SelectWithIndex(TSelector selector) const & -> Enumerable<std::invoke_result_t<TSelector, reference, int>>;

    //! Projects each element of a sequence to an Enumerable<T>, flattens the resulting sequences into one sequence, and invokes a result selector function on each element therein.
    //!
    //! @tparam TCollection The type of the intermediate elements collected by collectionSelector. Value type of return type of TCollectionSelector.
    //! @tparam TResult The type of the elements of the resulting sequence. Return type of TResultSelector.
    //!
    //! @tparam TCollectionSelector function<Enumerable<TCollection>(const T&)>.
    //! @tparam TResultSelector function<TResult(const T&, const TCollection&)>.
    //!
    //! @param collectionSelector A transform function to apply to each element of the input sequence.
    //! @param resultSelector A transform function to apply to each element of the intermediate sequence.
    //!
    //! @returns An Enumerable<T> whose elements are the result of invoking the one-to-many transform function collectionSelector on each element of source and then mapping each of those sequence elements and their corresponding source element to a result element.
    //!
    //! @see https://docs.microsoft.com/en-us/dotnet/api/system.linq.enumerable.selectmany?view=net-5.0#System_Linq_Enumerable_SelectMany__3_System_Collections_Generic_IEnumerable___0__System_Func___0_System_Collections_Generic_IEnumerable___1___System_Func___0___1___2__
    template<class TCollectionSelector, class TResultSelector>
    auto SelectMany(TCollectionSelector collectionSelector, TResultSelector resultSelector) &&
        -> Enumerable<std::invoke_result_t<TResultSelector, reference, std::decay_t<decltype(*std::begin(std::declval<std::invoke_result_t<TCollectionSelector, reference>>()))>>>;

    template<class TCollectionSelector, class TResultSelector>
    auto SelectMany(TCollectionSelector collectionSelector, TResultSelector resultSelector) const &
        -> Enumerable<std::invoke_result_t<TResultSelector, reference, std::decay_t<decltype(*std::begin(std::declval<std::invoke_result_t<TCollectionSelector, reference>>()))>>>;

    //! Projects each element of a sequence to an Enumerable<T> and flattens the resulting sequences into one sequence.
    //!
    //! @tparam TResult The type of the elements of the sequence returned by collectionSelector. Value type of return type of TCollectionSelector.
    //!
    //! @tparam TCollectionSelector function<Enumerable<TResult>(const T&)>.
    //!
    //! @param collectionSelector A transform function to apply to each element.
    //!
    //! @returns An Enumerable<T> whose elements are the result of invoking the one-to-many transform function on each element of the input sequence.
    //!
    //! @see https://docs.microsoft.com/en-us/dotnet/api/system.linq.enumerable.selectmany?view=net-5.0#System_Linq_Enumerable_SelectMany__2_System_Collections_Generic_IEnumerable___0__System_Func___0_System_Collections_Generic_IEnumerable___1___
    template<class TCollectionSelector>
    auto SelectMany(TCollectionSelector collectionSelector) &&
        -> Enumerable<std::decay_t<decltype(*std::begin(std::declval<std::invoke_result_t<TCollectionSelector, reference>>()))>>;

    template<class TCollectionSelector>
    auto SelectMany(TCollectionSelector collectionSelector) const &
        -> Enumerable<std::decay_t<decltype(*std::begin(std::declval<std::invoke_result_t<TCollectionSelector, reference>>()))>>;

    //! Projects each element of a sequence to an Enumerable<T>, flattens the resulting sequences into one sequence, and invokes a result selector function on each element therein. The index of each source element is used in the intermediate projected form of that element.
    //!
    //! @tparam TCollection The type of the intermediate elements collected by collectionSelector. Value type of return type of TCollectionSelector.
    //! @tparam TResult The type of the elements of the resulting sequence. Return type of TResultSelector.
    //!
    //! @tparam TCollectionSelector function<Enumerable<TCollection>(const T&, int)>.
    //! @tparam TResultSelector function<TResult(const T&, const TCollection&)>.
    //!
    //! @param collectionSelector A transform function to apply to each source element; the second parameter of the function represents the index of the source element.
    //! @param resultSelector A transform function to apply to each element of the intermediate sequence.
    //!
    //! @returns An Enumerable<T> whose elements are the result of invoking the one-to-many transform function collectionSelector on each element of source and then mapping each of those sequence elements and their corresponding source element to a result element.
    //!
    //! @see https://docs.microsoft.com/en-us/dotnet/api/system.linq.enumerable.selectmany?view=net-5.0#System_Linq_Enumerable_SelectMany__3_System_Collections_Generic_IEnumerable___0__System_Func___0_System_Int32_System_Collections_Generic_IEnumerable___1___System_Func___0___1___2__
    template<class TCollectionSelector, class TResultSelector>
    auto SelectManyWithIndex(TCollectionSelector collectionSelector, TResultSelector resultSelector) &&
        -> Enumerable<std::invoke_result_t<TResultSelector, reference, std::decay_t<decltype(*std::begin(std::declval<std::invoke_result_t<TCollectionSelector, reference, int>>()))>>>;

    template<class TCollectionSelector, class TResultSelector>
    auto SelectManyWithIndex(TCollectionSelector collectionSelector, TResultSelector resultSelector) const &
        -> Enumerable<std::invoke_result_t<TResultSelector, reference, std::decay_t<decltype(*std::begin(std::declval<std::invoke_result_t<TCollectionSelector, reference, int>>()))>>>;

    //! Projects each element of a sequence to an Enumerable<T>, and flattens the resulting sequences into one sequence. The index of each source element is used in the projected form of that element.
    //!
    //! @tparam TResult The type of the elements of the sequence returned by collectionSelector. Value type of return type of TCollectionSelector.
    //!
    //! @tparam TCollectionSelector function<Enumerable<TResult>(const T&, int)>.
    //!
    //! @param collectionSelector A transform function to apply to each source element; the second parameter of the function represents the index of the source element.
    //!
    //! @returns An Enumerable<T> whose elements are the result of invoking the one-to-many transform function on each element of an input sequence.
    //!
    //! @see https://docs.microsoft.com/en-us/dotnet/api/system.linq.enumerable.selectmany?view=net-5.0#System_Linq_Enumerable_SelectMany__2_System_Collections_Generic_IEnumerable___0__System_Func___0_System_Int32_System_Collections_Generic_IEnumerable___1___
    template<class TCollectionSelector>
    auto SelectManyWithIndex(TCollectionSelector collectionSelector) &&
        -> Enumerable<std::decay_t<decltype(*std::begin(std::declval<std::invoke_result_t<TCollectionSelector, reference, int>>()))>>;

    template<class TCollectionSelector>
    auto SelectManyWithIndex(TCollectionSelector collectionSelector) const &
        -> Enumerable<std::decay_t<decltype(*std::begin(std::declval<std::invoke_result_t<TCollectionSelector, reference, int>>()))>>;

    //! Determines whether two sequences are equal by comparing their elements by using a specified TEqual.
    //!
    //! @tparam TEqual function<bool(const T&, const T&)>.
    //!
    //! @param other An Enumerable<T> to compare to the first sequence.
    //! @param comparer An TEqual to use to compare elements.
    //!
    //! @returns true if the two source sequences are of equal length and their corresponding elements compare equal according to comparer; otherwise, false.
    //!
    //! @see https://docs.microsoft.com/en-us/dotnet/api/system.linq.enumerable.sequenceequal?view=net-5.0#System_Linq_Enumerable_SequenceEqual__1_System_Collections_Generic_IEnumerable___0__System_Collections_Generic_IEnumerable___0__System_Collections_Generic_IEqualityComparer___0__
    template<class TEqual>
    bool SequenceEqual(const Enumerable& other, TEqual comparer) &&;

    template<class TEqual>
    bool SequenceEqual(const Enumerable& other, TEqual comparer) const &;

    //! Determines whether two sequences are equal by comparing the elements by using the default equality comparer for their type.
    //!
    //! @param other An Enumerable<T> to compare to the first sequence.
    //!
    //! @returns true if the two source sequences are of equal length and their corresponding elements are equal according to the default equality comparer for their type; otherwise, false.
    //!
    //! @see https://docs.microsoft.com/en-us/dotnet/api/system.linq.enumerable.sequenceequal?view=net-5.0#System_Linq_Enumerable_SequenceEqual__1_System_Collections_Generic_IEnumerable___0__System_Collections_Generic_IEnumerable___0__
    bool SequenceEqual(const Enumerable& other) &&;

    bool SequenceEqual(const Enumerable& other) const &;

    //! Returns the only element of a sequence that satisfies a specified condition or a default value if no such element exists; this method throws an exception if more than one element satisfies the condition.
    //!
    //! @tparam TPredicate function<bool(const T&)>.
    //!
    //! @param defaultValue The value to return if no such element is found.
    //! @param predicate A function to test an element for a condition.
    //!
    //! @returns The single element of the input sequence that satisfies the condition, or defaultValue if no such element is found.
    //!
    //! @see https://docs.microsoft.com/en-us/dotnet/api/system.linq.enumerable.singleordefault?view=net-5.0#System_Linq_Enumerable_SingleOrDefault__1_System_Collections_Generic_IEnumerable___0__System_Func___0_System_Boolean__
    template<class TPredicate>
    value_type Single(value_type defaultValue, TPredicate predicate) &&;

    template<class TPredicate>
    value_type Single(value_type defaultValue, TPredicate predicate) const &;

    //! Returns the only element of a sequence, or a default value if the sequence is empty; this method throws an exception if there is more than one element in the sequence.
    //!
    //! @param defaultValue The value to return if the sequence contains no elements.
    //!
    //! @returns The single element of the input sequence, or defaultValue if the sequence contains no elements.
    //!
    //! @see https://docs.microsoft.com/en-us/dotnet/api/system.linq.enumerable.singleordefault?view=net-5.0#System_Linq_Enumerable_SingleOrDefault__1_System_Collections_Generic_IEnumerable___0__
    value_type Single(value_type defaultValue) &&;

    value_type Single(value_type defaultValue) const &;

    //! Bypasses a specified number of elements in a sequence and then returns the remaining elements.
    //!
    //! @param count The number of elements to skip before returning the remaining elements.
    //!
    //! @returns An Enumerable<T> that contains the elements that occur after the specified index in the input sequence.
    //!
    //! @see https://docs.microsoft.com/en-us/dotnet/api/system.linq.enumerable.skip?view=net-5.0
    Enumerable Skip(int count) &&;

    Enumerable Skip(int count) const &;

    //! Returns a new enumerable collection that contains the elements from source with the last count elements of the source collection omitted.
    //!
    //! @param count The number of elements to omit from the end of the collection.
    //!
    //! @returns A new enumerable collection that contains the elements from source minus count elements from the end of the collection.
    //!
    //! @see https://docs.microsoft.com/en-us/dotnet/api/system.linq.enumerable.skiplast?view=net-5.0
    Enumerable SkipLast(int count) &&;

    Enumerable SkipLast(int count) const &;

    //! Bypasses elements in a sequence as long as a specified condition is true and then returns the remaining elements.
    //!
    //! @tparam TPredicate function<bool(const T&)>.
    //!
    //! @param predicate A function to test each element for a condition.
    //!
    //! @returns An Enumerable<T> that contains the elements from the input sequence starting at the first element in the linear series that does not pass the test specified by predicate.
    //!
    //! @see https://docs.microsoft.com/en-us/dotnet/api/system.linq.enumerable.skipwhile?view=net-5.0#System_Linq_Enumerable_SkipWhile__1_System_Collections_Generic_IEnumerable___0__System_Func___0_System_Boolean__
    template<class TPredicate>
    Enumerable SkipWhile(TPredicate predicate) &&;

    template<class TPredicate>
    Enumerable SkipWhile(TPredicate predicate) const &;

    //! Bypasses elements in a sequence as long as a specified condition is true and then returns the remaining elements. The element's index is used in the logic of the predicate function.
    //!
    //! @tparam TPredicate function<bool(const T&, int)>.
    //!
    //! @param predicate A function to test each source element for a condition; the second parameter of the function represents the index of the source element.
    //!
    //! @returns An Enumerable<T> that contains the elements from the input sequence starting at the first element in the linear series that does not pass the test specified by predicate.
    //!
    //! @see https://docs.microsoft.com/en-us/dotnet/api/system.linq.enumerable.skipwhile?view=net-5.0#System_Linq_Enumerable_SkipWhile__1_System_Collections_Generic_IEnumerable___0__System_Func___0_System_Int32_System_Boolean__
    template<class TPredicate>
    Enumerable SkipWhileWithIndex(TPredicate predicate) &&;

    template<class TPredicate>
    Enumerable SkipWhileWithIndex(TPredicate predicate) const &;

    //! Returns a specified number of contiguous elements from the start of a sequence.
    //!
    //! @param count The number of elements to return.
    //!
    //! @returns An Enumerable<T> that contains the specified number of elements from the start of the input sequence.
    //!
    //! @see https://docs.microsoft.com/en-us/dotnet/api/system.linq.enumerable.take?view=net-5.0#System_Linq_Enumerable_Take__1_System_Collections_Generic_IEnumerable___0__System_Int32_
    Enumerable Take(int count) &&;

    Enumerable Take(int count) const &;

    //! Returns a new enumerable collection that contains the last count elements from source.
    //!
    //! @param count The number of elements to take from the end of the collection.
    //!
    //! @returns A new enumerable collection that contains the last count elements from source.
    //!
    //! @see https://docs.microsoft.com/en-us/dotnet/api/system.linq.enumerable.takelast?view=net-5.0
    Enumerable TakeLast(int count) &&;

    Enumerable TakeLast(int count) const &;

    //! Returns elements from a sequence as long as a specified condition is true.
    //!
    //! @tparam TPredicate function<bool(const T&)>.
    //!
    //! @param predicate A function to test each element for a condition.
    //!
    //! @returns An Enumerable<T> that contains the elements from the input sequence that occur before the element at which the test no longer passes.
    //!
    //! @see https://docs.microsoft.com/en-us/dotnet/api/system.linq.enumerable.takewhile?view=net-5.0#System_Linq_Enumerable_TakeWhile__1_System_Collections_Generic_IEnumerable___0__System_Func___0_System_Boolean__
    template<class TPredicate>
    Enumerable TakeWhile(TPredicate predicate) &&;

    template<class TPredicate>
    Enumerable TakeWhile(TPredicate predicate) const &;

    //! Returns elements from a sequence as long as a specified condition is true. The element's index is used in the logic of the predicate function.
    //!
    //! @tparam TPredicate function<bool(const T&, int)>.
    //!
    //! @param predicate A function to test each source element for a condition; the second parameter of the function represents the index of the source element.
    //!
    //! @returns An Enumerable<T> that contains elements from the input sequence that occur before the element at which the test no longer passes.
    //!
    //! @see https://docs.microsoft.com/en-us/dotnet/api/system.linq.enumerable.takewhile?view=net-5.0#System_Linq_Enumerable_TakeWhile__1_System_Collections_Generic_IEnumerable___0__System_Func___0_System_Int32_System_Boolean__
    template<class TPredicate>
    Enumerable TakeWhileWithIndex(TPredicate predicate) &&;

    template<class TPredicate>
    Enumerable TakeWhileWithIndex(TPredicate predicate) const &;

    //! Creates a container from a Enumerable<T>.
    //!
    //! @tparam TContainer A class template that takes at least one template parameter as value type.
    //!
    //! @returns A TContainer<T, ...> that contains elements from this Enumerable<T>.
    template<template<class, class...> class TContainer, class... U>
    auto ToContainer() && -> TContainer<value_type, U...>;

    template<template<class, class...> class TContainer, class... U>
    auto ToContainer() const & -> TContainer<value_type, U...>;

    //! Creates a vector from a Enumerable<T>.
    //!
    //! @returns A std::vector<T> that contains elements from this Enumerable<T>.
    Container ToContainer() &&;

    Container ToContainer() const &;

    template<class THash>
    Enumerable UnionHash(const Enumerable& other) &&;

    template<class THash>
    Enumerable UnionHash(const Enumerable& other) const &;

    template<class TLess>
    Enumerable UnionLess(const Enumerable& other) &&;

    template<class TLess>
    Enumerable UnionLess(const Enumerable& other) const &;

    template<class TEqual>
    Enumerable UnionEqual(const Enumerable& other) &&;

    template<class TEqual>
    Enumerable UnionEqual(const Enumerable& other) const &;

    //! Produces the set union of two sequences by using the default equality comparer.
    //!
    //! @param other An Enumerable<T> whose distinct elements form the second set for the union.
    //!
    //! @returns An Enumerable<T> that contains the elements from both input sequences, excluding duplicates.
    //!
    //! @see https://docs.microsoft.com/en-us/dotnet/api/system.linq.enumerable.union?view=net-5.0#System_Linq_Enumerable_Union__1_System_Collections_Generic_IEnumerable___0__System_Collections_Generic_IEnumerable___0__
    Enumerable Union(const Enumerable& other) &&;

    Enumerable Union(const Enumerable& other) const &;

    //! Filters a sequence of values based on a predicate.
    //!
    //! @tparam TPredicate function<bool(const T&)>.
    //!
    //! @param predicate A function to test each element for a condition.
    //!
    //! @returns An Enumerable<T> that contains elements from the input sequence that satisfy the condition.
    //!
    //! @see https://docs.microsoft.com/en-us/dotnet/api/system.linq.enumerable.where?view=net-5.0#System_Linq_Enumerable_Where__1_System_Collections_Generic_IEnumerable___0__System_Func___0_System_Boolean__
    template<class TPredicate>
    Enumerable Where(TPredicate predicate) &&;

    template<class TPredicate>
    Enumerable Where(TPredicate predicate) const &;

    //! Filters a sequence of values based on a predicate. Each element's index is used in the logic of the predicate function.
    //!
    //! @tparam TPredicate function<bool(const T&, int)>.
    //!
    //! @param predicate A function to test each source element for a condition; the second parameter of the function represents the index of the source element.
    //!
    //! @returns An Enumerable<T> that contains elements from the input sequence that satisfy the condition.
    //!
    //! @see https://docs.microsoft.com/en-us/dotnet/api/system.linq.enumerable.where?view=net-5.0#System_Linq_Enumerable_Where__1_System_Collections_Generic_IEnumerable___0__System_Func___0_System_Int32_System_Boolean__
    template<class TPredicate>
    Enumerable WhereWithIndex(TPredicate predicate) &&;

    template<class TPredicate>
    Enumerable WhereWithIndex(TPredicate predicate) const &;

    //! Applies a specified function to the corresponding elements of two sequences, producing a sequence of the results.
    //!
    //! @tparam TSecond The type of the elements of the second input sequence. Value type of TEnumerable.
    //! @tparam TResult The type of the elements of the result sequence. Return type of TResultSelector.
    //!
    //! @tparam TEnumerable The type of the second sequence to merge.
    //! @tparam TResultSelector function<TResultSelector(const T&, const TSecond&)>.
    //!
    //! @param other The second sequence to merge.
    //! @param resultSelector A function that specifies how to merge the elements from the two sequences.
    //!
    //! @returns An Enumerable<T> that contains merged elements of two input sequences.
    //!
    //! @see https://docs.microsoft.com/en-us/dotnet/api/system.linq.enumerable.zip?view=net-5.0#System_Linq_Enumerable_Zip__3_System_Collections_Generic_IEnumerable___0__System_Collections_Generic_IEnumerable___1__System_Func___0___1___2__
    template<class TEnumerable, class TResultSelector>
    auto Zip(const TEnumerable& other, TResultSelector resultSelector) && -> Enumerable<std::invoke_result_t<TResultSelector, reference, decltype(*std::begin(other))>>;

    template<class U, class TResultSelector>
    auto Zip(std::initializer_list<U> other, TResultSelector resultSelector) && ->Enumerable<std::invoke_result_t<TResultSelector, reference, decltype(*std::begin(other))>>;

    template<class TEnumerable, class TResultSelector>
    auto Zip(const TEnumerable& other, TResultSelector resultSelector) const & -> Enumerable<std::invoke_result_t<TResultSelector, reference, decltype(*std::begin(other))>>;

    template<class U, class TResultSelector>
    auto Zip(std::initializer_list<U> other, TResultSelector resultSelector) const & -> Enumerable<std::invoke_result_t<TResultSelector, reference, decltype(*std::begin(other))>>;

    //! Produces a sequence of pairs with elements from the two specified sequences.
    //!
    //! @tparam TSecond The type of the elements of the second input sequence. Value type of TEnumerable.
    //!
    //! @tparam TEnumerable The type of the second sequence to merge.
    //!
    //! @param other The second sequence to merge.
    //!
    //! @returns A sequence of pairs with elements taken from the first and second sequences, in that order.
    //!
    //! @see https://docs.microsoft.com/en-us/dotnet/api/system.linq.enumerable.zip?view=net-5.0#System_Linq_Enumerable_Zip__2_System_Collections_Generic_IEnumerable___0__System_Collections_Generic_IEnumerable___1__
    template<class TEnumerable>
    auto Zip(const TEnumerable& other) && -> Enumerable<std::pair<value_type, std::decay_t<decltype(*std::begin(other))>>>;

    template<class U>
    auto Zip(std::initializer_list<U> other) && -> Enumerable<std::pair<value_type, std::decay_t<decltype(*std::begin(other))>>>;

    template<class TEnumerable>
    auto Zip(const TEnumerable& other) const & -> Enumerable<std::pair<value_type, std::decay_t<decltype(*std::begin(other))>>>;

    template<class U>
    auto Zip(std::initializer_list<U> other) const & -> Enumerable<std::pair<value_type, std::decay_t<decltype(*std::begin(other))>>>;

#pragma endregion linq

#pragma region todo

    // AsEnumerable
    // Average
    // Cast
    // LongCount
    // Max
    // Min
    // OfType
    // Sum
    // ThenBy
    // ThenByDescending
    // ToArray
    // ToDictionary
    // ToHashSet
    // ToList
    // ToLookup

#pragma endregion todo

private:
    class Controller;

    Enumerable(promise_type& promise);

    Controller controller_{};
}; // class Enumerable

#pragma region ctad

template<class TIterator>
Enumerable(TIterator begin, TIterator) -> Enumerable<std::decay_t<decltype(*begin)>>;

template<class TEnumerable>
Enumerable(const TEnumerable& enumerable) -> Enumerable<std::decay_t<decltype(*std::begin(enumerable))>>;

#pragma endregion ctad

#pragma endregion Enumerable

#pragma region Grouping

template<class TKey, class TElement>
class Grouping : public Enumerable<TElement> {
public:
    template<class TEnumerable>
    Grouping(TKey key, const TEnumerable& enumerable);

    const TKey& Key() const;

private:
    TKey key_;
}; // class Grouping

template<class TKey, class TEnumerable>
Grouping(TKey, const TEnumerable& enumerable) -> Grouping<TKey, std::decay_t<decltype(*std::begin(enumerable))>>;

#pragma endregion Grouping

} // namespace cpplinq

#include "Enumerable.iterator.h"
#include "Enumerable.promise_type.h"

#include "Enumerable-impl.h"
#include "Enumerable.iterator-impl.h"
#include "Enumerable.promise_type-impl.h"
