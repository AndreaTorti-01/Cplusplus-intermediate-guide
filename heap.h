// I'LL IMPLEMENT A SIMPLE FUCKING (min) HEAP! IN C++!
#include <iostream>
#include <vector>
#include <mutex>

template <typename T>
class Heap
{
private:
    // inside, it's just an array. a vector, for comfortable reallocation.
    std::vector<T> vec_;
    std::mutex mtx_; // to protect vec_

    // indexing helpers
    static size_t Parent(size_t i) { return (i - 1) / 2; }
    static size_t LeftChild(size_t i) { return i * 2 + 1; }
    static size_t RightChild(size_t i) { return i * 2 + 2; }

    void AddImpl(const T &elem)
    {
        std::lock_guard<std::mutex> lock(mtx_);
        vec_.push_back(elem);
        // heapify up
        size_t currPos = vec_.size() - 1;
        // while parent is bigger, swap with it.
        while (currPos != 0 && vec_.at(Parent(currPos)) > vec_.at(currPos))
        {
            std::swap(vec_.at(Parent(currPos)), vec_.at(currPos));
            currPos = Parent(currPos);
        }
    }

public:
    // default constructors - vector will construct and destruct itself
    Heap() = default;
    ~Heap() = default;

    // 2 - copy constructor
    Heap(const Heap &other) : vec_(other.vec_) {} // copy the other vector

    // 3 - move constuctor
    Heap(Heap &&other) noexcept : vec_(std::move(other.vec_)) {} // move the other vector. no need to then nullify it because it's intelligent.

    // 4 - copy assignment operator
    Heap &operator=(const Heap &other)
    {
        if (this != &other)
        {
            // vector auto-copies
            vec_ = other.vec_;
        }
        return *this;
    }

    // 5 - move assignment operator
    Heap &operator=(Heap &&other) noexcept
    {
        if (this != &other)
        {
            // no need to free myself since vector frees itself when moved
            vec_ = std::move(other.vec_);
            // no need to nullptr/zero the other since vector does that itself
        }
        return *this;
    }

    // constructor by copy of vector
    Heap(const std::vector<T> &vec) : vec_(vec) {}
    // constr by move of vector
    Heap(std::vector<T> &&vec) : vec_(std::move(vec)) {}

    size_t Size() const
    {
        std::lock_guard<std::mutex> lock(const_cast<std::mutex &>(mtx_));
        return vec_.size();
    }
    void Reserve(size_t size) { vec_.reserve(size); }
    void Add(T &&elem) { AddImpl(std::move(elem)); }
    void Add(const T &elem) { AddImpl(elem); }
    T Pop()
    {
        std::lock_guard<std::mutex> lock(mtx_);
        T value = vec_.at(0);
        // swap with last
        std::swap(vec_.at(0), vec_.at(vec_.size() - 1));
        // trim
        vec_.resize(vec_.size() - 1);
        // heapify down
        size_t curr = 0;
        size_t n = vec_.size();
        while (true)
        {
            size_t l = LeftChild(curr);
            size_t r = RightChild(curr);
            size_t smallest = curr;

            if (l < n && vec_.at(l) < vec_.at(smallest))
                smallest = l;
            if (r < n && vec_.at(r) < vec_.at(smallest))
                smallest = r;

            if (smallest == curr)
                break; // heap property restored

            std::swap(vec_.at(curr), vec_.at(smallest));
            curr = smallest;
        }

        return value;
    }
};