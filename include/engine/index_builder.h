#pragma once

#include <string>
#include <string_view>

namespace engine {

class IndexBuilder {
public:
    void process(std::string_view input_path, std::string_view output_path);
};

}  // namespace engine
