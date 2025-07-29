#include "raylib.h"
#include <iostream>
#include <vector>

void InvertImagePixels(unsigned char* pixels, int width, int height) {
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int i = (y * width + x) * 4;
            pixels[i + 0] = 255 - pixels[i + 0]; // R
            pixels[i + 1] = 255 - pixels[i + 1]; // G
            pixels[i + 2] = 255 - pixels[i + 2]; // B
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

    InvertImagePixels(rawPixels.data(), width, height);

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
