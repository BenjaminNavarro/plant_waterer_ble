/**
 * @file vector_map.hpp
 * @author Benjamin Navarro
 * @brief include file for vector map class
 * @date 2022-06-10
 *
 * @ingroup containers
 *
 */
#pragma once

#include <algorithm>
#include <memory>
#include <utility>
#include <stdexcept>
#include <tuple>
#include <vector>

namespace pid {

namespace detail {
template <typename T, typename U>
using equality_comparison_t =
    decltype(std::declval<T&>() == std::declval<U&>());

template <typename T, typename U, typename = std::void_t<>>
struct IsEqualityComparable : std::false_type {};

template <typename T, typename U>
struct IsEqualityComparable<T, U, std::void_t<equality_comparison_t<T, U>>>
    : std::is_same<equality_comparison_t<T, U>, bool> {};

template <typename T, typename U>
static constexpr bool is_equality_comparable =
    IsEqualityComparable<T, U>::value;
} // namespace detail

template <typename Key, typename Value>
class VectorMapPair {
public:
    constexpr VectorMapPair() = default;

    constexpr VectorMapPair(const Key& key, const Value& value)
        : key_{key}, value_{value} {
    }

    template <typename K = Key, typename V = Value, typename ValueT = Value,
              std::enable_if_t<std::is_move_constructible_v<ValueT>, int> = 0>
    constexpr VectorMapPair(K&& key, V&& value)
        : key_{std::forward<K>(key)}, value_{std::forward<V>(value)} {
    }

    template <
        typename K = Key, typename V = Value, typename ValueT = Value,
        std::enable_if_t<not std::is_move_constructible_v<ValueT>, int> = 0>
    constexpr VectorMapPair(K&& key, const V& value)
        : key_{std::forward<K>(key)}, value_{value} {
    }

    template <typename K, typename V>
    constexpr VectorMapPair(VectorMapPair<K, V> pair)
        : key_{pair.key_}, value_{pair.value_} {
    }

    template <typename... KArgs, typename... VArgs>
    constexpr VectorMapPair(std::piecewise_construct_t /*unused*/,
                            std::tuple<KArgs...> key_args,
                            std::tuple<VArgs...> value_args)
        : key_{std::apply(
              [](KArgs&&... args) { return Key{std::forward<KArgs>(args)...}; },
              key_args)},
          value_{std::apply(
              [](VArgs&&... args) { return Key{std::forward<VArgs>(args)...}; },
              value_args)} {
    }

    const Key& key() const {
        return key_;
    }

    Value& value() {
        return value_;
    }

    const Value& value() const {
        return value_;
    }

private:
    Key key_{};
    Value value_{};
};

template <typename K, typename V>
VectorMapPair(K, V) -> VectorMapPair<K, V>;

//! \brief Iterator for VectorMap. Just wraps an iterator from the underlying
//! vector
//!
//! \tparam Key VectorMap's Key type
//! \tparam Value VectorMap's Value type
//! \tparam NodeContainer VectorMap's NodeContainer template
//! \tparam Forward True for a forward iterator, false for a reverse one
template <typename Key, typename Value, template <typename> class NodeContainer,
          bool Forward>
class VectorMapIterator {
public:
    using container_type = std::vector<
        NodeContainer<VectorMapPair<Key, std::remove_const_t<Value>>>>;
    using container_iterator_type = std::conditional_t<
        Forward,
        std::conditional_t<std::is_const_v<Value>,
                           typename container_type::const_iterator,
                           typename container_type::iterator>,
        std::conditional_t<std::is_const_v<Value>,
                           typename container_type::const_reverse_iterator,
                           typename container_type::reverse_iterator>>;

    using iterator_category =
        typename container_iterator_type::iterator_category;
    using difference_type = typename container_iterator_type::difference_type;
    using value_type = Value;
    using pointer = Value*;
    using reference = Value&;
    using iterator = VectorMapIterator<Key, std::remove_const_t<Value>,
                                       NodeContainer, Forward>;
    using const_iterator =
        VectorMapIterator<Key, std::add_const_t<Value>, NodeContainer, Forward>;

