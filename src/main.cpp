#include <raylib.h>

#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <MyImage.h>
#include <assert.h>
#include <chrono>

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

int min(int a, int b) {
    return (a < b) ? a : b;
}

double BilinearTap(const MyImage& image, double x, double y, int channel) {
    // returns the bilinear tap value at (x, y) for the given channel
    // x, y are normalized coordinates between 0.0 and 1.0
    int h = image.height;
    int w = image.width;

    // Map normalized coordinates to pixel coordinates
    double px = x * (w-1);
    double py = y * (h-1);

    int x0 = (int)px;
    int y0 = (int)py;
    int x1 = min(x0 + 1, w - 1);
    int y1 = min(y0 + 1, h - 1);

    // distances to the top-left corner
    double dx = px - x0;
    double dy = py - y0;

    // Retrieve the four neighboring pixels
    double top_left = image.GetPixel(x0, y0, channel);
    double top_right = image.GetPixel(x1, y0, channel);
    double bottom_left = image.GetPixel(x0, y1, channel);
    double bottom_right = image.GetPixel(x1, y1, channel);

    // Interpolate
    double top = (1-dx)*top_left + dx*top_right;
    double bottom = (1-dx)*bottom_left + dx*bottom_right;
    double result = (1-dy)*top + dy*bottom;

    return result;
}

MyImage Upsample(const MyImage& image) {
    int new_h = image.height * 2;
    int new_w = image.width * 2;
    int c = image.channels;
    MyImage upsampled(new_w, new_h, c);

    // 3x3 filter kernel offsets
    const std::vector<std::pair<double, double>> Coords = {
        {-1.0,  1.0}, { 0.0,  1.0}, { 1.0,  1.0},
        {-1.0,  0.0}, { 0.0,  0.0}, { 1.0,  0.0},
        {-1.0, -1.0}, { 0.0, -1.0}, { 1.0, -1.0}
    };

    const std::vector<double> Weights = {
        0.0625, 0.125,  0.0625,
        0.125,  0.25,   0.125,
        0.0625, 0.125,  0.0625
    };

    for (int i = 0; i < new_h; ++i) {
        for (int j = 0; j < new_w; ++j) {
            for (int ch = 0; ch < c; ++ch) {
                double acc = 0.0;
                for (int k = 0; k < Coords.size(); ++k) {
                    double ox = Coords[k].first;
                    double oy = Coords[k].second;
                    double weight = Weights[k];

                    // Normalized source coordinates
                    double x = (j + ox) / new_w;
                    double y = (i + oy) / new_h;

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
    const std::vector<std::pair<double,double>> Coords = {
        {-1.0,  1.0}, { 1.0,  1.0},
        {-1.0, -1.0}, { 1.0, -1.0},

        {-2.0,  2.0}, { 0.0,  2.0}, { 2.0,  2.0},
        {-2.0,  0.0}, { 0.0,  0.0}, { 2.0,  0.0},
        {-2.0, -2.0}, { 0.0, -2.0}, { 2.0, -2.0}
    };

    const std::vector<double> weights = {
        0.125, 0.125,
        0.125, 0.125,

        0.0555555, 0.0555555, 0.0555555,
        0.0555555, 0.0555555, 0.0555555,
        0.0555555, 0.0555555, 0.0555555
    };

    for (int i = 0; i < new_h; ++i) {
        for (int j = 0; j < new_w; ++j) {
            for (int ch = 0; ch < c; ++ch) {
                double acc = 0.0;
                for(int k = 0; k < Coords.size(); ++k) {
                    double ox = Coords[k].first;
                    double oy = Coords[k].second;
                    double weight = weights[k];

                    // Normalized coordinates in [0,1]
                    double x = (j + 0.5 + ox) / new_w;
                    double y = (i + 0.5 + oy) / new_h;

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
    for(int i=0; i<a.data.size(); ++i) {
        result.data[i] = a.data[i] * (1.0 - t) + b.data[i] * t;
    }
    return result;
}

void Bloom(MyImage& image, int samples = 8) {
    // Downsample stack
    std::vector<MyImage> downsampled_list;
    // Add the original image to the downsampled list, it makes the upsampling easier
    downsampled_list.push_back(image);
    MyImage temp = image;
    for(int i=0; i<samples; ++i) {
        MyImage downsampled = DownSample(temp);
        // std::cout << "Downsampled image " << i << " width: " << downsampled.width << " height: " << downsampled.height << std::endl;
        downsampled_list.push_back(downsampled);
        temp = downsampled;
        // temp.Save(("downsampled_" + std::to_string(i) + ".png").c_str());
    }

    // std::cout << std::endl;

    // Upsample stack
    double lerp_weight = 0.2;
    temp = downsampled_list[samples];
    for(int i=samples; i>0; --i) {
        MyImage upsampled = Upsample(temp);
        // std::cout << "Upsampled image " << i << " width: " << upsampled.width << " height: " << upsampled.height << std::endl;
        temp = Lerp(upsampled, downsampled_list[i-1], lerp_weight);
        // temp.Save(("upsampled_" + std::to_string(i) + ".png").c_str());
    }

    double mult = 6.0;
    // multiply every value by mult
    for (int y = 0; y < temp.height; ++y) {
        for (int x = 0; x < temp.width; ++x) {
            for (int c = 0; c < temp.channels; ++c) {
                temp.SetPixel(x, y, c, std::min(std::max(temp.GetPixel(x, y, c) * mult, 0.0), 1.0));
            }
        }
    }

    image = temp;
}

int main(int argc, const char** argv) {
    // load the image
    MyImage image("images/image2.png");
    // std::cout << "Image width: " << image.width << std::endl;
    // std::cout << "Image height: " << image.height << std::endl;
    // std::cout << "Image channels: " << image.channels << std::endl;

    BilinearTap(image, 0.5, 0.5, 0);
    // process the image
    // std::cout << "Before:" << image.GetPixel(512, 512, 0) << std::endl;
    
    // Measure time
    std::cout << "Performing Bloom...\n";
    auto start = std::chrono::high_resolution_clock::now();
    Bloom(image);
    auto end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Elapsed time: " << elapsed.count() << " seconds\n";
    // std::cout << "After:" << image.GetPixel(512, 512, 0) << std::endl;

    // save the image
    // image.Save("output.png");
    // std::cout << "Image saved to output.png" << std::endl;

    // display the result
    // DisplayImage("output.png");
    return 0;
}