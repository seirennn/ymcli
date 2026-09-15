#pragma once

#include <string>
#include <unordered_map>
#include <mutex>
#include <optional>

class ImageCache {
public:
    static ImageCache& getInstance() {
        static ImageCache instance;
        return instance;
    }

    void set(const std::string& video_id, const std::string& url);
    std::optional<std::string> get(const std::string& video_id);

private:
    ImageCache() = default;
    ~ImageCache() = default;
    
    std::unordered_map<std::string, std::string> cache_;
    std::mutex mutex_;
};
