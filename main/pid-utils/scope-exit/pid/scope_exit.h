/**
 * @defgroup scope-exit scope-exit: trigeering functions when exitting scope
 *
 */

/**
 * @file scope_exit.h
 * @author Benjamin Navarro
 * @brief include file for the scope exit class
 * @date 2022-06-10
 * @ingroup scope-exit
 * @ingroup pid-utils
 */
#pragma once

#include <exception>
#include <functional>
#include <utility>

namespace pid {

template <typename Callable>
class ScopeExit;

template <typename Callable>
class ScopeSuccess;

template <typename Callable>
class ScopeFail;

namespace detail {

template <template <typename> class ScopeExitT, typename Callable, typename T>
using make_scope_exit_any_return_type =
    typename std::conditional<std::is_lvalue_reference<T>::value,
                              ScopeExitT<Callable&>,
                              ScopeExitT<Callable>>::type;

template <typename Callable, typename T>
using make_scope_exit_return_type =
    make_scope_exit_any_return_type<ScopeExit, Callable, T>;

template <typename Callable, typename T>
using make_scope_success_return_type =
    make_scope_exit_any_return_type<ScopeSuccess, Callable, T>;

template <typename Callable, typename T>
using make_scope_fail_return_type =
    make_scope_exit_any_return_type<ScopeFail, Callable, T>;

} // namespace detail

template <typename Callable>
class ScopeExit {
public:
    template <typename Fn>
    explicit ScopeExit(Fn&& on_exit) noexcept
        : on_exit_{std::is_lvalue_reference<decltype(on_exit)>::value
                       ? on_exit
                       : std::forward<Callable>(on_exit)} {
        static_assert(
            std::is_convertible<Callable, std::function<void()>>::value,
            "The given object must be callable with no arguments");
    }

    ScopeExit(const ScopeExit&) = delete;
    ScopeExit(ScopeExit&& other) noexcept
        : on_exit_{std::move(other.on_exit_)},
          call_on_exit_{other.call_on_exit_} {
        other.release();
    }

    ScopeExit& operator=(const ScopeExit&) = default;
    ScopeExit& operator=(ScopeExit&&) = delete;

    ~ScopeExit() {
        if (call_on_exit_) {
            on_exit_();
        }
    }

    void release() {
        call_on_exit_ = false;
    }

private:
    Callable on_exit_;
    bool call_on_exit_{true};
};

template <typename Callable>
auto scope_exit(Callable&& on_exit)
    -> detail::make_scope_exit_return_type<Callable, decltype(on_exit)> {
    using callable =
        detail::make_scope_exit_return_type<Callable, decltype(on_exit)>;
    return callable{std::forward<Callable>(on_exit)};
}

// deduction guide for ScopeExit
template <typename Callable>
ScopeExit(Callable&& callable) -> ScopeExit<typename std::conditional<
    std::is_lvalue_reference<decltype(callable)>::value, Callable&,
    Callable>::type>;

template <typename Callable>
class ScopeSuccess {
public:
    template <typename Fn>
    explicit ScopeSuccess(Fn&& on_exit) noexcept
        : on_exit_{std::forward<Fn>(on_exit)} {
    }

    ScopeSuccess(ScopeSuccess&&) noexcept = default;

    ~ScopeSuccess() {
        if (std::uncaught_exceptions() > 0) {
            on_exit_.release();
        }
    }

private:
    ScopeExit<Callable> on_exit_;
};

template <typename Callable>
auto scope_success(Callable&& on_exit)
    -> detail::make_scope_success_return_type<Callable, decltype(on_exit)> {
    using callable =
        detail::make_scope_success_return_type<Callable, decltype(on_exit)>;
    return callable{std::forward<Callable>(on_exit)};
}

// deduction guide for ScopeSuccess
template <typename Callable>
ScopeSuccess(Callable&& callable) -> ScopeSuccess<typename std::conditional<
    std::is_lvalue_reference<decltype(callable)>::value, Callable&,
    Callable>::type>;

template <typename Callable>
class ScopeFail {
public:
    template <typename Fn>
    explicit ScopeFail(Fn&& on_exit) noexcept
        : on_exit_{std::forward<Fn>(on_exit)} {
    }

    ScopeFail(ScopeFail&&) noexcept = default;

    ~ScopeFail() {
        if (std::uncaught_exceptions() == 0) {
            on_exit_.release();
        }
    }

private:
    ScopeExit<Callable> on_exit_;
};

template <typename Callable>
auto scope_fail(Callable&& on_exit)
    -> detail::make_scope_fail_return_type<Callable, decltype(on_exit)> {
    using callable =
        detail::make_scope_fail_return_type<Callable, decltype(on_exit)>;
    return callable{std::forward<Callable>(on_exit)};
}

// deduction guide for ScopeExit
template <typename Callable>
ScopeFail(Callable&& callable) -> ScopeFail<typename std::conditional<
    std::is_lvalue_reference<decltype(callable)>::value, Callable&,
    Callable>::type>;

} // namespace pid