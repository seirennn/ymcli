#include "Queue.hpp"
#include <algorithm>
#include <chrono>
#include <numeric>

namespace ymcli {

Queue::Queue() : rng_(std::chrono::system_clock::now().time_since_epoch().count()) {}

void Queue::addTrack(const Track& track) {
    tracks_.push_back(track);
    if (shuffled_) {
        shuffle_order_.push_back(tracks_.size() - 1);
        if (current_index_ == -1) {
            current_index_ = 0;
        } else {
            // Swap the new element with a random element in the remainder of the array
            std::uniform_int_distribution<size_t> dist(current_index_ + 1, shuffle_order_.size() - 1);
            std::swap(shuffle_order_.back(), shuffle_order_[dist(rng_)]);
        }
    } else {
        if (current_index_ == -1) {
            current_index_ = 0;
        }
    }
}

void Queue::addTracks(const std::vector<Track>& tracks) {
    for (const auto& track : tracks) {
        addTrack(track);
    }
}

void Queue::removeTrack(size_t index) {
    if (index >= tracks_.size()) return;

    if (shuffled_) {
        // Find the logical index of this physical track
        auto it = std::find(shuffle_order_.begin(), shuffle_order_.end(), index);
        if (it != shuffle_order_.end()) {
            int logical_idx = std::distance(shuffle_order_.begin(), it);
            shuffle_order_.erase(it);
            
            // Adjust indices in shuffle_order_ that were > index
            for (auto& idx : shuffle_order_) {
                if (idx > index) idx--;
            }
            
            if (logical_idx < current_index_) {
                current_index_--;
            } else if (logical_idx == current_index_ && current_index_ >= (int)shuffle_order_.size()) {
                current_index_ = -1;
            }
        }
    } else {
        if ((int)index < current_index_) {
            current_index_--;
        } else if ((int)index == current_index_ && current_index_ >= (int)(tracks_.size() - 1)) {
            current_index_ = -1;
        }
    }
    
    tracks_.erase(tracks_.begin() + index);
    if (tracks_.empty()) {
        current_index_ = -1;
    }
}

void Queue::moveTrack(size_t from, size_t to) {
    if (from >= tracks_.size() || to >= tracks_.size() || from == to) return;

    if (shuffled_) {
        // For shuffled state, we just reorder the logical list
        size_t val = shuffle_order_[from];
        shuffle_order_.erase(shuffle_order_.begin() + from);
        shuffle_order_.insert(shuffle_order_.begin() + to, val);
        
        if (current_index_ == (int)from) {
            current_index_ = to;
        } else if (current_index_ > (int)from && current_index_ <= (int)to) {
            current_index_--;
        } else if (current_index_ >= (int)to && current_index_ < (int)from) {
            current_index_++;
        }
    } else {
        Track t = tracks_[from];
        tracks_.erase(tracks_.begin() + from);
        tracks_.insert(tracks_.begin() + to, t);

        if (current_index_ == (int)from) {
            current_index_ = to;
        } else if (current_index_ > (int)from && current_index_ <= (int)to) {
            current_index_--;
        } else if (current_index_ >= (int)to && current_index_ < (int)from) {
            current_index_++;
        }
    }
}

void Queue::clear() {
    tracks_.clear();
    shuffle_order_.clear();
    current_index_ = -1;
}

void Queue::clearExceptCurrent() {
    if (current_index_ < 0 || current_index_ >= (int)size()) {
        clear();
        return;
    }

    size_t actual_idx = resolveIndex(current_index_);
    Track current_track = tracks_[actual_idx];
    
    tracks_.clear();
    shuffle_order_.clear();
    
    tracks_.push_back(current_track);
    current_index_ = 0;
    if (shuffled_) {
        shuffle_order_.push_back(0);
    }
}

std::optional<Track> Queue::current() const {
    if (current_index_ < 0 || current_index_ >= (int)size()) return std::nullopt;
    return tracks_[resolveIndex(current_index_)];
}

std::optional<Track> Queue::next() {
    if (tracks_.empty()) return std::nullopt;

    if (repeat_ == RepeatMode::One) {
        return current();
    }

    current_index_++;
    
    if (current_index_ >= (int)size()) {
        if (repeat_ == RepeatMode::All) {
            current_index_ = 0;
            if (shuffled_) regenerateShuffle();
        } else {
            current_index_ = -1;
            return std::nullopt;
        }
    }

    return current();
}

std::optional<Track> Queue::previous() {
    if (tracks_.empty()) return std::nullopt;

    if (repeat_ == RepeatMode::One) {
        return current();
    }

    current_index_--;
    
    if (current_index_ < 0) {
        if (repeat_ == RepeatMode::All) {
            current_index_ = (int)size() - 1;
        } else {
            current_index_ = 0;
        }
    }

    return current();
}

void Queue::jumpTo(size_t index) {
    if (index < size()) {
        current_index_ = index;
    }
}

void Queue::toggleShuffle() {
    if (tracks_.empty()) {
        shuffled_ = !shuffled_;
        return;
    }

    if (!shuffled_) {
        shuffled_ = true;
        size_t actual_current = current_index_;
        regenerateShuffle();
        
        // Put actual_current at logical index 0
        if (actual_current >= 0 && actual_current < tracks_.size()) {
            auto it = std::find(shuffle_order_.begin(), shuffle_order_.end(), actual_current);
            if (it != shuffle_order_.end()) {
                std::swap(shuffle_order_[0], *it);
            }
        }
        current_index_ = 0;
    } else {
        shuffled_ = false;
        if (current_index_ >= 0 && current_index_ < (int)shuffle_order_.size()) {
            current_index_ = shuffle_order_[current_index_];
        }
        shuffle_order_.clear();
    }
}

void Queue::cycleRepeat() {
    switch (repeat_) {
        case RepeatMode::Off: repeat_ = RepeatMode::All; break;
        case RepeatMode::All: repeat_ = RepeatMode::One; break;
        case RepeatMode::One: repeat_ = RepeatMode::Off; break;
    }
}

bool Queue::isShuffled() const {
    return shuffled_;
}

RepeatMode Queue::getRepeatMode() const {
    return repeat_;
}

const std::vector<Track>& Queue::tracks() const {
    return tracks_;
}

size_t Queue::currentIndex() const {
    return current_index_ >= 0 ? current_index_ : 0;
}

size_t Queue::size() const {
    return tracks_.size();
}

bool Queue::empty() const {
    return tracks_.empty();
}

void Queue::playNow(const std::vector<Track>& tracks, size_t startIndex) {
    clear();
    addTracks(tracks);
    if (startIndex < size()) {
        current_index_ = startIndex;
    } else {
        current_index_ = tracks.empty() ? -1 : 0;
    }
}

size_t Queue::resolveIndex(size_t logical) const {
    if (shuffled_ && logical < shuffle_order_.size()) {
        return shuffle_order_[logical];
    }
    return logical;
}

void Queue::regenerateShuffle() {
    shuffle_order_.resize(tracks_.size());
    std::iota(shuffle_order_.begin(), shuffle_order_.end(), 0);
    std::shuffle(shuffle_order_.begin(), shuffle_order_.end(), rng_);
}

} // namespace ymcli