    VectorMapIterator(container_iterator_type it) : it_{it} {
    }

    template <typename V = Value, std::enable_if_t<std::is_const_v<V>, int> = 0>
    VectorMapIterator(typename container_type::iterator it) : it_{it} {
    }

    operator VectorMapIterator<Key, const Value, NodeContainer, Forward>()
        const {
        return it_;
    }

    VectorMapIterator& operator++() {
        it_++;
        return *this;
    }

    VectorMapIterator operator++(int) {
        auto tmp = *this;
        ++(*this);
        return tmp;
    }

    VectorMapIterator& operator--() {
        it_--;
        return *this;
    }

    VectorMapIterator operator--(int) {
        auto tmp = *this;
        --(*this);
        return tmp;
    }

    VectorMapIterator& operator+=(difference_type n) {
        it_ += n;
        return *this;
    }

    VectorMapIterator& operator-=(difference_type n) {
        it_ -= n;
        return *this;
    }

    VectorMapIterator operator+(difference_type n) const {
        auto tmp = *this;
        tmp += n;
        return tmp;
    }

    VectorMapIterator operator-(difference_type n) const {
        auto tmp = *this;
        tmp -= n;
        return tmp;
    }

    difference_type operator-(const const_iterator& other) const {
        return it_ - other.it_;
    }

    friend bool operator==(const VectorMapIterator& a,
                           const VectorMapIterator& b) {
        return a.it_ == b.it_;
    }

    friend bool operator!=(const VectorMapIterator& a,
                           const VectorMapIterator& b) {
        return a.it_ != b.it_;
    }

    friend bool operator<(const VectorMapIterator& a,
                          const VectorMapIterator& b) {
        return a.it_ < b.it_;
    }

    friend bool operator<=(const VectorMapIterator& a,
                           const VectorMapIterator& b) {
        return a.it_ <= b.it_;
    }

    friend bool operator>(const VectorMapIterator& a,
                          const VectorMapIterator& b) {
        return a.it_ > b.it_;
    }

    friend bool operator>=(const VectorMapIterator& a,
                           const VectorMapIterator& b) {
        return a.it_ >= b.it_;
    }

    const auto& operator*() const {
        return **it_;
    }

    auto& operator*() {
        return **it_;
    }

    auto* operator->() {
        return it_->get();
    }

private:
    template <typename, typename, template <typename> class>
    friend class VectorMap;

    friend iterator;

    container_iterator_type it_;
};

//! \brief Wraps T inside a unique_ptr so that moving the node doesn't change
//! the stored T address
template <typename T>
struct StableVectorMapNode {
    static constexpr bool is_stable = true;

    StableVectorMapNode(std::unique_ptr<T> x) : v{std::move(x)} {
    }

    StableVectorMapNode(StableVectorMapNode&&) noexcept = default;
    StableVectorMapNode(const StableVectorMapNode& other) : v{create(*other)} {
    }

    ~StableVectorMapNode() = default;

    StableVectorMapNode& operator=(StableVectorMapNode&&) noexcept = default;
    StableVectorMapNode& operator=(const StableVectorMapNode& other) {
        v = create(*other);
        return *this;
    }

    const T* get() const {
        return v.get();
    }

    T* get() {
        return v.get();
    }

    const T* operator->() const {
        return get();
    }

    T* operator->() {
        return get();
    }

    const T& operator*() const {
        return *v;
    }

    T& operator*() {
        return *v;
    }

    template <typename... Args>
    static auto create(Args&&... args) {
        return std::make_unique<T>(std::forward<Args>(args)...);
    }

    std::unique_ptr<T> v;
};

//! \brief Wraps a T directly to avoid memory allocations and increase cache
//! friendlyness but moving the node will change the stored T address
template <typename T>
struct UnstableVectorMapNode {
    static constexpr bool is_stable = false;

    UnstableVectorMapNode() = default;
    UnstableVectorMapNode(T&& x) : v{std::move(x)} {
    }

    auto* get() const {
        return &v;
    }

    auto* get() {
        return &v;
    }

    const auto* operator->() const {
        return get();
    }

    auto* operator->() {
        return get();
    }

    const auto& operator*() const {
        return v;
    }

