#include <MyImage.h>
#include <assert.h>
#include <raylib.h>

#include <chrono>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

void DisplayImage(const char* path) {
    int WindowWidth = 700;
    int WindowHeight = 700;
    InitWindow(WindowWidth, WindowHeight, "Bloom C++");
    SetTargetFPS(60);

    // Load and resize the image
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

double BilinearTap(const MyImage& image, const double x, const double y, const int channel) {
    // returns the bilinear tap value at (x, y) for the given channel
    // x, y are normalized coordinates between 0.0 and 1.0
    int w = image.width - 1;
    int h = image.height - 1;

    // Map normalized coordinates to pixel coordinates
    const double px = x * w;
    const double py = y * h;

    const int x0 = (int)px;
    const int y0 = (int)py;
    int x1 = std::min(x0 + 1, w);
    int y1 = std::min(y0 + 1, h);

    // distances to the top-left corner
    double dx = px - x0;
    double dy = py - y0;

    // Retrieve the four neighboring pixels
    // double top_left = image.GetPixel(x0, y0, channel);
    // double top_right = image.GetPixel(x1, y0, channel);
    // double bottom_left = image.GetPixel(x0, y1, channel);
    // double bottom_right = image.GetPixel(x1, y1, channel);

    // Direct memory access for speed
    int channels = image.channels;
    int stride = image.width * channels;
    const double* row0 = image.data.data() + y0 * stride;
    const double* row1 = image.data.data() + y1 * stride;

    const int x0_times_channels = x0 * channels;
    const int x1_times_channels = x1 * channels;
    double top_left = row0[x0_times_channels + channel];
    double top_right = row0[x1_times_channels + channel];
    double bottom_left = row1[x0_times_channels + channel];
    double bottom_right = row1[x1_times_channels + channel];

    // Interpolate
    // double top = (1 - dx) * top_left + dx * top_right;
    // double bottom = (1 - dx) * bottom_left + dx * bottom_right;
    // double result = (1 - dy) * top + dy * bottom;
    // return result;

    // Optimized interpolation (Fewer multiplications)
    double top = top_left + dx * (top_right - top_left);
    double bottom = bottom_left + dx * (bottom_right - bottom_left);
    return top + dy * (bottom - top);
}

MyImage Upsample(const MyImage& image) {
    int new_h = image.height * 2;
    int new_w = image.width * 2;
    int c = image.channels;
    MyImage upsampled(new_w, new_h, c);

    // 3x3 filter kernel offsets
    static const double Coords[9][2] = {
        {-1.0, 1.0}, {0.0, 1.0}, {1.0, 1.0}, {-1.0, 0.0}, {0.0, 0.0}, {1.0, 0.0}, {-1.0, -1.0}, {0.0, -1.0}, {1.0, -1.0}};

    static const double Weights[9] = {
        0.0625, 0.125, 0.0625,
        0.125, 0.25, 0.125,
        0.0625, 0.125, 0.0625};

    // To avoid divisions in the inner-loop
    double inv_new_w = 1.0 / new_w;
    double inv_new_h = 1.0 / new_h;

    for (int i = 0; i < new_h; ++i) {
        for (int j = 0; j < new_w; ++j) {
            for (int ch = 0; ch < c; ++ch) {
                double acc = 0.0;
                for (int k = 0; k < 9; ++k) {
                    double ox = Coords[k][0];
                    double oy = Coords[k][1];
                    double weight = Weights[k];

                    // Normalized source coordinates
                    double x = (j + ox) * inv_new_w;
                    double y = (i + oy) * inv_new_h;

                    // Clamp
                    x = std::min(std::max(x, 0.0), 1.0);
                    y = std::min(std::max(y, 0.0), 1.0);

                    double value = BilinearTap(image, x, y, ch);
                    acc += value * weight;
                }

                upsampled.SetPixel(j, i, ch, acc);
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

    // Coords offsets
    static const double Coords[13][2] = {
        {-1.0, 1.0},
        {1.0, 1.0},
        {-1.0, -1.0},
        {1.0, -1.0},

        {-2.0, 2.0},
        {0.0, 2.0},
        {2.0, 2.0},
        {-2.0, 0.0},
        {0.0, 0.0},
        {2.0, 0.0},
        {-2.0, -2.0},
        {0.0, -2.0},
        {2.0, -2.0}};

    static const double weights[13] = {
        0.125, 0.125,
        0.125, 0.125,

        0.0555555, 0.0555555, 0.0555555,
        0.0555555, 0.0555555, 0.0555555,
        0.0555555, 0.0555555, 0.0555555};
    
    // To avoid divisions in the inner-loop
    double inv_new_w = 1.0 / new_w;
    double inv_new_h = 1.0 / new_h;

    for (int i = 0; i < new_h; ++i) {
        for (int j = 0; j < new_w; ++j) {
            for (int ch = 0; ch < c; ++ch) {
                double acc = 0.0;
                double j_plus_half = j + 0.5;
                double i_plus_half = i + 0.5;
                for (int k = 0; k < 13; ++k) {
                    double ox = Coords[k][0];
                    double oy = Coords[k][1];
                    double weight = weights[k];

                    // Normalized coordinates in [0,1]
                    double x = (j_plus_half + ox) * inv_new_w;
                    double y = (i_plus_half + oy) * inv_new_h;

                    // Clamp
                    x = std::min(std::max(x, 0.0), 1.0);
                    y = std::min(std::max(y, 0.0), 1.0);

                    double value = BilinearTap(image, x, y, ch);
                    acc += value * weight;
                }
                downsampled.SetPixel(j, i, ch, acc);
            }
        }
    }
    return downsampled;
}

MyImage Lerp(const MyImage& a, const MyImage& b, double t) {
    assert(a.width == b.width && a.height == b.height && a.channels == b.channels);
    MyImage result(a.width, a.height, a.channels);
    for (int i = 0; i < a.data.size(); ++i) {
        result.data[i] = a.data[i] * (1.0 - t) + b.data[i] * t;
    }
    return result;
}

void Bloom(MyImage& image, int samples = 8) {
    // Downsample stack
    std::vector<MyImage> downsampled_list;
    downsampled_list.reserve(samples + 1);

    // Add the original image
    downsampled_list.emplace_back(std::move(image));
    
    // Downsample chain
    for (int i = 0; i < samples; ++i) {
        downsampled_list.emplace_back(DownSample(downsampled_list.back()));
    }

    // Upsample stack
    constexpr double lerp_weight = 0.2;
    MyImage temp = std::move(downsampled_list[samples]);
    
    for (int i = samples; i > 0; --i) {
        MyImage upsampled = Upsample(temp);
        temp = Lerp(upsampled, downsampled_list[i-1], lerp_weight);
    }
    
    
    // Final multiplication and clamping
    constexpr double mult = 6.0;
    double* data = temp.data.data();
    int total_elements = temp.width * temp.height * temp.channels;
    
    for (int i = 0; i < total_elements; ++i) {
        data[i] = std::max(0.0, std::min(data[i] * mult, 1.0));
    }
    
    // image = temp;
    image = std::move(temp);
}

int main(int argc, const char** argv) {
    // load the image
    MyImage image("images/image2.png");

    // Measure time
    std::cout << "Performing Bloom...\n";
    auto start = std::chrono::high_resolution_clock::now();
    Bloom(image);
    auto end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Elapsed time: " << elapsed.count() << " seconds\n";

    // Save and display the result
    // image.Save("output.png");
    // DisplayImage("output.png");
    return 0;
}