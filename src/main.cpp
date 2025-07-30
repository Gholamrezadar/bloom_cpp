#include <raylib.h>

#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <MyImage.h>

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

void ProcessImage(MyImage& image) {
    for (int y = 0; y < image.height; ++y) {
        for (int x = 0; x < image.width; ++x) {
            if (image.GetPixel(x, y, 0) > 0.5) {
                image.SetPixel(x, y, 1, 0.0);
                image.SetPixel(x, y, 2, 0.0);
            }
        }
    }
}

int main(int argc, const char** argv) {
    // load the image
    MyImage image("images/image2.png");
    std::cout << "Image width: " << image.width << std::endl;
    std::cout << "Image height: " << image.height << std::endl;
    std::cout << "Image channels: " << image.channels << std::endl;

    // process the image
    ProcessImage(image);

    // save the image
    image.Save("output.png");
    std::cout << "Image saved to output.png" << std::endl;

    // display the result
    DisplayImage("output.png");
    return 0;
}