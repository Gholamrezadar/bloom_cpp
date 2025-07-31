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

| Technique                                                 | Time      | Speedup |
| --------------------------------------------------------- | --------- | ------- |
| Baseline                                                  | 3.05 secs | 1x      |
| Flat vector instead of 3D vector                          | 1.30 secs | 2.3x    |
| Inline the GetPixel and SetPixel                          | 1.08 secs | 2.82x   |
| Flat Lerp instead of 3D Lerp                              | 1.06 secs | 2.87x   |
| Arithmetic Optimisations in BilinearTap                   | 0.98 secs | 3.11x   |
| static const double arrays instead of vectors for weights | 0.94 secs | 3.24x   |

### 1. Flat vector instead of 3D vector

This improved the performance from `3.05` secs to `1.30` secs per 1024x1024 image.

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

### 2. Inline GetPixel and SetPixel

GetPixel and SetPixel are called very often, let's reduce the number of function calls.

This improved the performance from `1.30` secs to `1.08` secs per 1024x1024 image.

### 3. Flat Lerp instead of 3D Lerp

This didn't help that much.

From

```cpp
MyImage Lerp(const MyImage& a, const MyImage& b, double t) {
    MyImage result(a.width, a.height, a.channels);
    for (int i = 0; i < a.height; ++i) {
        for (int j = 0; j < a.width; ++j) {
            for (int ch = 0; ch < a.channels; ++ch) {
                result.SetPixel(j, i, ch, a.GetPixel(j, i, ch) * (1.0 - t) + b.GetPixel(j, i, ch) * t);
            }
        }
    }
    return result;
}
```

To

```cpp
MyImage Lerp(const MyImage& a, const MyImage& b, double t) {
    MyImage result(a.width, a.height, a.channels);
    for(int i=0; i<a.data.size(); ++i) {
        result.data[i] = a.data[i] * (1.0 - t) + b.data[i] * t;
    }
    return result;
}
```

### 4. Micro-optimizations in BilinearTap

Reducing the number of multiplications and also fetching data for the four pixels directly from memory.

This improved the performance from `1.06` secs to `0.98` secs per 1024x1024 image.

Refer to BilinearTap() for more details.

### 5. static const double arrays instead of vectors for weights

Used `static const double` arrays instead of `std::vector` for weights.

This improved the performance from `0.98` secs to `0.94` secs per 1024x1024 image.
