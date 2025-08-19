#pragma once

#include "../main_ble/include/gap.hpp"

#include <optional>
#include <string_view>
#include <cassert>
#include <cstring>

namespace plant {

class UserDefinedName {
public:
    UserDefinedName() {
        name_.fill(0);
    }

    void set(std::string_view name) {
        assert(name.size() <= sizeof(name_));
        std::copy(std::begin(name), std::end(name), std::begin(name_));
        std::fill(std::begin(name_) + name.size(), std::end(name_), 0);
    }

    template <typename T, std::size_t N>
    void set(const std::array<T, N>& name) {
        const auto length = std::min(name.size(), name_.size());
        std::copy_n(std::begin(name), length, std::begin(name_));
        // std::fill(std::begin(name_) + length, std::end(name_), 0);
    }

    std::string_view get() const {
        return std::string_view{name_.data(), std::strlen(name_.data())};
    }

    auto as_array() const {
        return name_;
    }

private:
    std::array<char, ManufacturerData::Part2::name_max_size> name_;
};

std::optional<UserDefinedName> read_user_defined_name_from_storage();
void write_user_defined_name_to_storage(const UserDefinedName& name);

} // namespace plant