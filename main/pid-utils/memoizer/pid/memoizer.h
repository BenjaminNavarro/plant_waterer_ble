#pragma once

#include <utility>
#include <unordered_map>
#include <functional>
#include <cstdint>
#include <cstddef>

namespace pid {

namespace detail {
template <class T>
void hash_combine(std::size_t& seed, T const& value) {
    if constexpr (sizeof(std::size_t) == sizeof(std::uint64_t)) {
        seed ^= std::hash<T>()(value) + 0x517cc1b727220a95 + (seed << 6) +
                (seed >> 2);
    } else {
        seed ^= std::hash<T>()(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }
}

template <typename... T>
std::size_t hash_all(T&&... values) {
    std::size_t seed{0};
    (hash_combine(seed, values), ...);
    return seed;
}

struct TupleHasher {
    using is_transparent = void;

    template <typename... T>
    std::size_t operator()(const std::tuple<T...>& tuple) const noexcept {
        return std::apply(hash_all<const T&...>, tuple);
    }
};

} // namespace detail

template <typename, typename = void>
class Memoizer;

template <typename Ret, typename... Args, typename Tup>
class Memoizer<Ret(Args...), Tup> {
public:
    static_assert(sizeof...(Args) == std::tuple_size_v<Tup>);

    template <typename T>
    Memoizer(T func) : function_{std::move(func)} {
    }

    template <typename... T>
    const Ret& operator()(T&&... args) {
        static_assert(sizeof...(args) == sizeof...(Args));

        if (auto it = find(std::forward<T>(args)...); it != cache_.end()) {
            if (not it->second.valid) {
                it->second.value = function_(std::forward<T>(args)...);
                it->second.valid = true;
            }
            return it->second.value;
        } else {
            auto [where, inserted] = cache_.emplace(
                std::tie(args...),
                Entry{function_(std::forward<T>(args)...), true});
            return where->second.value;
        }
    }

    void invalidate_all() {
        for (auto& [key, entry] : cache_) {
            entry.valid = false;
        }
    }

    template <typename... T>
    void invalidate(T&&... args) {
        static_assert(sizeof...(args) == sizeof...(Args));

        if (auto it = find(std::forward<T>(args)...); it != cache_.end()) {
            it->second.valid = false;
        }
    }

    void reevaluate_all() {
        for (auto& [key, value] : cache_) {
            value.value = std::apply(function_, key);
            value.valid = true;
        }
    }

    template <typename... T>
    void reevaluate(T&&... args) {
        static_assert(sizeof...(args) == sizeof...(Args));

        if (auto it = find(std::forward<T>(args)...); it != cache_.end()) {
            it->second.value = std::apply(function_, it->first);
            it->second.valid = true;
        }
    }

    //! \brief Reserve the memory required to hold at least \p count function
    //! evaluation results
    void reserve(std::size_t count) {
        cache_.reserve(count);
    }

private:
    struct Entry {
        Ret value;
        bool valid{};
    };

#ifndef __cpp_lib_generic_unordered_lookup
    template <typename T, std::size_t... I>
    [[nodiscard]] Tup to_tuple(T args, std::index_sequence<I...> /*unused*/
    ) {
        return Tup{
            std::tuple_element_t<I, Tup>{std::move(std::get<I>(args))}...};
    }
#endif

    template <typename... T>
    [[nodiscard]] auto find(T&&... args) {
#ifdef __cpp_lib_generic_unordered_lookup
        return cache_.find(std::tie(args...));
#else
        return cache_.find(
            to_tuple(std::tie(args...),
                     std::make_index_sequence<std::tuple_size_v<Tup>>{}));
#endif
    }

    std::unordered_map<Tup, Entry, detail::TupleHasher, std::equal_to<>> cache_;
    std::function<Ret(Args...)> function_;
};

template <typename Ret, typename... Args>
class Memoizer<Ret(Args...), void>
    : public Memoizer<Ret(Args...), std::tuple<Args...>> {
public:
    using Memoizer<Ret(Args...), std::tuple<Args...>>::Memoizer;
    using Memoizer<Ret(Args...), std::tuple<Args...>>::operator=;
};

} // namespace pid