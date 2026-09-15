#include "ImageCache.hpp"

void ImageCache::set(const std::string& video_id, const std::string& url) {
    std::lock_guard<std::mutex> lock(mutex_);
    cache_[video_id] = url;
}

std::optional<std::string> ImageCache::get(const std::string& video_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = cache_.find(video_id);
    if (it != cache_.end()) {
        return it->second;
    }
    return std::nullopt;
}