    auto& operator*() {
        return v;
    }

    template <typename... Args>
    static auto create(Args&&... args) {
        return T{std::forward<Args>(args)...};
    }

    T v;
};

//! \brief An associative container that keeps insertion order, minimizes memory
//! allocations and provide better cache friendlyness than std::map.
//!
//! The only guanrantee that std::map provides that VectorMap cannot is iterator
//! stability. This means that adding or removing elements might invalidate
//! existing iterators (assume it will always will).
//!
//! Then, the main difference with std::map is that elements are not sorted (to
//! preserve insertion order) and so a linear search must be performed to find
//! an element. But if your keys are fast to compare and that you don't require
//! pointer stability (see below) then the search should be quite fast as all
//! the key/value pairs are contiguous in memory which should allows for a low
//! number of cache miss.
//!
//! As hinted above, VectorMap can provide key and value pointer stability if
//! needed. In that case, each key/value pair will be dynamically allocated
//! (single allocation for both) to keep their addresses stable when adding or
//! removing elements from the container. If you need this stability, use
//! pid::vector_map (or alternatively pid::stable_vector_map if you prefer to be
//! explicit).
//!
//! If you don't require pointer stability, then you can use
//! pid::unstable_vector_map which should gives you better performance (less
//! memory allocations, better cache friendlyness)
//!
//! All element access operations accepts any type that is equality comparable
//! with the Key type. This avoids the construction of temporaries just for the
//! sake of looking up a key (e.g you can pass an std::string_view when Key =
//! std::string without having to convert the string_view to a string)
//!
//! \tparam Key Key type
//! \tparam Value Value type
//! \tparam NodeContainer How nodes are stored inside the underlying vector
template <typename Key, typename Value,
          template <typename> class NodeContainer = StableVectorMapNode>
class VectorMap {
public:
    using key_type = Key;
    using mapped_type = Value;
    using pair_type = VectorMapPair<Key, Value>;
    using value_type = NodeContainer<pair_type>;
    using size_type = typename std::vector<value_type>::size_type;
    using difference_type = typename std::vector<value_type>::difference_type;
    using reference = value_type;
    using const_reference = const value_type&;
    using pointer = value_type*;
    using const_pointer = const value_type*;
    using iterator = VectorMapIterator<Key, Value, NodeContainer, true>;
    using const_iterator =
        VectorMapIterator<Key, const Value, NodeContainer, true>;
    using reverse_iterator =
        VectorMapIterator<Key, Value, NodeContainer, false>;
    using const_reverse_iterator =
        VectorMapIterator<Key, const Value, NodeContainer, false>;

    template <typename T>
    static constexpr bool is_comparable_key =
        detail::is_equality_comparable<T, Key>;

    template <typename T>
    static constexpr bool is_key_constructible_from =
        std::is_constructible_v<Key, const T&> or
        std::is_constructible_v<Key, T&&>;

    template <typename T>
    static constexpr bool is_value_constructible_from =
        std::is_constructible_v<Value, const T&> or
        std::is_constructible_v<Value, T&&>;

    static constexpr bool is_stable = value_type::is_stable;

    VectorMap() = default;

    VectorMap(const_iterator begin, const_iterator end) {
        insert(begin, end);
    }

    explicit VectorMap(std::initializer_list<pair_type> init) {
        insert(init);
    }

    VectorMap(const VectorMap& other) = default;
    VectorMap(VectorMap&& other) noexcept = default;

    ~VectorMap() = default;

    VectorMap& operator=(const VectorMap& other) = default;
    VectorMap& operator=(VectorMap&& other) noexcept = default;

    VectorMap& operator=(std::initializer_list<pair_type> init) {
        *this = VectorMap{init};
        return *this;
    }

    // Element access

    template <typename K>
    [[nodiscard]] Value& at(const K& key) {
        static_assert(is_comparable_key<K>);
        return const_cast<Value&>(std::as_const(*this).at(key));
    }

    template <typename K>
    [[nodiscard]] const Value& at(const K& key) const {
        static_assert(is_comparable_key<K>);
        if (auto it = find(key); it != end()) {
            return (*it).value();
        } else {
            throw std::out_of_range("VectorMap::at key not found");
        }
    }

