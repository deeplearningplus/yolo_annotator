#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <filesystem>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

namespace fs = std::filesystem;

struct BoundingBox {
    cv::Rect rect;
    int classId;
};

class YoloAnnotator {
private:
    std::vector<std::string> imageFiles;
    std::vector<std::string> classNames;
    size_t currentImageIndex = 0;
    cv::Mat currentImage;
    cv::Mat displayImage;
    std::string windowName = "YOLO Annotator";
    std::vector<BoundingBox> currentBoxes;
    bool isDrawing = false;
    cv::Point startPoint;
    cv::Point endPoint;
    int currentClass = 0;
	size_t processedCount = 0;
    
public:
    YoloAnnotator(const std::string& directoryPath, const std::string& classesFile) {
        loadClassNames(classesFile);
        loadImagesFromDirectory(directoryPath);
        cv::namedWindow(windowName, cv::WINDOW_NORMAL);
        cv::setMouseCallback(windowName, mouseCallback, this);
    }

    void loadClassNames(const std::string& classesFile) {
        std::ifstream file(classesFile);
        std::string line;
        while (std::getline(file, line)) {
            if (!line.empty()) {
                classNames.push_back(line);
            }
        }
        if (classNames.empty()) {
            throw std::runtime_error("No classes loaded from classes file");
        }
        std::cout << "Loaded " << classNames.size() << " classes" << std::endl;
    }

