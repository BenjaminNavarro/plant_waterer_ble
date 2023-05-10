/**
 * @file memory_zone.hpp
 * @author Robin Passama
 * @brief include file for memory zone utilisty class
 * @date 2022-06-10
 *
 * @ingroup containers
 *
 */
#pragma once

#include <cstdint>
#include <array>
#include <iostream>
#include <functional>

namespace pid {

template <typename T>
struct MemoryZoneElement {
    using reference_t = T&;
    using const_reference_t = const T&;
    using pointer_t = T*;
    using const_pointer_t = const T*;
    using memory_pointer =
        MemoryZoneElement<T>*; // points somewhere in the containing heap

private:
    uint8_t used_;
    T val_;
    memory_pointer next_, previous_;
    uint32_t index_;

public:
    MemoryZoneElement()
        : used_{0},
          val_{}, // T must be default constructible
          next_{nullptr},
          previous_{nullptr},
          index_{0} {
    }

    MemoryZoneElement(const MemoryZoneElement&) = delete;
    MemoryZoneElement& operator=(const MemoryZoneElement&) = delete;
    MemoryZoneElement(MemoryZoneElement&&) = delete;
    MemoryZoneElement& operator=(MemoryZoneElement&&) = delete;

    virtual ~MemoryZoneElement() = default;

    uint32_t& index() {
        return (index_);
    }

    const uint32_t& index() const {
        return (index_);
    }

    const_pointer_t pointer() const {
        return (&val_);
    }

    pointer_t pointer() {
        return (&val_);
    }

    reference_t reference() {
        return (val_);
    }

    const_reference_t reference() const {
        return (val_);
    }

    memory_pointer next() const {
        return (next_);
    }

    memory_pointer& next() {
        return (next_);
    }

    memory_pointer previous() const {
        return (previous_);
    }

    memory_pointer& previous() {
        return (previous_);
    }

    void set_used() {
        used_ = true;
    }

    bool used() const {
        return (used_);
    }

    void reset() {
        used_ = false;
        next_ = previous_ = nullptr;
    }
};

template <typename T>
struct MemoryZoneIterator {
    using memory_t = MemoryZoneElement<T>;
    using element_type = memory_t*;

private:
    element_type element_; // the pointed type is always non const

public:
    MemoryZoneIterator() : element_(nullptr) {
    }
    ~MemoryZoneIterator() = default;
    MemoryZoneIterator(const MemoryZoneIterator& it) = default;
    MemoryZoneIterator& operator=(const MemoryZoneIterator& it) = default;

    void set(element_type pointed) {
        element_ = pointed;
    }

    element_type get() const {
        return (element_);
    }

    typename memory_t::pointer_t operator->() const {
        return (element_->pointer());
    }

    typename memory_t::reference_t operator*() {
        return (element_->reference());
    }

    bool operator==(const MemoryZoneIterator& other) const {
        return (element_ == other.element_);
    }

    bool operator!=(const MemoryZoneIterator& other) const {
        return (element_ != other.element_);
    }

    operator bool() const {
        return (element_ != nullptr);
    }

    MemoryZoneIterator& operator++() {
        element_ = element_->next();
        return (*this);
    }

    MemoryZoneIterator& operator--() {
        element_ = element_->previous();
        return (*this);
    }

    MemoryZoneIterator operator++(int) {
        auto temp(*this); // need a copy in this case
        element_ = element_->next();
        return (temp);
    }

    MemoryZoneIterator operator--(int) {
        auto temp(*this); // need a copy in this case
        element_ = element_->previous();
        return (temp);
    }
};

template <typename T>
struct MemoryZoneConstIterator {
    using memory_t = MemoryZoneElement<T>;
    using element_type = memory_t*;

protected:
    element_type element_; // the pointed type is always non const

public:
    MemoryZoneConstIterator() : element_(nullptr) {
    }
    ~MemoryZoneConstIterator() = default;
    MemoryZoneConstIterator(const MemoryZoneConstIterator& it) = default;
    MemoryZoneConstIterator&
    operator=(const MemoryZoneConstIterator& it) = default;

    MemoryZoneConstIterator(const MemoryZoneIterator<T>& it)
        : element_(it.get()) { // this constructor is usefull to create a
                               // const_iterator from an iterator
    }

    element_type get() const {
        return (element_);
    }

    typename memory_t::const_pointer_t operator->() const {
        return (element_->pointer());
    }

    typename memory_t::const_reference_t operator*() const {
        return (element_->reference());
    }

    bool operator==(const MemoryZoneConstIterator& other) const {
        return (element_ == other.element_);
    }

    bool operator!=(const MemoryZoneConstIterator& other) const {
        return (element_ != other.element_);
    }

    operator bool() const {
        return (element_ != nullptr);
    }

    MemoryZoneConstIterator& operator++() {
        element_ = element_->next();
        return (*this);
    }

