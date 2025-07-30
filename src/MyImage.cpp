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
    data = std::vector<std::vector<std::vector<double>>>(
        height,
        std::vector<std::vector<double>>(
            width,
            std::vector<double>(channels, 0.0)));

    Color* colors = LoadImageColors(image_);
    // fill the data Matrix with normalized (0.0-1.0) values of the image
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int index = y * width + x;
            data[y][x][0] = colors[index].r / 255.0;
            data[y][x][1] = colors[index].g / 255.0;
            data[y][x][2] = colors[index].b / 255.0;
            if (channels == 4) {
                data[y][x][3] = colors[index].a / 255.0;
            }
        }
    }
    UnloadImageColors(colors);
}

MyImage::~MyImage() {
    UnloadImage(image_);
}

double MyImage::GetPixel(int x, int y, int channel) {
    return data[y][x][channel];
}

void MyImage::SetPixel(int x, int y, int channel, double value) {
    data[y][x][channel] = value;
}

void MyImage::Save(const char* filename) {
    Color* colors = (Color*)malloc(width * height * sizeof(Color));

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int index = y * width + x;
            colors[index].r = (unsigned char)(data[y][x][0] * 255.0);
            colors[index].g = (unsigned char)(data[y][x][1] * 255.0);
            colors[index].b = (unsigned char)(data[y][x][2] * 255.0);
            colors[index].a = (channels == 4)
                                  ? (unsigned char)(data[y][x][3] * 255.0)
                                  : 255;
        }
    }

    // Copy to image_.data
    if (image_.format != PIXELFORMAT_UNCOMPRESSED_R8G8B8A8) {
        ImageFormat(&image_, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);
    }

    memcpy(image_.data, colors, width * height * sizeof(Color));
    free(colors);

    ExportImage(image_, filename);
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