/**
 * @file span.hpp
 * @author Robin Passama
 * @brief include file for span class
 * @date 2022-06-10
 *
 * @ingroup containers
 *
 */
#pragma once

#include <array>
#include <cstddef>
#include <iterator>
#include <limits>
#include <type_traits>

#include <cassert>

namespace pid {

inline constexpr std::size_t dynamic_extent =
    std::numeric_limits<std::size_t>::max();

template <std::size_t Extent>
class SpanSize {
public:
    constexpr SpanSize() noexcept = default;
    explicit constexpr SpanSize([[maybe_unused]] size_t size) noexcept {
        assert(size == Extent);
    };

    [[nodiscard]] constexpr std::size_t size() const {
        return Extent;
    }
};

template <>
class SpanSize<dynamic_extent> {
public:
    constexpr SpanSize() noexcept = default;

    explicit constexpr SpanSize(size_t size) noexcept : size_{size} {
    }

    [[nodiscard]] constexpr std::size_t size() const {
        return size_;
    }

private:
    std::size_t size_{0};
};

template <typename T, std::size_t Extent = dynamic_extent>
class Span : public SpanSize<Extent> {
public:
    static constexpr std::size_t extent = Extent;

    using element_type = T;
    using value_type = std::remove_cv_t<T>;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;
    using iterator = pointer;
    using reverse_iterator = std::reverse_iterator<iterator>;

    template <std::size_t N = Extent,
              std::enable_if_t<N == 0 or N == dynamic_extent, int> = 0>
    constexpr Span() noexcept : SpanSize<Extent>{} {
    }

    constexpr Span(T* first, size_type count)
        : SpanSize<Extent>{count}, data_{first} {
    }

    constexpr Span(T* first, T* last)
        : SpanSize<Extent>{static_cast<size_type>(last - first)}, data_{first} {
    }

    template <std::size_t N>
    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
    constexpr Span(T (&arr)[N]) noexcept : SpanSize<Extent>{N}, data_{arr} {
    }

    template <typename U, std::size_t N>
    constexpr Span(std::array<U, N>& arr) noexcept
        : SpanSize<Extent>{N}, data_{arr.data()} {
    }

    template <typename U, std::size_t N>
    constexpr Span(const std::array<U, N>& arr) noexcept
        : SpanSize<Extent>{N}, data_{arr.data()} {
    }

    template <
        typename U, std::size_t N, std::size_t S = Extent,
        std::enable_if_t<
            S == dynamic_extent or N == dynamic_extent or N == Extent, int> = 0>
    constexpr Span(const Span<U, N>& span) noexcept
        : SpanSize<Extent>{span.size()}, data_{span.data()} {
    }

    constexpr Span(const Span&) noexcept = default;
    constexpr Span& operator=(const Span&) noexcept = default;

    using SpanSize<Extent>::size;

    [[nodiscard]] constexpr iterator begin() const {
        return data_;
    }

    [[nodiscard]] constexpr iterator end() const {
        return data_ + size();
    }

    [[nodiscard]] constexpr reverse_iterator rbegin() const {
        return std::reverse_iterator{begin()};
    }

    [[nodiscard]] constexpr reverse_iterator rend() const {
        return std::reverse_iterator{end()};
    }

    [[nodiscard]] constexpr reference front() const {
        return *begin();
    }

    [[nodiscard]] constexpr reference back() const {
        return *(end() - 1);
    }

    [[nodiscard]] constexpr reference operator[](size_type idx) const {
        return data_[idx];
    }

    [[nodiscard]] constexpr pointer data() const noexcept {
        return data_;
    }

    [[nodiscard]] constexpr size_type size_bytes() const noexcept {
        return size() * sizeof(T);
    }

    [[nodiscard]] constexpr bool empty() const noexcept {
        return size() == 0;
    }

    template <std::size_t Count>
    [[nodiscard]] constexpr Span<T, Count> first() const {
        static_assert(Count <= Extent);
        return {data_, Count};
    }

    [[nodiscard]] constexpr Span<T, dynamic_extent>
    first(size_type count) const {
        return {data_, count};
    }

    template <std::size_t Count>
    [[nodiscard]] constexpr Span<T, Count> last() const {
        static_assert(Count <= Extent);
        return {data_ + size() - Count, Count};
    }

    [[nodiscard]] constexpr Span<T, dynamic_extent>
    last(size_type count) const {
        return {data_ + size() - count, count};
    }

    template <std::size_t Offset, std::size_t Count = dynamic_extent>
    [[nodiscard]] constexpr Span<T, Count != dynamic_extent    ? Count
                                    : Extent != dynamic_extent ? Extent - Offset
                                                               : dynamic_extent>
    subspan() const {
        if constexpr (Count == dynamic_extent) {
            return {data_ + Offset, size() - Offset};
        } else {
            return {data_ + Offset, Count};
        }
    }

    [[nodiscard]] constexpr Span<T, dynamic_extent>
    subspan(size_type offset, size_type count = dynamic_extent) const {
        if (count == dynamic_extent) {
            return {data_ + offset, size() - offset};
        } else {
            return {data_ + offset, count};
        }
    }

private:
    T* data_{nullptr};
};

// deduction guides

template <typename T, std::size_t Extent>
Span(T (&)[Extent]) -> Span<T, Extent>; // NOLINT(modernize-avoid-c-arrays)

template <typename T, std::size_t Extent>
Span(std::array<T, Extent>&) -> Span<T, Extent>;

template <typename T, std::size_t Extent>
Span(const std::array<T, Extent>&) -> Span<const T, Extent>;

template <typename T>
Span(T*, T*) -> Span<T>;

template <typename T>
Span(T*, std::size_t) -> Span<T>;

// iterator access free functions

template <typename T, std::size_t Extent>
auto begin(pid::Span<T, Extent>& span) {
    return span.begin();
}

template <typename T, std::size_t Extent>
auto begin(const pid::Span<T, Extent>& span) {
    return span.begin();
}

template <typename T, std::size_t Extent>
auto end(pid::Span<T, Extent>& span) {
    return span.end();
}

template <typename T, std::size_t Extent>
auto end(const pid::Span<T, Extent>& span) {
    return span.end();
}

template <typename T, std::size_t Extent>
auto rbegin(pid::Span<T, Extent>& span) {
    return span.rbegin();
}

template <typename T, std::size_t Extent>
auto rbegin(const pid::Span<T, Extent>& span) {
    return span.rbegin();
}

template <typename T, std::size_t Extent>
auto rend(pid::Span<T, Extent>& span) {
    return span.rend();
}

template <typename T, std::size_t Extent>
auto rend(const pid::Span<T, Extent>& span) {
    return span.rend();
}

template <typename T, std::size_t Extent = dynamic_extent>
using span = Span<T, Extent>;

} // namespace pid