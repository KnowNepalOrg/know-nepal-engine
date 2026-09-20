#include "engine/index_builder.h"

#include <cstdlib>
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: index-builder <input.json> <output.json>\n";
        return 1;
    }
    try {
        engine::IndexBuilder builder;
        builder.process(argv[1], argv[2]);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
