#pragma once
#include <raylib.h>
#include <vector>

class MyImage {
   public:
    const char* path;
    int width = 0;
    int height = 0;
    int channels = 0;
    std::vector<std::vector<std::vector<double>>> data;  // Note: double values for pixels between 0.0 and 1.0

   public:
    MyImage(const char* path);
    ~MyImage();
    
    double GetPixel(int x, int y, int channel);
    void SetPixel(int x, int y, int channel, double value);
    void Save(const char* filename);

   private:
    Image image_;  // Underlying Raylib Image

   private:
    int GetChannelCount_(int format);
};