    MemoryZoneConstIterator& operator--() {
        element_ = element_->previous();
        return (*this);
    }

    MemoryZoneConstIterator operator++(int) {
        auto temp(*this); // need a copy in this case
        element_ = element_->next();
        return (temp);
    }

    MemoryZoneConstIterator operator--(int) {
        auto temp(*this); // need a copy in this case
        element_ = element_->previous();
        return (temp);
    }
};

template <typename T, unsigned int Limit>
class MemoryZone {
public:
    using size_type = uint32_t;

    using contained_type = T;
    using value_type = T;

    using reference = T&;
    using const_reference = const T&;
    using pointer = T*;
    using const_pointer = const T*;

    using iterator = MemoryZoneIterator<T>;
    using const_iterator = MemoryZoneConstIterator<T>;

    using element_type = MemoryZoneElement<T>*;

private:
    std::array<MemoryZoneElement<T>, Limit> data_buffer_;
    iterator first_element_, last_element_;
    uint32_t next_element_slot_;
    size_type size_;
    static const size_type capacity_ = Limit;

    void reset() {
        for (size_type i = 0; i < capacity_; ++i) {
            data_buffer_[i].index() = i;
            data_buffer_[i].reset();
        }
        first_element_.set(nullptr);
        last_element_ = first_element_;
        next_element_slot_ = 0;
        size_ = 0;
    }

    void increment_next_slot_index() {
        if (size_ == capacity_) { // now full
            next_element_slot_ = capacity_;
            return;
        }
        ++next_element_slot_;
        if (next_element_slot_ == capacity_) {
            next_element_slot_ = 0;
        }
        while (data_buffer_[next_element_slot_].used()) {
            ++next_element_slot_;
            if (next_element_slot_ == capacity_) {
                next_element_slot_ = 0;
            }
        }
    }

    element_type use_free_slot() {
        auto curr = at_index(next_element_slot_); // get the first free slot
        ++size_;
        data_buffer_[next_element_slot_].set_used(); // this slot is used
        increment_next_slot_index();
        return (curr);
    }

    void copy(const MemoryZone& other) {
        reset();
        size_ = other.size_;
        next_element_slot_ = other.next_element_slot_;
        // now need to correctly assin pointers

        for (unsigned int i = 0; i < capacity_; ++i) {
            if (other.data_buffer_[i].used()) { // data_buffer_[i] used as well
                data_buffer_[i].set_used();
                if (other.data_buffer_[i].next() != nullptr) {
                    data_buffer_[i].next() =
                        &data_buffer_[other.data_buffer_[i].next()->index()];
                } else {
                    data_buffer_[i].next() = nullptr;
                }
                if (other.data_buffer_[i].previous() != nullptr) {
                    data_buffer_[i].previous() =
                        &data_buffer_
                            [other.data_buffer_[i].previous()->index()];
                } else {
                    data_buffer_[i].previous() = nullptr;
                }
                data_buffer_[i].reference() =
                    *other.data_buffer_[i].pointer(); // copy the data
            } else {
                data_buffer_[i].reset();
            }
        }
        if (other.empty()) {
            first_element_.set(nullptr);
            last_element_.set(nullptr); // if no first then no last element
        } else {
            first_element_.set(
                &data_buffer_[other.first_element_.get()->index()]);
            last_element_.set(&data_buffer_[other.last_element_.get()
                                                ->index()]); // if no first then
                                                             // no last element
        }
    }

protected:
    iterator add_element(iterator iter) {
        if (size_ == capacity_) { // cannot add anything to a full zone
            return end();
        }
        iterator ret;
        if (static_cast<bool>(first_element_)) { // not empty
            if (iter == first_element_) { // insert at first place BEFORE the
                                          // current first element
                // the first element will change (BUT NOT the last one)
                auto temp_first = first_element_;
                // always adding at the beginning
                first_element_.set(
                    use_free_slot()); // get the next free element from the zone
                first_element_.get()->previous() =
                    nullptr; // first has no previous
                first_element_.get()->next() =
                    temp_first.get(); // next is old first element
                temp_first.get()->previous() =
                    first_element_
                        .get(); // previous of old first element is new element
                ret = first_element_;
            } else if (iter == end()) { // insert just AFTER last element
                auto temp_first = last_element_;
                last_element_.set(
                    use_free_slot()); // get the next free element from the zone
                last_element_.get()->previous() = temp_first.get();
                last_element_.get()->next() =
                    nullptr; // last element has no next
                temp_first.get()->next() = last_element_.get();
                ret = last_element_;
            } else { // insert just BEFORE the target element
                auto temp_iter = iter;
                // always adding at the beginning
                iter.set(
                    use_free_slot()); // get the next free element from the zone
                // manage double linked list
                iter.get()->previous() =
                    temp_iter.get()
                        ->previous(); // new element has same previous as the
                                      // previous element
                temp_iter.get()->previous() =
                    iter.get(); // previous of target is new element
                iter.get()->next() =
                    temp_iter.get(); // next is old first element
                iter.get()->previous()->next() =
                    iter.get(); // nect element of previous is the new one
                ret = iter;
            }
        } else { // empty zone, iterator is not useful
            first_element_.set(use_free_slot());
            // manage double linked list
            first_element_.get()->previous() = nullptr;
            first_element_.get()->next() = nullptr;
            last_element_ = first_element_;
            ret = first_element_;
        }
        return ret;
    }