	// Load all supported image files from the directory
    void loadImagesFromDirectory(const std::string& directoryPath) {
        try {
            for (const auto& entry : fs::directory_iterator(directoryPath)) {
                std::string extension = entry.path().extension().string();
                // Convert extension to lowercase for comparison
                std::transform(extension.begin(), extension.end(), 
                             extension.begin(), ::tolower);
                
                // Check if the file is an image
                if (extension == ".jpg" || extension == ".jpeg" || 
                    extension == ".png" || extension == ".bmp") {
                    imageFiles.push_back(entry.path().string());
                }
            }
            
            if (imageFiles.empty()) {
                throw std::runtime_error("No images found in the specified directory");
            }
            
            std::cout << "Found " << imageFiles.size() << " images." << std::endl;
            
            // Sort the files to ensure consistent ordering
            std::sort(imageFiles.begin(), imageFiles.end());
            
            // Load the first image
            loadCurrentImage();
        }
        catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            throw;
        }
    }

    void loadCurrentImage() {
        if (currentImageIndex < imageFiles.size()) {
            currentImage = cv::imread(imageFiles[currentImageIndex]);
            if (currentImage.empty()) {
                std::cerr << "Error: Could not load image " 
                         << imageFiles[currentImageIndex] << std::endl;
                return;
            }
            
            // Load existing annotations if they exist
            loadAnnotations();
            updateDisplay();
        }
    }

    void updateDisplay() {
        currentImage.copyTo(displayImage);
        
        // Draw all existing boxes
        for (const auto& box : currentBoxes) {
            drawBox(displayImage, box.rect, box.classId);
        }
        
        // Draw the current box being created
        if (isDrawing) {
            cv::rectangle(displayImage, cv::Rect(startPoint, endPoint), 
                         cv::Scalar(0, 255, 0), 2);
        }

        // Display class info
        std::string className = "Current Class: " + classNames[currentClass];
        cv::putText(displayImage, className, cv::Point(10, 30),
                    cv::FONT_HERSHEY_SIMPLEX, 0.75, cv::Scalar(0, 255, 0), 2);

        cv::imshow(windowName, displayImage);
    }

    void drawBox(cv::Mat& img, const cv::Rect& rect, int classId) {
        cv::rectangle(img, rect, cv::Scalar(0, 255, 0), 2);
        std::string label = classNames[classId];
        cv::putText(img, label, rect.tl() + cv::Point(0, -5),
                    cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 0), 2);
    }

    static void mouseCallback(int event, int x, int y, int flags, void* userdata) {
        YoloAnnotator* annotator = reinterpret_cast<YoloAnnotator*>(userdata);
        annotator->onMouse(event, x, y, flags);
    }

    void onMouse(int event, int x, int y, int flags) {
        switch(event) {
            case cv::EVENT_LBUTTONDOWN:
                startPoint = cv::Point(x, y);
                endPoint = startPoint;
                isDrawing = true;
                break;

            case cv::EVENT_MOUSEMOVE:
                if (isDrawing) {
                    endPoint = cv::Point(x, y);
                    updateDisplay();
                }
                break;

            case cv::EVENT_LBUTTONUP:
                if (isDrawing) {
                    endPoint = cv::Point(x, y);
                    isDrawing = false;
                    
                    // Create and add the bounding box
                    cv::Rect rect(cv::Point(std::min(startPoint.x, endPoint.x),
                                          std::min(startPoint.y, endPoint.y)),
                                cv::Point(std::max(startPoint.x, endPoint.x),
                                          std::max(startPoint.y, endPoint.y)));
                    
                    if (rect.width > 5 && rect.height > 5) {
                        currentBoxes.push_back({rect, currentClass});
                        saveAnnotations();
                        updateDisplay();
                    }
                }
                break;
        }
    }

    void saveAnnotations() {
        std::string imagePath = imageFiles[currentImageIndex];
        std::string annotationPath = imagePath.substr(0, imagePath.find_last_of('.')) + ".txt";
        
        std::ofstream file(annotationPath);
        if (!file.is_open()) {
            std::cerr << "Error: Could not create annotation file" << std::endl;
            return;
        }

        // Write annotations if they exist
        if (!currentBoxes.empty()) {

            float imageWidth = static_cast<float>(currentImage.cols);
            float imageHeight = static_cast<float>(currentImage.rows);

            for (const auto& box : currentBoxes) {
                // Convert to YOLO format (normalized coordinates)
                float x = (box.rect.x + box.rect.width/2.0f) / imageWidth;
                float y = (box.rect.y + box.rect.height/2.0f) / imageHeight;
                float w = box.rect.width / imageWidth;
                float h = box.rect.height / imageHeight;

                file << box.classId << " " << x << " " << y << " " << w << " " << h << "\n";
            }
		}
		file.close();
		// Update and display progress
        processedCount++;
        std::cout << "Processed " << processedCount << "/" << imageFiles.size()
                  << " images" << std::endl;
    }

    void loadAnnotations() {
        currentBoxes.clear();
        std::string imagePath = imageFiles[currentImageIndex];
        std::string annotationPath = imagePath.substr(0, imagePath.find_last_of('.')) + ".txt";
        
        std::ifstream file(annotationPath);
        if (!file.is_open()) {
            return; // No annotations exist yet
        }

        std::string line;
        float imageWidth = static_cast<float>(currentImage.cols);
        float imageHeight = static_cast<float>(currentImage.rows);

        while (std::getline(file, line)) {
            std::istringstream iss(line);
            int classId;
            float x, y, w, h;
            
            if (iss >> classId >> x >> y >> w >> h) {
                // Convert from YOLO format to pixel coordinates
                int pixelX = static_cast<int>((x - w/2) * imageWidth);
                int pixelY = static_cast<int>((y - h/2) * imageHeight);
                int pixelW = static_cast<int>(w * imageWidth);
                int pixelH = static_cast<int>(h * imageHeight);

                currentBoxes.push_back({cv::Rect(pixelX, pixelY, pixelW, pixelH), classId});
            }
        }
    }

    void jumpToImage(int index) {
        if (index >= 0 && index < imageFiles.size()) {
            saveAnnotations(); // Save current annotations before switching
            currentImageIndex = index;
            loadCurrentImage();
            std::cout << "Jumped to image " << index + 1 << " of " << imageFiles.size() << std::endl;
        } else {
            std::cerr << "Invalid index. Please specify an index between 1 and " << imageFiles.size() << std::endl;
        }
    }


    void run() {
        while (true) {
            int key = cv::waitKey(1) & 0xFF;
            
            switch (key) {
                case 27: // ESC key to exit
                    return;
                
                case 'n': case 'N': // Next image
                    if (currentImageIndex < imageFiles.size() - 1) {
                        currentImageIndex++;
                        loadCurrentImage();
                    }
                    break;
                
                case 'p': case 'P': // Previous image
                    if (currentImageIndex > 0) {
                        currentImageIndex--;
                        loadCurrentImage();
                    }
                    break;
                
                case 'c': case 'C': // Change class
                    currentClass = (currentClass + 1) % classNames.size();
                    updateDisplay();
                    break;
                
                case 'd': case 'D': // Delete last box
                    if (!currentBoxes.empty()) {
                        currentBoxes.pop_back();
                        saveAnnotations();
                        updateDisplay();
                    }
                    break;

                case 'j': case 'J': // Jump to specific image
                    int index;
                    std::cout << "Enter image index (1 to " << imageFiles.size() << "): ";
                    std::cin >> index;
                    jumpToImage(index - 1);
                    break;
            }
        }
    }

    ~YoloAnnotator() {
        cv::destroyAllWindows();
    }
};

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cout << "Usage: " << argv[0] << " <directory_path> <classes_file>" << std::endl;
        return -1;
    }

    try {
        YoloAnnotator annotator(argv[1], argv[2]);
        annotator.run();
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }

    return 0;
}
