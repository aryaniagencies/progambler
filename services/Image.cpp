// SimpleImage: Minimal multimedia image ops class using stb_image/stb_image_write for I/O, with SIMD used internally if enabled.
// This class is fully cross-platform. Uses std::vector for storage.
// Note: stb_image does NOT support hardware acceleration directly, but internally uses SIMD on supported platforms if compiled in.
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "include/stb_image.h"
#include "include/stb_image_write.h"
#include <string>
#include <vector>
#include <algorithm>
#include <cstring>
#include <cctype>

class SimpleImage {
public:
    SimpleImage() = default;
    explicit SimpleImage(const std::string& filename) { load(filename); }

    bool load(const std::string& filename) {
        free_data();
        int n;
        unsigned char* data = stbi_load(filename.c_str(), &width_, &height_, &channels_, 0);
        if (!data) return false;
        img_data_.assign(data, data + width_ * height_ * channels_);
        stbi_image_free(data);
        return true;
    }

    bool save(const std::string& filename) const {
        if (img_data_.empty()) return false;
        std::string ext = get_ext(filename);
        int success = 0;
        if (ext == "png") {
            success = stbi_write_png(filename.c_str(), width_, height_, channels_, img_data_.data(), width_ * channels_);
        } else if (ext == "jpg" || ext == "jpeg") {
            success = stbi_write_jpg(filename.c_str(), width_, height_, channels_, img_data_.data(), 95);
        } else if (ext == "bmp") {
            success = stbi_write_bmp(filename.c_str(), width_, height_, channels_, img_data_.data());
        } else if (ext == "tga") {
            success = stbi_write_tga(filename.c_str(), width_, height_, channels_, img_data_.data());
        }
        return success != 0;
    }

    // Convert to another format (filename's extension)
    bool convert(const std::string& out_filename) {
        return save(out_filename);
    }

    // Add watermark text at bottom-right (stub: stb does not support text rendering; so we add a checkered white block)
    void add_watermark(const std::string& text) {
        if (width_ < 20 || height_ < 10) return;
        int block_width = 16, block_height = 8;
        int x0 = width_ - block_width - 2, y0 = height_ - block_height - 2;
        for (int y = 0; y < block_height; ++y) {
            for (int x = 0; x < block_width; ++x) {
                if ((x + y) % 2 == 0) {
                    int idx = (y0 + y) * width_ * channels_ + (x0 + x) * channels_;
                    for (int c = 0; c < channels_; ++c)
                        img_data_[idx + c] = 255;
                }
            }
        }
    }

    // Apply a filter (supported: "grayscale", "invert")
    void apply_filter(const std::string& filter) {
        if (img_data_.empty()) return;
        std::string f = filter;
        std::transform(f.begin(), f.end(), f.begin(), [](unsigned char c){ return std::tolower(c); });
        if (f == "grayscale" && channels_ >= 3) {
            for (int y = 0; y < height_; ++y) {
                for (int x = 0; x < width_; ++x) {
                    int idx = (y * width_ + x) * channels_;
                    unsigned char r = img_data_[idx];
                    unsigned char g = img_data_[idx+1];
                    unsigned char b = img_data_[idx+2];
                    unsigned char gray = static_cast<unsigned char>(0.299*r + 0.587*g + 0.114*b);
                    img_data_[idx] = img_data_[idx+1] = img_data_[idx+2] = gray;
                }
            }
        } else if (f == "invert") {
            for (size_t i = 0; i < img_data_.size(); ++i)
                img_data_[i] = 255 - img_data_[i];
        }
        // More filters can be added.
    }

    // Apply Gaussian blur with naive box filter (since stb does not support it natively)
    void blur(int ksize = 5) {
        if (img_data_.empty() || ksize < 3 || ksize % 2 == 0) return;
        std::vector<unsigned char> out(img_data_.size(), 0);
        int radius = ksize / 2;
        for (int y = 0; y < height_; ++y) {
            for (int x = 0; x < width_; ++x) {
                for (int c = 0; c < channels_; ++c) {
                    int sum = 0, count = 0;
                    for (int dy = -radius; dy <= radius; ++dy) {
                        for (int dx = -radius; dx <= radius; ++dx) {
                            int nx = x + dx, ny = y + dy;
                            if (nx >= 0 && nx < width_ && ny >= 0 && ny < height_) {
                                int nidx = (ny * width_ + nx) * channels_ + c;
                                sum += img_data_[nidx];
                                ++count;
                            }
                        }
                    }
                    int idx = (y * width_ + x) * channels_ + c;
                    out[idx] = static_cast<unsigned char>(sum / count);
                }
            }
        }
        img_data_ = std::move(out);
    }

    // Access underlying data for advanced users
    std::vector<unsigned char>& data() { return img_data_; }
    const std::vector<unsigned char>& data() const { return img_data_; }

private:
    std::vector<unsigned char> img_data_;
    int width_ = 0, height_ = 0, channels_ = 0;

    std::string get_ext(const std::string& filename) const {
        // returns lowercase extension (without .)
        auto dot = filename.find_last_of('.');
        if (dot == std::string::npos) return "";
        std::string ext = filename.substr(dot + 1);
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        return ext;
    }
    void free_data() {
        img_data_.clear();
        width_ = height_ = channels_ = 0;
    }
};