    iterator remove_element(iterator iter) {
        if (empty()) { // canot remove anything from an empty zone
            return end();
        }
        if (iter == first_element_) { // need to change the begin iterator
            // first element becomes the next element
            if (first_element_.get()->next() !=
                nullptr) { // the removed element has a follower
                ++first_element_;
                first_element_.get()->previous() =
                    nullptr; // new first element has no previous (by
                             // definition)
            } else {         // only one element
                first_element_.set(nullptr);
                last_element_ = first_element_;
            }
        } else if (iter == last_element_) { // need to change the last iterator
            // if code pass here it means the last element is not the first one
            --last_element_;
            last_element_.get()->next() =
                nullptr; // new first element has no next (by definition)
        } else { // any element in the memory except the first and last one
            iter.get()->previous()->next() = iter.get()->next();
            // if code pass here then it means this is not the last iterator so
            // iter.get()->next() is never nullptr
            iter.get()->next()->previous() = iter.get()->previous();
        }
        iter.get()->reset(); // reset the removed element (no more used), but
                             // its value is still accessible
        next_element_slot_ =
            iter.get()->index(); // always take the last removed slot as next
                                 // slot to use
        --size_;
        return (iter);
    }

public:
    MemoryZone()
        : data_buffer_(),
          first_element_(),
          last_element_(),
          next_element_slot_(0),
          size_(0) {
        reset();
    }

    MemoryZone(const MemoryZone& copied) : MemoryZone() {
        copy(copied); // deep copy
    }

    MemoryZone(MemoryZone&& moved) : MemoryZone() { // swapping memory
        copy(moved);
    }

    MemoryZone& operator=(const MemoryZone& copied) {
        if (&copied != this) {
            copy(copied);
        }
        return (*this);
    }

    MemoryZone& operator=(MemoryZone&& moved) noexcept {
        copy(moved);
        return (*this);
    }

    virtual ~MemoryZone() = default;

    // iterating over the circular buffer
    size_type size() const {
        return (size_);
    }

    element_type at_index(uint32_t idx) {
        return (&data_buffer_[idx]);
    }

    element_type at_index(uint32_t idx) const {
        return (&data_buffer_[idx]);
    }

    // iterating over the circular buffer
    size_type capacity() const {
        return (capacity_);
    }

    iterator begin() {
        return (first_element_);
    }

    const_iterator begin() const {
        return (const_iterator(first_element_));
    }

    iterator last() {
        return (last_element_);
    }

    const_iterator last() const {
        return (last_element_);
    }

    iterator end() {
        return (iterator());
    }

    const_iterator end() const {
        return (const_iterator());
    }

    iterator start() {
        return (iterator());
    }

    const_iterator start() const {
        return (const_iterator());
    }

    // using the circular buffer as a FIFO queue
    bool full() const {
        return (capacity() == size());
    }

    bool empty() const {
        return (size_ == 0);
    }

    void clear() {
        for (unsigned int i = 0; i < capacity_; ++i) {
            data_buffer_[i].reset();
        }
        first_element_.set(nullptr);
        last_element_.set(nullptr);
        next_element_slot_ = size_ = 0;
    }

    const_iterator
    find(const std::function<bool(const_reference)>& condition) const {
        for (auto iter = begin(); iter != end(); ++iter) {
            if (condition(*iter)) {
                return (iter);
            }
        }
        return (end());
    }

    iterator find(const std::function<bool(const_reference)>& condition) {
        for (auto iter = begin(); iter != end(); ++iter) {
            if (condition(*iter)) {
                return (iter);
            }
        }
        return (end());
    }

    void print_memory(std::ostream& os) {
        for (unsigned int i = 0; i < Limit; ++i) {
            os << std::to_string(i) << ": "
               << reinterpret_cast<void*>(&data_buffer_[i])
               << " used: " << (data_buffer_[i].used() ? "YES" : "NO")
               << " pre: "
               << (data_buffer_[i].previous() == nullptr
                       ? "NULL"
                       : std::to_string(data_buffer_[i].previous()->index()))
               << " next: "
               << (data_buffer_[i].next() == nullptr
                       ? "NULL"
                       : std::to_string(data_buffer_[i].next()->index()))
               << std::endl;
        }
    }
};

} // namespace pid