    template <typename K>
    [[nodiscard]] Value& operator[](const K& key) {
        static_assert(is_comparable_key<K>);
        if (auto it = find(key); it != end()) {
            return it->value();
        } else {
            return insert(key, Value{}).first->value();
        }
    }

    // Capacity

    [[nodiscard]] bool empty() const noexcept {
        return elements_.empty();
    }

    [[nodiscard]] size_type size() const noexcept {
        return elements_.size();
    }

    [[nodiscard]] size_type max_size() const noexcept {
        return elements_.max_size();
    }

    [[nodiscard]] size_type capacity() const noexcept {
        return elements_.capacity();
    }

    void reserve(size_t count) {
        elements_.reserve(count);
    }

    // Modifiers

    void clear() {
        elements_.clear();
    }

    std::pair<iterator, bool> insert(const pair_type& element) {
        return insert(element.key(), element.value());
    }

    std::pair<iterator, bool> insert(const value_type& element) {
        return insert(element->key(), element->value());
    }

    template <typename K, typename V,
              std::enable_if_t<not std::is_convertible_v<K, const_iterator> and
                                   not std::is_convertible_v<K, iterator>,
                               int> = 0>
    std::pair<iterator, bool> insert(K&& key, V&& value) {
        static_assert(is_key_constructible_from<K> and
                      is_value_constructible_from<V>);
        if (auto it = find(key); it != end()) {
            return {it, false};
        } else {
            elements_.emplace_back(value_type::create(std::forward<K>(key),
                                                      std::forward<V>(value)));
            return {--end(), true};
        }
    }

    void insert(const_iterator begin, const_iterator end) {
        for (auto it = begin; it != end; ++it) {
            insert(it->key(), it->value());
        }
    }

    void insert(std::initializer_list<pair_type> init) {
        for (auto it = init.begin(); it != init.end(); ++it) {
            insert(it->key(), it->value());
        }
    }

    std::pair<iterator, bool> insert_or_assign(const pair_type& element) {
        return insert_or_assign(element.key(), element.value());
    }

    std::pair<iterator, bool> insert_or_assign(const value_type& element) {
        return insert_or_assign(element->key(), element->value());
    }

    template <typename K, typename V>
    std::pair<iterator, bool> insert_or_assign(K&& key, V&& value) {
        static_assert(is_key_constructible_from<K> and
                      is_value_constructible_from<V>);
        if (auto it = find(key); it != end()) {
            it->value() = value;
            return {it, false};
        } else {
            elements_.emplace_back(value_type::create(std::forward<K>(key),
                                                      std::forward<V>(value)));
            return {--end(), true};
        }
    }

    void insert_or_assign(const_iterator begin, const_iterator end) {
        for (auto it = begin; it != end; ++it) {
            insert_or_assign(it->key(), it->value());
        }
    }

    void insert_or_assign(std::initializer_list<pair_type> init) {
        for (auto it = init.begin(); it != init.end(); ++it) {
            insert_or_assign(it->key(), it->value());
        }
    }

    template <typename... Args>
    std::pair<iterator, bool> emplace(Args&&... args) {
        value_type element = value_type::create(std::forward<Args>(args)...);
        if (auto it = find(element->key()); it != end()) {
            return {it, false};
        } else {
            elements_.emplace_back(std::move(element));
            return {--end(), true};
        }
    }

    template <typename K, typename... Args>
    std::pair<iterator, bool> try_emplace(K&& key, Args&&... args) {
        if (auto it = find(key); it != end()) {
            return {it, false};
        } else {
            elements_.emplace_back(value_type::create(
                std::forward<K>(key), std::forward<Args>(args)...));
            return {--end(), true};
        }
    }

    void erase(const_iterator pos) {
        elements_.erase(pos.it_);
    }

    void erase(const_iterator first, const_iterator last) {
        elements_.erase(first.it_, last.it_);
    }

    template <typename K,
              std::enable_if_t<not std::is_convertible_v<K, const_iterator> and
                                   not std::is_convertible_v<K, iterator>,
                               int> = 0>
    void erase(K&& key) {
        if (auto it = find(key); it != end()) {
            erase(it);
        }
    }

