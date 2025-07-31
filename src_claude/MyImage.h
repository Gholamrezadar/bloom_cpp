#pragma once
#include <raylib.h>
#include <vector>

class MyImage {
public:
    int width;
    int height;
    int channels;
    
    // Constructors
    MyImage(const char* path);
    MyImage(int width, int height, int channels);
    
    // Copy constructor and assignment operator for proper memory management
    MyImage(const MyImage& other);
    MyImage& operator=(const MyImage& other);
    
    // Move constructor and assignment operator for efficiency
    MyImage(MyImage&& other) noexcept;
    MyImage& operator=(MyImage&& other) noexcept;
    
    ~MyImage();
    
    // Fast pixel access - inline for maximum performance
    inline double GetPixel(int x, int y, int channel) const {
        return data_[y * width * channels + x * channels + channel];
    }
    
    inline void SetPixel(int x, int y, int channel, double value) {
        data_[y * width * channels + x * channels + channel] = value;
    }
    
    // Direct data access for optimized operations
    inline const double* GetRawData() const { return data_; }
    inline double* GetRawData() { return data_; }
    
    void Save(const char* filename);
    
private:
    const char* path;
    Image image_;
    double* data_;  // Flat array for cache-friendly access
    
    int GetChannelCount_(int format);
    void AllocateData();
    void DeallocateData();
};