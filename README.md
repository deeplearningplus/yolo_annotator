# yolo_annotator
Yolo annotator created with the help of Copilot Claude 3.5 Sonnet


Compilation
```bash
git clone https://github.com/deeplearningplus/yolo_annotator
cd yolo_annotator

mkdir -p build && cd build && cmake .. && make
```

Usage
```bash
./build/yolo_annotator images obj.names
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




