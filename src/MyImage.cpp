#include <raylib.h>
#include <MyImage.h>
#include <cstring>
#include <cstdlib>

MyImage::MyImage(const char* path) : path(path) {
    image_ = LoadImage(path);
    width = image_.width;
    height = image_.height;
    channels = GetChannelCount_(image_.format);

    // Initialize the data Matrix
    data = std::vector<double>(width * height * channels, 0.0);
    Color* colors = LoadImageColors(image_);
    // fill the data Matrix with normalized (0.0-1.0) values of the image
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int index = y * width + x;
            SetPixel(x, y, 0, colors[index].r / 255.0);
            SetPixel(x, y, 1, colors[index].g / 255.0);
            SetPixel(x, y, 2, colors[index].b / 255.0);
            if (channels == 4) {
                SetPixel(x, y, 3, colors[index].a / 255.0);
            }
        }
    }
    UnloadImageColors(colors);
}

MyImage::MyImage(int width, int height, int channels) : width(width), height(height), channels(channels) {
    data = std::vector<double>(width * height * channels, 0.0);
    // NOTE: bad! where is the image_ initilization?
}

MyImage::~MyImage() {
    // UnloadImage(image_);
}

// Move constructor
MyImage::MyImage(MyImage&& other) noexcept
    : width(other.width), height(other.height), channels(other.channels),
      path(other.path), image_(other.image_), data(other.data) {
    other.width = other.height = other.channels = 0;
}

// Move assignment
MyImage& MyImage::operator=(MyImage&& other) noexcept {
    if (this != &other) {
        
        width = other.width;
        height = other.height;
        channels = other.channels;
        path = other.path;
        image_ = other.image_;
        data = other.data;
        
        other.width = other.height = other.channels = 0;
    }
    return *this;
}

void MyImage::Save(const char* filename) {
    Color* colors = (Color*)malloc(width * height * sizeof(Color));

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int index = y * width + x;
            colors[index].r = (unsigned char)(GetPixel(x, y, 0) * 255.0);
            colors[index].g = (unsigned char)(GetPixel(x, y, 1) * 255.0);
            colors[index].b = (unsigned char)(GetPixel(x, y, 2) * 255.0);
            colors[index].a = (channels == 4)
                                  ? (unsigned char)(GetPixel(x, y, 3) * 255.0)
                                  : 255;
        }
    }

    image_ = GenImageColor(width, height, BLANK);
    image_.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
    image_.width = width;
    image_.height = height;
    image_.mipmaps = 1;
    image_.data = colors;
    
    ExportImage(image_, filename);
    free(colors);
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
            return 0;  // or throw error if unknown
    }
}