    void swap(VectorMap& other) {
        elements_.swap(other.elements_);
    }

    void merge(VectorMap& other) {
        // Iterators can be invalidated when adding/removing elements so we keep
        // a count of of many loop iterations we have to perform and recompute
        // an iterator position at each iteration to make sure it is always
        // valid
        size_type idx{};
        const auto elems = other.size();
        while (idx < elems) {
            auto it =
                std::next(other.begin(), static_cast<difference_type>(idx));
            auto [pos, inserted] = insert(*it);
            if (inserted) {
                other.erase(it);
            }
            ++idx;
        }
    }

    void sort() {
        sort(std::less<>{});
    }

    template <class Compare>
    void sort(Compare comp) {
        std::sort(elements_.begin(), elements_.end(),
                  [comp](const auto& lhs, const auto& rhs) {
                      return comp(lhs->key(), rhs->key());
                  });
    }

    // Lookup

    template <typename K>
    [[nodiscard]] size_type count(const K& key) const {
        return contains(key) ? 1 : 0;
    }

    template <typename K>
    [[nodiscard]] iterator find(const K& key) noexcept {
        return std::find_if(
            elements_.begin(), elements_.end(),
            [&key](const value_type& pair) { return pair->key() == key; });
    }

    template <typename K>
    [[nodiscard]] const_iterator find(const K& key) const noexcept {
        return std::find_if(
            elements_.begin(), elements_.end(),
            [&key](const value_type& pair) { return pair->key() == key; });
    }

    template <typename K>
    [[nodiscard]] bool contains(const K& key) const {
        return find(key) != end();
    }

    // Iterators

    [[nodiscard]] iterator begin() noexcept {
        return elements_.begin();
    }

    [[nodiscard]] const_iterator begin() const noexcept {
        return elements_.begin();
    }

    [[nodiscard]] const_iterator cbegin() const noexcept {
        return begin();
    }

    [[nodiscard]] reverse_iterator rbegin() noexcept {
        return elements_.rbegin();
    }

    [[nodiscard]] const_reverse_iterator rbegin() const noexcept {
        return elements_.rbegin();
    }

    [[nodiscard]] const_reverse_iterator crbegin() const noexcept {
        return rbegin();
    }

    [[nodiscard]] iterator end() noexcept {
        return elements_.end();
    }

    [[nodiscard]] const_iterator end() const noexcept {
        return elements_.end();
    }

    [[nodiscard]] const_iterator cend() const noexcept {
        return end();
    }

    [[nodiscard]] reverse_iterator rend() noexcept {
        return elements_.rend();
    }

    [[nodiscard]] const_reverse_iterator rend() const noexcept {
        return elements_.rend();
    }

