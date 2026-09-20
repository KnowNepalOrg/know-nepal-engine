#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace engine {

std::string normalize(std::string_view input);
std::vector<std::string> tokenize(std::string_view normalized);
std::string slugify(std::string_view input);

}  // namespace engine
