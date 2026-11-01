// Minimal C++ networking module using cpp-httplib & OpenSSL with basic retry capability.
// Adapted from cpp-httplib basic HTTPS client examples.

#include <httplib.h>
#include <openssl/ssl.h>
#include <string>
#include <chrono>
#include <thread>
#include <optional>

class HttpClient {
public:
    HttpClient(const std::string& host, int port = 443) : client_(host.c_str(), port)
    {
        // Required for HTTPS
        client_.enable_server_certificate_verification(true);
        // (Optional) set connection, read and write timeouts
        client_.set_connection_timeout(5, 0); // 5 seconds
        client_.set_read_timeout(20, 0);      // 20 seconds
        client_.set_write_timeout(20, 0);     // 20 seconds
    }

    // Returns response body if status==200, nullopt otherwise.
    // Retries up to retry_count times (total tries = retry_count+1)
    std::optional<std::string> get(const std::string& path, int retry_count = 2, int retry_delay_ms = 1000)
    {
        for (int i = 0; i <= retry_count; ++i) {
            auto res = client_.Get(path.c_str());
            if (res && res->status == 200) {
                return res->body;
            }
            if (i < retry_count) {
                std::this_thread::sleep_for(std::chrono::milliseconds(retry_delay_ms));
            }
        }
        return std::nullopt;
    }

    // Returns response body if status==200, nullopt otherwise.
    std::optional<std::string> post(const std::string& path, const std::string& body, const std::string& content_type = "application/json", int retry_count = 2, int retry_delay_ms = 1000)
    {
        for (int i = 0; i <= retry_count; ++i) {
            auto res = client_.Post(path.c_str(), body, content_type.c_str());
            if (res && res->status == 200) {
                return res->body;
            }
            if (i < retry_count) {
                std::this_thread::sleep_for(std::chrono::milliseconds(retry_delay_ms));
            }
        }
        return std::nullopt;
    }

private:
    httplib::SSLClient client_;
};