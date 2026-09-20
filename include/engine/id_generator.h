#pragma once

#include <string_view>

namespace engine {

std::string generate_id(std::string_view module, std::string_view type,
                        std::string_view name, std::string_view existing_id);

}  // namespace engine
