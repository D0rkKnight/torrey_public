#include <iostream>
#include <png++/png.hpp>

int main() {
    try {
        png::image<png::rgba_pixel> image("example.png");

        std::cout << "Image width: " << image.get_width() << std::endl;
        std::cout << "Image height: " << image.get_height() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
