# CoD Bloom Effect on CPU

Implementing a bloom effect on CPU for learning purposes.

## Result

![output](output.png)

## Performance Comparison

Measured on an Intel i7-12700 CPU

| Implementation    | time      | fps       |
| ----------------- | --------- | --------- |
| Python            | 39.5 secs | 0.025 fps |
| C++ (Debug)       | 2.7 secs  | 0.37 fps  |
| C++ (Release)     | 0.68 secs | 1.47 fps  |
| C++ (Multithread) | ? secs    | ? fps     |
| OpenGL            | ? secs    | ? fps     |

## Performance Improvements

I tried different approaches to improve performance, here I list them for future reference.

| Technique                        | Time     | fps   |
| -------------------------------- | -------- | ----- |
| Baseline                         | 0.5 secs | - fps |
| Flat vector instead of 3D vector | 0.5 secs | - fps |

### 1. Flat vector instead of 3D vector

Old:

```cpp
std::vector<std::vector<std::vector<double>>> pixels;
```

Now:

```cpp
std::vector<double> pixels;
```
