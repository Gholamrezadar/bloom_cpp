#include <raylib.h>
#include <MyImage.h>
#include <cstring>
#include <cstdlib>
#include <algorithm>

MyImage::MyImage(const char* path) : path(path), data_(nullptr) {
    image_ = LoadImage(path);
    width = image_.width;
    height = image_.height;
    channels = GetChannelCount_(image_.format);
    
    AllocateData();
    
    // Load image data efficiently
    Color* colors = LoadImageColors(image_);
    
    // Parallel memory layout conversion
    const double inv255 = 1.0 / 255.0;
    int total_pixels = width * height;
    
    #pragma omp parallel for schedule(static) if(total_pixels > 50000)
    for (int pixel = 0; pixel < total_pixels; ++pixel) {
        const Color& c = colors[pixel];
        double* dst = data_ + pixel * channels;
        
        dst[0] = c.r * inv255;
        dst[1] = c.g * inv255;
        dst[2] = c.b * inv255;
        if (channels == 4) {
            dst[3] = c.a * inv255;
        }
    }
    
    UnloadImageColors(colors);
}

MyImage::MyImage(int width, int height, int channels) 
    : width(width), height(height), channels(channels), path(nullptr), data_(nullptr) {
    AllocateData();
    // Initialize to zero
    std::fill(data_, data_ + width * height * channels, 0.0);
}

// Copy constructor
MyImage::MyImage(const MyImage& other) 
    : width(other.width), height(other.height), channels(other.channels), 
      path(other.path), image_(other.image_), data_(nullptr) {
    AllocateData();
    int total_size = width * height * channels;
    std::memcpy(data_, other.data_, total_size * sizeof(double));
}

// Copy assignment
MyImage& MyImage::operator=(const MyImage& other) {
    if (this != &other) {
        DeallocateData();
        
        width = other.width;
        height = other.height;
        channels = other.channels;
        path = other.path;
        image_ = other.image_;
        
        AllocateData();
        int total_size = width * height * channels;
        std::memcpy(data_, other.data_, total_size * sizeof(double));
    }
    return *this;
}

// Move constructor
MyImage::MyImage(MyImage&& other) noexcept
    : width(other.width), height(other.height), channels(other.channels),
      path(other.path), image_(other.image_), data_(other.data_) {
    other.data_ = nullptr;
    other.width = other.height = other.channels = 0;
}

// Move assignment
MyImage& MyImage::operator=(MyImage&& other) noexcept {
    if (this != &other) {
        DeallocateData();
        
        width = other.width;
        height = other.height;
        channels = other.channels;
        path = other.path;
        image_ = other.image_;
        data_ = other.data_;
        
        other.data_ = nullptr;
        other.width = other.height = other.channels = 0;
    }
    return *this;
}

MyImage::~MyImage() {
    DeallocateData();
}

void MyImage::AllocateData() {
    if (width > 0 && height > 0 && channels > 0) {
        data_ = new double[width * height * channels];
    }
}

void MyImage::DeallocateData() {
    delete[] data_;
    data_ = nullptr;
}

void MyImage::Save(const char* filename) {
    Color* colors = new Color[width * height];
    
    int total_pixels = width * height;
    
    // Parallel conversion from double to Color
    #pragma omp parallel for schedule(static) if(total_pixels > 50000)
    for (int pixel = 0; pixel < total_pixels; ++pixel) {
        const double* src = data_ + pixel * channels;
        Color& c = colors[pixel];
        
        // Clamp and convert to byte values
        c.r = (unsigned char)std::max(0.0, std::min(src[0] * 255.0, 255.0));
        c.g = (unsigned char)std::max(0.0, std::min(src[1] * 255.0, 255.0));
        c.b = (unsigned char)std::max(0.0, std::min(src[2] * 255.0, 255.0));
        c.a = (channels == 4) 
            ? (unsigned char)std::max(0.0, std::min(src[3] * 255.0, 255.0))
            : 255;
    }
    
    Image output = {
        .data = colors,
        .width = width,
        .height = height,
        .mipmaps = 1,
        .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8
    };
    
    ExportImage(output, filename);
    delete[] colors;
}

int MyImage::GetChannelCount_(int format) {
    switch (format) {
        case PIXELFORMAT_UNCOMPRESSED_GRAYSCALE:
            return 1;
        case PIXELFORMAT_UNCOMPRESSED_GRAY_ALPHA:
            return 2;
        case PIXELFORMAT_UNCOMPRESSED_R8G8B8:
            return 3;
        case PIXELFORMAT_UNCOMPRESSED_R8G8B8A8:
            return 4;
        default:
            return 3;  // Default to RGB
    }
}