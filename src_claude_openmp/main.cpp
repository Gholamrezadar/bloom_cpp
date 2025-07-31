#include <raylib.h>
#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <MyImage.h>
#include <assert.h>
#include <chrono>
#include <algorithm>
#include <omp.h>

void DisplayImage(const char* path) {
    int WindowWidth = 700;
    int WindowHeight = 700;
    InitWindow(WindowWidth, WindowHeight, "Bloom C++");
    SetTargetFPS(60);
    
    Image image = LoadImage(path);
    ImageResize(&image, WindowWidth, WindowHeight);
    Texture2D texture = LoadTextureFromImage(image);
    
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawTexture(texture, 0, 0, WHITE);
        EndDrawing();
    }
    
    UnloadImage(image);
    CloseWindow();
}

// Optimized bilinear sampling with direct data access
inline double BilinearTap(const double* data, int width, int height, int channels, 
                         double x, double y, int channel) {
    // Map normalized coordinates to pixel coordinates
    double px = x * (width - 1);
    double py = y * (height - 1);
    
    int x0 = (int)px;
    int y0 = (int)py;
    int x1 = std::min(x0 + 1, width - 1);
    int y1 = std::min(y0 + 1, height - 1);
    
    double dx = px - x0;
    double dy = py - y0;
    
    // Direct memory access for speed
    int stride = width * channels;
    const double* row0 = data + y0 * stride;
    const double* row1 = data + y1 * stride;
    
    double top_left = row0[x0 * channels + channel];
    double top_right = row0[x1 * channels + channel];
    double bottom_left = row1[x0 * channels + channel];
    double bottom_right = row1[x1 * channels + channel];
    
    // Optimized interpolation
    double top = top_left + dx * (top_right - top_left);
    double bottom = bottom_left + dx * (bottom_right - bottom_left);
    return top + dy * (bottom - top);
}

MyImage Upsample(const MyImage& image) {
    int new_h = image.height * 2;
    int new_w = image.width * 2;
    int c = image.channels;
    
    MyImage upsampled(new_w, new_h, c);
    
    // Pre-computed kernel weights and offsets
    static const double coords[9][2] = {
        {-1.0,  1.0}, { 0.0,  1.0}, { 1.0,  1.0},
        {-1.0,  0.0}, { 0.0,  0.0}, { 1.0,  0.0},
        {-1.0, -1.0}, { 0.0, -1.0}, { 1.0, -1.0}
    };
    
    static const double weights[9] = {
        0.0625, 0.125,  0.0625,
        0.125,  0.25,   0.125,
        0.0625, 0.125,  0.0625
    };
    
    const double* src_data = image.GetRawData();
    double* dst_data = upsampled.GetRawData();
    
    int dst_stride = new_w * c;
    double inv_new_w = 1.0 / new_w;
    double inv_new_h = 1.0 / new_h;
    
    // Parallel processing of rows with OpenMP
    #pragma omp parallel for schedule(dynamic, 16) if(new_h > 64)
    for (int i = 0; i < new_h; ++i) {
        double* dst_row = dst_data + i * dst_stride;
        
        for (int j = 0; j < new_w; ++j) {
            double* dst_pixel = dst_row + j * c;
            
            for (int ch = 0; ch < c; ++ch) {
                double acc = 0.0;
                
                // Unrolled loop for better performance
                for (int k = 0; k < 9; ++k) {
                    double x = (j + coords[k][0]) * inv_new_w;
                    double y = (i + coords[k][1]) * inv_new_h;
                    
                    // Clamp coordinates
                    x = std::max(0.0, std::min(x, 1.0));
                    y = std::max(0.0, std::min(y, 1.0));
                    
                    acc += BilinearTap(src_data, image.width, image.height, c, x, y, ch) * weights[k];
                }
                
                dst_pixel[ch] = acc;
            }
        }
    }
    
    return upsampled;
}

