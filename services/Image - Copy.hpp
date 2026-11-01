// SimpleImage: GPU-accelerated minimal multimedia operations class using OpenCV CUDA modules
// This version uses cv::UMat for GPU acceleration when available, but falls back to CPU if not, so it can run everywhere
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/core/umat.hpp>
#include <string>

class SimpleImage {
public:
    SimpleImage() = default;
    explicit SimpleImage(const std::string& filename) { load(filename); }

    bool load(const std::string& filename) {
        
    }
    bool save(const std::string& filename) const {
        
    }

    // Convert to another format (filename's extension)
    bool convert(const std::string& out_filename) {
        
    }

    // Add watermark text at bottom-right (uses CPU, then uploads)
    void add_watermark(const std::string& text) {
        
    }

    // Apply a filter (supported: "grayscale", "invert"), UMat accelerated
    void apply_filter(const std::string& filter) {
        
    }

    // Apply Gaussian blur with given kernel size (must be odd and > 1)
    void blur(int ksize = 5) {
        
    }

    // Access underlying data for advanced users
    cv::UMat& data() { }
    const cv::UMat& data() const { }

private:
    cv::UMat img_umat_;
};