    [[nodiscard]] const_reverse_iterator crend() const noexcept {
        return rend();
    }

private:
    std::vector<value_type> elements_;
};

template <typename Key, typename Value, template <typename> class NodeContainer>
[[nodiscard]] bool operator==(const VectorMap<Key, Value, NodeContainer>& a,
                              const VectorMap<Key, Value, NodeContainer>& b) {
    if (a.size() != b.size()) {
        return false;
    }

    auto it_a = a.begin();
    auto it_b = b.begin();
    for (size_t i = 0; i < a.size(); i++) {
        if (not(it_a->key() == it_b->key() and
                it_a->value() == it_b->value())) {
            return false;
        }
        ++it_a;
        ++it_b;
    }
    return true;
}

template <typename Key, typename Value, template <typename> class NodeContainer>
[[nodiscard]] bool operator!=(const VectorMap<Key, Value, NodeContainer>& a,
                              const VectorMap<Key, Value, NodeContainer>& b) {
    return not(a == b);
}

template <typename Key, typename Value, template <typename> class NodeContainer>
void swap(VectorMap<Key, Value, NodeContainer>& a,
          VectorMap<Key, Value, NodeContainer>& b) {
    a.swap(b);
}

template <typename Key, typename Value, template <typename> class NodeContainer>
[[nodiscard]] auto begin(VectorMap<Key, Value, NodeContainer>& m) noexcept {
    return m.begin();
}

template <typename Key, typename Value, template <typename> class NodeContainer>
[[nodiscard]] auto
begin(const VectorMap<Key, Value, NodeContainer>& m) noexcept {
    return m.begin();
}

template <typename Key, typename Value, template <typename> class NodeContainer>
[[nodiscard]] auto
cbegin(const VectorMap<Key, Value, NodeContainer>& m) noexcept {
    return m.cbegin();
}

template <typename Key, typename Value, template <typename> class NodeContainer>
[[nodiscard]] auto rbegin(VectorMap<Key, Value, NodeContainer>& m) noexcept {
    return m.rbegin();
}

template <typename Key, typename Value, template <typename> class NodeContainer>
[[nodiscard]] auto
rbegin(const VectorMap<Key, Value, NodeContainer>& m) noexcept {
    return m.rbegin();
}

template <typename Key, typename Value, template <typename> class NodeContainer>
[[nodiscard]] auto
crbegin(const VectorMap<Key, Value, NodeContainer>& m) noexcept {
    return m.crbegin();
}

template <typename Key, typename Value, template <typename> class NodeContainer>
[[nodiscard]] auto end(VectorMap<Key, Value, NodeContainer>& m) noexcept {
    return m.end();
}

template <typename Key, typename Value, template <typename> class NodeContainer>
[[nodiscard]] auto end(const VectorMap<Key, Value, NodeContainer>& m) noexcept {
    return m.end();
}

template <typename Key, typename Value, template <typename> class NodeContainer>
[[nodiscard]] auto
cend(const VectorMap<Key, Value, NodeContainer>& m) noexcept {
    return m.cend();
}

template <typename Key, typename Value, template <typename> class NodeContainer>
[[nodiscard]] auto rend(VectorMap<Key, Value, NodeContainer>& m) noexcept {
    return m.rend();
}

template <typename Key, typename Value, template <typename> class NodeContainer>
[[nodiscard]] auto
rend(const VectorMap<Key, Value, NodeContainer>& m) noexcept {
    return m.rend();
}

template <typename Key, typename Value, template <typename> class NodeContainer>
[[nodiscard]] auto
crend(const VectorMap<Key, Value, NodeContainer>& m) noexcept {
    return m.crend();
}

template <typename Key, typename Value>
using stable_vector_map = VectorMap<Key, Value, StableVectorMapNode>;

template <typename Key, typename Value>
using unstable_vector_map = VectorMap<Key, Value, UnstableVectorMapNode>;

template <typename Key, typename Value>
using vector_map = stable_vector_map<Key, Value>;

} // namespace pid

namespace std {

template <typename K, typename V>
struct tuple_size<pid::VectorMapPair<K, V>>
    : std::integral_constant<std::size_t, 2> {};

template <class K, class V>
struct tuple_element<0, pid::VectorMapPair<K, V>> {
    using type = const K;
};

template <class K, class V>
struct tuple_element<1, pid::VectorMapPair<K, V>> {
    using type = V;
};

} // namespace std

namespace pid {
template <std::size_t I, class K, class V>
constexpr auto& get(pid::VectorMapPair<K, V>& pair) noexcept {
    static_assert(I < 2);
    if constexpr (I == 0) {
        return pair.key();
    } else {
        return pair.value();
    }
}

template <std::size_t I, class K, class V>
constexpr auto& get(const pid::VectorMapPair<K, V>& pair) noexcept {
    static_assert(I < 2);
    if constexpr (I == 0) {
        return pair.key();
    } else {
        return pair.value();
    }
}

template <std::size_t I, class K, class V>
constexpr auto&& get(pid::VectorMapPair<K, V>&& pair) noexcept {
    static_assert(I < 2);
    if constexpr (I == 0) {
        return pair.key();
    } else {
        return pair.value();
    }
}

template <std::size_t I, class K, class V>
constexpr auto&& get(const pid::VectorMapPair<K, V>&& pair) noexcept {
    static_assert(I < 2);
    if constexpr (I == 0) {
        return pair.key();
    } else {
        return pair.value();
    }
}

} // namespace pid