MyImage DownSample(const MyImage& image) {
    int new_h = image.height / 2;
    int new_w = image.width / 2;
    int c = image.channels;
    
    MyImage downsampled(new_w, new_h, c);
    
    // Pre-computed coordinates and weights
    static const double coords[13][2] = {
        {-1.0,  1.0}, { 1.0,  1.0},
        {-1.0, -1.0}, { 1.0, -1.0},
        {-2.0,  2.0}, { 0.0,  2.0}, { 2.0,  2.0},
        {-2.0,  0.0}, { 0.0,  0.0}, { 2.0,  0.0},
        {-2.0, -2.0}, { 0.0, -2.0}, { 2.0, -2.0}
    };
    
    static const double weights[13] = {
        0.125, 0.125, 0.125, 0.125,
        0.0555555, 0.0555555, 0.0555555,
        0.0555555, 0.0555555, 0.0555555,
        0.0555555, 0.0555555, 0.0555555
    };
    
    const double* src_data = image.GetRawData();
    double* dst_data = downsampled.GetRawData();
    
    int dst_stride = new_w * c;
    double inv_new_w = 1.0 / new_w;
    double inv_new_h = 1.0 / new_h;
    
    // Parallel processing with dynamic scheduling for load balancing
    #pragma omp parallel for schedule(dynamic, 8) if(new_h > 32)
    for (int i = 0; i < new_h; ++i) {
        double* dst_row = dst_data + i * dst_stride;
        
        for (int j = 0; j < new_w; ++j) {
            double* dst_pixel = dst_row + j * c;
            
            for (int ch = 0; ch < c; ++ch) {
                double acc = 0.0;
                
                for (int k = 0; k < 13; ++k) {
                    double x = (j + 0.5 + coords[k][0]) * inv_new_w;
                    double y = (i + 0.5 + coords[k][1]) * inv_new_h;
                    
                    x = std::max(0.0, std::min(x, 1.0));
                    y = std::max(0.0, std::min(y, 1.0));
                    
                    acc += BilinearTap(src_data, image.width, image.height, c, x, y, ch) * weights[k];
                }
                
                dst_pixel[ch] = acc;
            }
        }
    }
    
    return downsampled;
}

MyImage Lerp(const MyImage& a, const MyImage& b, double t) {
    assert(a.width == b.width && a.height == b.height && a.channels == b.channels);
    
    MyImage result(a.width, a.height, a.channels);
    
    const double* a_data = a.GetRawData();
    const double* b_data = b.GetRawData();
    double* result_data = result.GetRawData();
    
    int total_elements = a.width * a.height * a.channels;
    double inv_t = 1.0 - t;
    
    // Highly parallel vectorized operation
    #pragma omp parallel for simd schedule(static) if(total_elements > 10000)
    for (int i = 0; i < total_elements; ++i) {
        result_data[i] = a_data[i] * inv_t + b_data[i] * t;
    }
    
    return result;
}

void Bloom(MyImage& image, int samples = 8) {
    std::cout << "Using " << omp_get_max_threads() << " threads for parallel processing\n";
    
    // Reserve space to avoid reallocations
    std::vector<MyImage> downsampled_list;
    downsampled_list.reserve(samples + 1);
    
    // Add the original image
    downsampled_list.emplace_back(std::move(image));
    
    // Downsample chain - Sequential due to dependencies
    std::cout << "Downsampling chain...\n";
    for (int i = 0; i < samples; ++i) {
        downsampled_list.emplace_back(DownSample(downsampled_list.back()));
    }
    
    // Upsample chain with lerping - Sequential due to dependencies
    std::cout << "Upsampling and blending...\n";
    constexpr double lerp_weight = 0.2;
    MyImage temp = std::move(downsampled_list[samples]);
    
    for (int i = samples; i > 0; --i) {
        MyImage upsampled = Upsample(temp);
        temp = Lerp(upsampled, downsampled_list[i-1], lerp_weight);
    }
    
    // Final multiplication and clamping - Highly parallel
    std::cout << "Final processing...\n";
    constexpr double mult = 6.0;
    double* data = temp.GetRawData();
    int total_elements = temp.width * temp.height * temp.channels;
    
    #pragma omp parallel for simd schedule(static) if(total_elements > 10000)
    for (int i = 0; i < total_elements; ++i) {
        data[i] = std::max(0.0, std::min(data[i] * mult, 1.0));
    }
    
    image = std::move(temp);
}

// Function to set optimal number of threads based on system and image size
void SetOptimalThreadCount(int image_size) {
    int max_threads = omp_get_max_threads();
    int optimal_threads;
    
    // Adjust thread count based on image size and system capabilities
    if (image_size < 512 * 512) {
        optimal_threads = std::min(4, max_threads);  // Small images don't benefit from many threads
    } else if (image_size < 1024 * 1024) {
        optimal_threads = std::min(8, max_threads);
    } else {
        optimal_threads = max_threads;  // Large images can use all threads
    }
    
    omp_set_num_threads(optimal_threads);
    std::cout << "Set thread count to: " << optimal_threads << " (max available: " << max_threads << ")\n";
}

int main(int argc, const char** argv) {
    MyImage image("images\\image2.png");
    
    // Set optimal thread count based on image size
    SetOptimalThreadCount(image.width * image.height);
    
    std::cout << "Image: " << image.width << "x" << image.height << " (" << image.channels << " channels)\n";
    std::cout << "Performing Bloom...\n";
    
    auto start = std::chrono::high_resolution_clock::now();
    Bloom(image);
    auto end = std::chrono::high_resolution_clock::now();
    
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Elapsed time: " << elapsed.count() << " seconds\n";
    
    image.Save("output.png");
    DisplayImage("output.png");
    
    return 0;
}