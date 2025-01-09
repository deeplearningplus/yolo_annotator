# yolo_annotator
Yolo annotator created with the help of Copilot Claude 3.5 Sonnet


## Compilation
Make sure you have installed OpenCV.
```bash
git clone https://github.com/deeplearningplus/yolo_annotator
cd yolo_annotator

# 1. Compile it with g++
g++ -o yolo_annotator main.cpp `pkg-config --cflags --libs opencv` -std=c++17 -Wall -O3

# 2. Compile it with cmake and make
mkdir -p build && cd build && cmake .. && make && mv yolo_annotator ../
```

## Usage
```bash
./yolo_annotator images obj.names
```


#### Keyboard Shortcuts

Shortcut | Description | 
--- | --- |
<kbd>n, N</kbd> | Next image |
<kbd>p, P</kbd> | Previous image |
<kbd>c, C</kbd> | Change class |
<kbd>d, D</kbd> | Delete last box |
<kbd>j, J</kbd> | Jump to specific image |
<kbd>ESC</kbd> | Close application |




