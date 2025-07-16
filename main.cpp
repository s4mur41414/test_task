#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image/stb_image_write.h"

#include <iostream>
#include <string>
#include <stdexcept>

class Image {
private:
    int width = 0;
    int height = 0;
    int channels = 0;
    unsigned char* imageData = nullptr;
    std::string inputPath = "";
    std::string outputName = "";

    void validateImage() const {
        if (!imageData) {
            throw std::runtime_error("No image data loaded");
        }
    }

    // функция извлекающая название из пути для дефолтного названия
    void getOutputDefaultName() {
    size_t lastSlash = inputPath.find_last_of("/\\");
    size_t lastDot = inputPath.find_last_of('.');
    size_t start = (lastSlash == std::string::npos) ? 0 : lastSlash + 1;
    if (lastDot == std::string::npos || lastDot < start) {
        outputName = inputPath.substr(start);
    } else {
        outputName = inputPath.substr(start, lastDot - start);
    }
    if (outputName.empty()) {
        outputName = "output";
    }
}

public:
    // работаем по правилу трех
    Image() = default;
    ~Image() {
        freeImage();
    }
    Image(const Image&) = delete;
    Image& operator=(const Image&) = delete;

    void loadImage(const std::string& filePath) {
        imageData = stbi_load(filePath.c_str(), &width, &height, &channels, 0);
        if (!imageData) {
            throw std::runtime_error("Failed to load image: " + filePath);
        }
        
        inputPath = filePath;
        getOutputDefaultName();
    }

    void changingBrightness(int brightLevel) {
        validateImage();
        
        for (int i = 0; i < width * height * channels; i += channels) { //идем попиксельно
            for (int c = 0; c < 3; c++) { // обрабатываем только R, G, B, без канала A
                int newValue = imageData[i + c] + brightLevel;
                imageData[i + c] = static_cast<unsigned char> (newValue > 255 ? 255 : (newValue < 0 ? 0 : newValue) //проверяем границы и преобразуем значение обратно в unsigned char
                );
            }
        }
    }

    void saveImageAsPng(const std::string& customPath) {
        validateImage();
        
        std::string outputPath = customPath.empty() ? (outputName + "_different_brightness.png") : customPath;
        
        if (!stbi_write_png(outputPath.c_str(), width, height, channels, imageData, width * channels)) {
            throw std::runtime_error("Failed to save image: " + outputPath);
        }
        
        std::cout << "Image saved as: " << outputPath << std::endl;
    }

    void freeImage() {
        if (imageData) {
            stbi_image_free(imageData);
            imageData = nullptr;
            width = height = channels = 0;
        }
    }
    int getWidth() const { return width; }
    int getHeight() const { return height; }
    int getChannels() const { return channels; }
};

int getBrightness() {
    int level;
    while (true) {
        std::cout << "Enter brightness level (-255 to 255): ";
        if (std::cin >> level && level >= -255 && level <= 255) {
            return level;
        }
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cerr << "Invalid input.\n";
    }
}

int main() {
    try {
        Image img;
        std::string inputPath;
        std::cout << "Enter path to image: ";
        std::cin >> inputPath;
        img.loadImage(inputPath);
        std::cout << "Image loaded: " << img.getWidth() << "x" << img.getHeight() << ", channels: " << img.getChannels() << std::endl;
        
        int brightLevel = getBrightness();
        img.changingBrightness(brightLevel);
        std::string savePath = "";
        std::cout << "Enter output path (leave empty for default): ";
        std::cin.ignore();
        std::getline(std::cin, savePath);
        img.saveImageAsPng(savePath);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}