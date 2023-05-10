/**
 * @file bounded_vector.hpp
 * @author Robin Passama
 * @brief include file for bounded vector class
 * @date 2022-06-10
 *
 * @ingroup containers
 *
 */
#pragma once

#include <cstdint>
#include <utility>
#include <stdexcept>
#include <pid/memory_zone.hpp>

namespace pid {

template <typename T, unsigned int Limit>
class BoundedVector : public MemoryZone<T, Limit> {
public:
    using size_type = typename MemoryZone<T, Limit>::size_type;
    using iterator = typename MemoryZone<T, Limit>::iterator;
    using const_iterator = typename MemoryZone<T, Limit>::const_iterator;

    BoundedVector() : MemoryZone<T, Limit>() {
    }
    virtual ~BoundedVector() = default;
    BoundedVector(const BoundedVector& copied) : MemoryZone<T, Limit>(copied) {
    }
    BoundedVector(BoundedVector&& moved)
        : MemoryZone<T, Limit>(std::move(moved)) {
    }
    BoundedVector& operator=(const BoundedVector& copied) {
        this->MemoryZone<T, Limit>::operator=(copied);
        return (*this);
    }
    BoundedVector& operator=(BoundedVector&& moved) {
        this->MemoryZone<T, Limit>::operator=(std::move(moved));
        return (*this);
    }

    T& operator[](size_type index) {
        if (index >= this->size()) {
            throw std::out_of_range(
                "calling operator[] with index " + std::to_string(index) +
                " that is greater than size:" + std::to_string(this->size()));
        }
        iterator it;
        if (index > this->size() / 2) {
            it = this->last();
            auto i = this->size() - 1;
            while (i-- != index) {
                --it;
            }
        } else {
            it = this->begin();
            size_type i = 0;
            while (i++ != index) {
                ++it;
            }
        }
        return *it;
    }

    const T& operator[](size_type index) const {
        if (index >= this->size()) {
            throw std::out_of_range(
                "calling operator[] with index " + std::to_string(index) +
                " that is greater than size:" + std::to_string(this->size()));
        }
        iterator it;
        if (index > this->size() / 2) {
            it = this->last();
            auto i = this->size() - 1;
            while (i-- != index) {
                --it;
            }
        } else {
            it = this->begin();
            size_type i = 0;
            while (i++ != index) {
                ++it;
            }
        }
        return *it;
    }

    // using the circular buffer as a FIFO queue
    T& front() {
        if (this->empty()) {
            throw std::out_of_range("calling front() on an empty vector");
        }
        return (*this->begin());
    }

    T& back() {
        if (this->empty()) {
            throw std::out_of_range("calling back() on an empty vector");
        }
        return (*this->last());
    }

    iterator pop_front() noexcept {
        return (this->remove_element(this->begin()));
    }

    iterator pop_back() {
        return (this->remove_element(this->last()));
    }

    iterator push_front(T&& t) noexcept {
        auto it = this->add_element(this->begin());
        if (it != this->end()) {
            *it = std::move(t);
        }
        return (it);
    }

    iterator push_front(const T& t) noexcept {
        auto it = this->add_element(this->begin());
        if (it != this->end()) {
            *it = t;
        }
        return (it);
    }

    iterator push_back(T&& t) noexcept {
        auto it = this->add_element(this->end());
        if (it != this->end()) {
            *it = std::move(t);
        }
        return (it);
    }

    iterator push_back(const T& t) noexcept {
        auto it = this->add_element(this->end());
        if (it != this->end()) {
            *it = t;
        }
        return (it);
    }

    void resize(size_t count) {
        if (count > this->size()) {
            for (size_t i = 0; i < count - this->size(); i++) {
                push_back(T{});
            }
        } else if (count < this->size()) {
            for (size_t i = 0; i < count - this->size(); i++) {
                pop_back();
            }
        }
    }
};

} // namespace pid
