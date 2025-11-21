

#pragma once
#include <memory_resource>
#include <cstddef>
#include <iterator>
#include <stdexcept>
#include <list>

class dynamic_list_memory_resource : public std::pmr::memory_resource {
private:
    struct block_info {
        void* ptr;
        std::size_t size;
        block_info(void* p, std::size_t s) : ptr(p), size(s) {}
    };
    
    std::list<block_info> allocated_blocks;
    std::list<block_info> free_blocks;

public:
    dynamic_list_memory_resource() = default;

    ~dynamic_list_memory_resource() override {
        for (const auto& block : allocated_blocks) {
            ::operator delete(block.ptr);
        }
        for (const auto& block : free_blocks) {
            ::operator delete(block.ptr);
        }
    }

protected:
    void* do_allocate(std::size_t bytes, std::size_t alignment) override {
        auto it = free_blocks.begin();
        while (it != free_blocks.end()) {
            if (it->size >= bytes) {
                void* result = it->ptr;
                std::size_t remaining_size = it->size - bytes;
                
                if (remaining_size > 0) {
                    void* free_part = static_cast<char*>(result) + bytes;
                    free_blocks.emplace_front(free_part, remaining_size);
                }
                
                free_blocks.erase(it);
                allocated_blocks.emplace_front(result, bytes);
                return result;
            }
            ++it;
        }
        
        void* new_block = ::operator new(bytes);
        allocated_blocks.emplace_front(new_block, bytes);
        return new_block;
    }

    void do_deallocate(void* p, std::size_t bytes, std::size_t alignment) override {
        auto it = allocated_blocks.begin();
        while (it != allocated_blocks.end()) {
            if (it->ptr == p) {
                free_blocks.splice(free_blocks.begin(), allocated_blocks, it);
                return;
            }
            ++it;
        }
        
        throw std::runtime_error("Attempt to deallocate non-allocated memory");
    }

    bool do_is_equal(const std::pmr::memory_resource& other) const noexcept override {
        return this == &other;
    }
};

template<typename T>
class dynamic_array {
private:
    std::pmr::polymorphic_allocator<T> allocator;
    T* data = nullptr;
    std::size_t capacity_ = 0;
    std::size_t size_ = 0;

    void resize_if_needed() {
        if (size_ >= capacity_) {
            std::size_t new_capacity = capacity_ == 0 ? 4 : capacity_ * 2;
            T* new_data = allocator.allocate(new_capacity);
            
            for (std::size_t i = 0; i < size_; ++i) {
                try {
                    std::construct_at(new_data + i, std::move_if_noexcept(data[i]));
                } catch (...) {
                    for (std::size_t j = 0; j < i; ++j) {
                        std::destroy_at(new_data + j);
                    }
                    allocator.deallocate(new_data, new_capacity);
                    throw;
                }
            }
            
            for (std::size_t i = 0; i < size_; ++i) {
                std::destroy_at(data + i);
            }
            if (data) {
                allocator.deallocate(data, capacity_);
            }
            
            data = new_data;
            capacity_ = new_capacity;
        }
    }

public:
    using value_type = T;
    using allocator_type = std::pmr::polymorphic_allocator<T>;

    class iterator {
    private:
        T* ptr;

    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = T*;
        using reference = T&;

        explicit iterator(T* p = nullptr) : ptr(p) {}

        reference operator*() const { return *ptr; }
        pointer operator->() const { return ptr; }

        iterator& operator++() {
            ++ptr;
            return *this;
        }

        iterator operator++(int) {
            iterator tmp = *this;
            ++ptr;
            return tmp;
        }

        bool operator==(const iterator& other) const { return ptr == other.ptr; }
        bool operator!=(const iterator& other) const { return ptr != other.ptr; }
    };

    explicit dynamic_array(std::pmr::memory_resource* mr = std::pmr::get_default_resource())
        : allocator(mr), data(nullptr), capacity_(0), size_(0) {}

    dynamic_array(std::size_t initial_size, std::pmr::memory_resource* mr = std::pmr::get_default_resource())
        : allocator(mr), capacity_(initial_size), size_(initial_size) {
        if (initial_size > 0) {
            data = allocator.allocate(capacity_);
            for (std::size_t i = 0; i < size_; ++i) {
                std::construct_at(data + i);
            }
        }
    }

