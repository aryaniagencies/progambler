// SimpleImage: Minimal multimedia image ops class using stb_image/stb_image_write for I/O.
// Hardware acceleration is not natively supported in stb, but SIMD is used internally if enabled.
// This class is fully cross-platform. Optionally uses std::vector for storage.
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "stb/stb_image.h"
#include "stb/stb_image_write.h"
#include <string>
#include <vector>
#include <algorithm>
#include <cstring>

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

    // Add watermark text at bottom-right. (Stub: full text rendering not supported in stb.)
    void add_watermark(const std::string& text) {
        // Minimal watermark: just a white pixel pattern at bottom-right for demonstration
        if (width_ < 20 || height_ < 10) return;
        int x0 = width_ - 18, y0 = height_ - 8;
        for (int y = 0; y < 8; ++y) {
            for (int x = 0; x < 16; ++x) {
                if ((x + y) % 2 == 0) {
                    int idx = (y0 + y) * width_ * channels_ + (x0 + x) * channels_;
                    for (int c = 0; c < channels_; ++c)
                        img_data_[idx + c] = 255;  // white pixel
                }
            }
        }
    }

    // Apply a filter (supported: "grayscale", "invert")
    void apply_filter(const std::string& filter) {
        if (filter == "grayscale" && channels_ >= 3) {
            for (size_t i = 0; i < img_data_.size(); i += channels_) {
                uint8_t r = img_data_[i], g = img_data_[i+1], b = img_data_[i+2];
                uint8_t gray = static_cast<uint8_t>(0.299 * r + 0.587 * g + 0.114 * b);
                img_data_[i] = img_data_[i+1] = img_data_[i+2] = gray;
            }
        } else if (filter == "invert") {
            for (auto& px : img_data_)
                px = 255 - px;
        }
    }

    // Simple box blur, single pass
    void blur(int ksize = 5) {
        if (ksize < 3 || ksize % 2 == 0) return;
        std::vector<uint8_t> out(img_data_);
        int halfk = ksize / 2;
        for (int y = 0; y < height_; ++y) {
            for (int x = 0; x < width_; ++x) {
                for (int c = 0; c < channels_; ++c) {
                    int sum = 0, count = 0;
                    for (int ky = -halfk; ky <= halfk; ++ky) {
                        for (int kx = -halfk; kx <= halfk; ++kx) {
                            int nx = x + kx, ny = y + ky;
                            if (nx >= 0 && nx < width_ && ny >= 0 && ny < height_) {
                                sum += img_data_[(ny * width_ + nx) * channels_ + c];
                                ++count;
                            }
                        }
                    }
                    out[(y * width_ + x) * channels_ + c] = static_cast<uint8_t>(sum / count);
                }
            }
        }
        img_data_.swap(out);
    }

    // Access underlying data for advanced users
    std::vector<uint8_t>& data() { return img_data_; }
    const std::vector<uint8_t>& data() const { return img_data_; }

private:
    void free_data() { img_data_.clear(); width_ = height_ = channels_ = 0; }
    std::string get_ext(const std::string& filename) const {
        auto dot = filename.find_last_of('.');
        if (dot == std::string::npos) return "";
        std::string ext = filename.substr(dot + 1);
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        return ext;
    }

    std::vector<uint8_t> img_data_;
    int width_ = 0, height_ = 0, channels_ = 0;
};
