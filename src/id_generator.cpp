#include "engine/id_generator.h"
#include "engine/normalize.h"

namespace engine {

std::string generate_id(std::string_view module, std::string_view type,
                        std::string_view name, std::string_view existing_id) {
    if (!existing_id.empty()) {
        return std::string(existing_id);
    }
    std::string slug = slugify(name);
    std::string id;
    id.reserve(module.size() + type.size() + slug.size() + 2);
    id.append(module);
    id.push_back('-');
    id.append(type);
    id.push_back('-');
    id.append(slug);
    return id;
}

}  // namespace engine
