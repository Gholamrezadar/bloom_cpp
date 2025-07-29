#include "raylib.h"
#include <iostream>
#include <vector>

// void InvertImagePixels(unsigned char* pixels, int width, int height) {
//     for (int y = 0; y < height; ++y) {
//         for (int x = 0; x < width; ++x) {
//             int i = (y * width + x) * 4;
//             pixels[i + 0] = 255 - pixels[i + 0]; // R
//             pixels[i + 1] = 255 - pixels[i + 1]; // G
//             pixels[i + 2] = 255 - pixels[i + 2]; // B
//         }
//     }
// }

// Convert [0,255] byte RGBA to [0.0, 1.0] float RGBA
std::vector<float> ConvertToFloat(const unsigned char* pixels, int width, int height) {
    std::vector<float> result(width * height * 4);
    for (int i = 0; i < width * height * 4; ++i) {
        result[i] = static_cast<float>(pixels[i]) / 255.0f;
    }
    return result;
}

// Convert float RGBA [0.0, 1.0] back to byte RGBA [0,255]
std::vector<unsigned char> ConvertToByte(const std::vector<float>& floats) {
    std::vector<unsigned char> result(floats.size());
    for (size_t i = 0; i < floats.size(); ++i) {
        float clamped = std::min(std::max(floats[i], 0.0f), 1.0f);
        result[i] = static_cast<unsigned char>(clamped * 255.0f);
    }
    return result;
}

int GetIndex(int x, int y, int width) {
    return (y * width + x)*4;
}

std::vector<float> DownSampleImage(const std::vector<float>& img, int width, int height) {
    std::vector<float> result(width/2 * height/2 * 4);
    for(int y = 0; y < height/2; ++y) {
        for(int x = 0; x < width/2; ++x) {
            int i = GetIndex(x, y, width/2);
            float c = img[i + 0];

            result[i + 0] = c;
            result[i + 1] = c;
            result[i + 2] = c;
            result[i + 3] = 1.0f;
        }

    }
    return result;
}


void ProcessImage(std::vector<float>& img, int width, int height) {
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int i = GetIndex(x, y, width);
            float c = 1.0 - img[i + 0];

            img[i+0] = c; // R
            img[i+1] = c; // G
            img[i+2] = c; // B
        }
    }
}

int main() {
    const int screenWidth = 1280;
    const int screenHeight = 720/2;

    InitWindow(screenWidth, screenHeight, "Bloom C++");

    Image image = LoadImage("images/image.png");
    if (image.data == nullptr) {
        std::cerr << "Failed to load image!" << std::endl;
        CloseWindow();
        return 1;
    }

    int width = image.width;
    int height = image.height;

    Color* originalColors = LoadImageColors(image);
    std::vector<unsigned char> rawPixels(width * height * 4);

    for (int i = 0; i < width * height; ++i) {
        rawPixels[i * 4 + 0] = originalColors[i].r;
        rawPixels[i * 4 + 1] = originalColors[i].g;
        rawPixels[i * 4 + 2] = originalColors[i].b;
        rawPixels[i * 4 + 3] = originalColors[i].a;
    }

    // convert to float RGBA [0.0, 1.0]
    std::vector<float> float_image = ConvertToFloat(rawPixels.data(), width, height);
    ProcessImage(float_image, width, height);
    // convert back to byte RGBA [0,255]
    rawPixels = ConvertToByte(float_image);

    Image invertedImage = {
        rawPixels.data(), width, height, 1, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8
    };

    Texture2D originalTexture = LoadTextureFromImage(image);
    Texture2D invertedTexture = LoadTextureFromImage(invertedImage);

    // resize texture to fit screen
    float originalTextureAspectRatio = (float)originalTexture.width / (float)originalTexture.height;
    float invertedTextureAspectRatio = (float)invertedTexture.width / (float)invertedTexture.height;

    originalTexture.width = screenWidth / 2;
    originalTexture.height = originalTexture.width / originalTextureAspectRatio;
    invertedTexture.width = screenWidth / 2;
    invertedTexture.height = invertedTexture.width / invertedTextureAspectRatio;

    UnloadImage(image);
    UnloadImageColors(originalColors);

    SetTargetFPS(60);
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);

        // draw images side by side (half the screen width each)
        DrawTexture(originalTexture, 0, 0, WHITE);
        DrawText("Original", screenWidth / 4, 10, 20, WHITE);

        DrawTexture(invertedTexture, screenWidth / 2, 0, WHITE);
        DrawText("Inverted", 3*screenWidth / 4, 10, 20, DARKGRAY);


        EndDrawing();
    }

    UnloadTexture(originalTexture);
    UnloadTexture(invertedTexture);
    CloseWindow();
    return 0;
}
