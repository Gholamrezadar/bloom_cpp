#pragma once
#include <raylib.h>

#include <vector>

class MyImage {
   public:
    const char* path;
    int width = 0;
    int height = 0;
    int channels = 0;
    std::vector<double> data;

   public:
    MyImage(const char* path);
    MyImage(int width, int height, int channels);
    
    // Copy constructor and assignment operator for proper memory management
    MyImage(const MyImage& other);
    MyImage& operator=(const MyImage& other);
    
    // Move constructor and assignment operator for efficiency
    MyImage(MyImage&& other) noexcept;
    MyImage& operator=(MyImage&& other) noexcept;

    ~MyImage();

    inline const double GetPixel(int x, int y, int channel) const {
        return data[(y * width + x) * channels + channel];
    }

    inline void SetPixel(int x, int y, int channel, double value) {
        data[(y * width + x) * channels + channel] = value;
    }

    void Save(const char* filename);

   private:
    Image image_;  // Underlying Raylib Image

   private:
    int GetChannelCount_(int format);
};