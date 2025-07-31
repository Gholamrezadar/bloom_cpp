# CoD Bloom Effect on CPU

Implementing a bloom effect on CPU for learning purposes.

## Result

![output](output.png)

## Performance Comparison

Measured on an Intel i7-12700 CPU

| Implementation         | time      | fps       |
| ---------------------- | --------- | --------- |
| Python                 | 39.5 secs | 0.025 fps |
| C++ (Debug)            | 2.7 secs  | 0.37 fps  |
| C++ (Release)          | 0.68 secs | 1.47 fps  |
| C++ (Improved+Release) | ? secs    | ? fps     |
| C++ (Multithread)      | ? secs    | ? fps     |
| OpenGL                 | ? secs    | ? fps     |

## Performance Improvements

I tried different approaches to improve performance, here I list them for future reference. (CPU: i7-4930k)

| Technique                        | Time      | fps      |
| -------------------------------- | --------- | -------- |
| Baseline                         | 3.05 secs | 0.32 fps |
| Flat vector instead of 3D vector | 1.30 secs | 0.76 fps |

### 1. Flat vector instead of 3D vector

This improved the performance from 3.05 secs to 1.30 secs per 1024x1024 image.

From

```cpp
std::vector<std::vector<std::vector<double>>> pixels;
```

```cpp
double GetPixel(int x, int y, int channel) const{
    return data[y][x][channel];
}

void SetPixel(int x, int y, int channel, double value) {
    data[y][x][channel] = value;
}
```

To

```cpp
std::vector<double> pixels;
```

```cpp
double GetPixel(int x, int y, int channel) const{
    return data[y * width + x * channels + channel];
}
void SetPixel(int x, int y, int channel, double value) {
    data[y * width + x * channels + channel] = value;
}
```
