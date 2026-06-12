#pragma once
#include <vector>
#include <cstddef>
namespace csopesy {

// Fixed-capacity FIFO of floats with contiguous storage for ImGui::PlotLines.
class CpuHistory {
public:
    explicit CpuHistory(std::size_t capacity) : cap_(capacity) {
        buf_.reserve(capacity);
    }
    void push(float v) {
        if (buf_.size() < cap_) buf_.push_back(v);
        else { buf_.erase(buf_.begin()); buf_.push_back(v); }
    }
    const float* data() const { return buf_.data(); }
    std::size_t size() const { return buf_.size(); }
    std::size_t capacity() const { return cap_; }
private:
    std::size_t cap_;
    std::vector<float> buf_;
};
}