    ~dynamic_array() {
        clear();
        if (data) {
            allocator.deallocate(data, capacity_);
        }
    }

    dynamic_array(const dynamic_array& other)
        : allocator(other.allocator), capacity_(other.capacity_), size_(other.size_) {
        if (capacity_ > 0) {
            data = allocator.allocate(capacity_);
            for (std::size_t i = 0; i < size_; ++i) {
                std::construct_at(data + i, other.data[i]);
            }
        }
    }

    dynamic_array(dynamic_array&& other) noexcept
        : allocator(std::move(other.allocator)), data(other.data), 
          capacity_(other.capacity_), size_(other.size_) {
        other.data = nullptr;
        other.capacity_ = 0;
        other.size_ = 0;
    }

    dynamic_array& operator=(const dynamic_array& other) {
        if (this != &other) {
            clear();
            if (capacity_ < other.size_) {
                if (data) {
                    allocator.deallocate(data, capacity_);
                }
                capacity_ = other.capacity_;
                data = allocator.allocate(capacity_);
            }
            size_ = other.size_;
            for (std::size_t i = 0; i < size_; ++i) {
                std::construct_at(data + i, other.data[i]);
            }
        }
        return *this;
    }

    dynamic_array& operator=(dynamic_array&& other) noexcept {
        if (this != &other) {
            clear();
            if (data) {
                allocator.deallocate(data, capacity_);
            }
            data = other.data;
            capacity_ = other.capacity_;
            size_ = other.size_;
            allocator = std::move(other.allocator);
            other.data = nullptr;
            other.capacity_ = 0;
            other.size_ = 0;
        }
        return *this;
    }

    T& operator[](std::size_t index) { return data[index]; }
    const T& operator[](std::size_t index) const { return data[index]; }

    T& at(std::size_t index) {
        if (index >= size_) throw std::out_of_range("Index out of range");
        return data[index];
    }

    const T& at(std::size_t index) const {
        if (index >= size_) throw std::out_of_range("Index out of range");
        return data[index];
    }

    T& front() { return data[0]; }
    const T& front() const { return data[0]; }
    T& back() { return data[size_ - 1]; }
    const T& back() const { return data[size_ - 1]; }

    iterator begin() { return iterator(data); }
    iterator end() { return iterator(data + size_); }

    std::size_t size() const { return size_; }
    std::size_t capacity() const { return capacity_; }
    bool empty() const { return size_ == 0; }

    void push_back(const T& value) {
        resize_if_needed();
        std::construct_at(data + size_, value);
        ++size_;
    }

    void push_back(T&& value) {
        resize_if_needed();
        std::construct_at(data + size_, std::move(value));
        ++size_;
    }

    template<typename... Args>
    void emplace_back(Args&&... args) {
        resize_if_needed();
        std::construct_at(data + size_, std::forward<Args>(args)...);
        ++size_;
    }

    void pop_back() {
        if (size_ > 0) {
            --size_;
            std::destroy_at(data + size_);
        }
    }

    void clear() {
        for (std::size_t i = 0; i < size_; ++i) {
            std::destroy_at(data + i);
        }
        size_ = 0;
    }

    void resize(std::size_t new_size) {
        if (new_size > capacity_) {
            std::size_t new_capacity = std::max(capacity_ * 2, new_size);
            T* new_data = allocator.allocate(new_capacity);
            
            for (std::size_t i = 0; i < size_; ++i) {
                std::construct_at(new_data + i, std::move_if_noexcept(data[i]));
                std::destroy_at(data + i);
            }
            
            for (std::size_t i = size_; i < new_size; ++i) {
                std::construct_at(new_data + i);
            }
            
            if (data) {
                allocator.deallocate(data, capacity_);
            }
            
            data = new_data;
            capacity_ = new_capacity;
        } else if (new_size > size_) {
            for (std::size_t i = size_; i < new_size; ++i) {
                std::construct_at(data + i);
            }
        } else if (new_size < size_) {
            for (std::size_t i = new_size; i < size_; ++i) {
                std::destroy_at(data + i);
            }
        }
        
        size_ = new_size;
    }
};
