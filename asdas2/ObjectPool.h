// ObjectPool.h
#pragma once
#include <stack>
#include <mutex>
#include <cstring>
#include <memory>

template <typename T>
class ObjectPool {
public:
    std::shared_ptr<T> Allocate() {
        std::lock_guard<std::mutex> lock(mutex_);
        T* rawPtr;
        if (!pool_.empty()) {
            rawPtr = pool_.top();
            pool_.pop();
            std::memset(rawPtr, 0, sizeof(T));
        }
        else {
            rawPtr = new T();
        }

        return std::shared_ptr<T>(rawPtr, [this](T* ptr) {
            this->Release(ptr);
            });
    }

private:
    void Release(T* obj) {
        std::lock_guard<std::mutex> lock(mutex_);
        pool_.push(obj);
    }

    std::stack<T*> pool_;
    std::mutex mutex_;
};