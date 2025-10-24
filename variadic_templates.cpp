// variadic_examples.cpp
// ================================================================
// Demonstrates two classic uses of C++ variadic templates:
//   1. A recursive adder function using parameter packs (...)
//   2. A toy "MyVector" class that inefficiently reallocates memory
//      when inserting multiple elements via variadic templates.
// ================================================================

#include <iostream>
#include <utility> // for std::forward
#include <cstddef> // for size_t

// --------------------------------------------------------------
// EXAMPLE 1: Recursive variadic template function (adder)
// --------------------------------------------------------------
//
// A variadic template lets you accept an arbitrary number of arguments
// using a *parameter pack*.
// Here, we recursively "peel off" arguments until only one is left.

// Base case: only one argument left
template <typename T>
T adder(T value)
{
    std::cout << "Base case reached with value = " << value << '\n';
    return value;
}

// Recursive case: at least two arguments
template <typename T, typename... Args>
T adder(T first, Args... rest)
{
    std::cout << "Adding " << first << " + adder(" << sizeof...(rest)
              << " more args)\n";
    return first + adder(rest...); // recursively reduce the pack
}

// --------------------------------------------------------------
// EXAMPLE 2: Toy "MyVector" class with a variadic "push_back_all"
// --------------------------------------------------------------
//
// This demonstrates a *different* use of variadic templates: expanding
// multiple arguments into multiple function calls.
//
// We'll simulate a vector that can "append" multiple elements at once,
// but does so in a horribly inefficient way: it *deallocates* and
// *reallocates* memory every single time we add something.
//
// The goal is just to show variadic template expansion syntax, not
// good engineering practice!
// --------------------------------------------------------------

template <typename T>
class MyVector
{
private:
    T *data_ = nullptr;    // pointer to the heap array
    std::size_t size_ = 0; // how many elements we have

public:
    MyVector() = default;

    ~MyVector()
    {
        delete[] data_; // free allocated memory
    }

    std::size_t size() const { return size_; }

    void push_back(const T &value)
    {
        // INEFFICIENT: allocate new memory every single time
        T *new_data = new T[size_ + 1];

        // copy old elements
        for (std::size_t i = 0; i < size_; ++i)
            new_data[i] = data_[i];

        // add the new element
        new_data[size_] = value;

        // free old memory
        delete[] data_;
        data_ = new_data;
        ++size_;
    }

    // Variadic version: pushes back multiple elements recursively
    template <typename... Args>
    void push_back_all(const T &first, const Args &...rest)
    {
        std::cout << "Adding one element (current size = " << size_ << ")\n";
        push_back(first); // add the first element

        // If there are more elements, expand the rest recursively
        if constexpr (sizeof...(rest) > 0)
        {
            push_back_all(rest...);
        }
    }

    // Print contents
    void print() const
    {
        std::cout << "MyVector contents (" << size_ << " elements): ";
        for (std::size_t i = 0; i < size_; ++i)
            std::cout << data_[i] << ' ';
        std::cout << '\n';
    }
};

// --------------------------------------------------------------
// MAIN
// --------------------------------------------------------------

int main()
{
    std::cout << "=== VARIADIC ADDER EXAMPLE ===\n";
    int result = adder(1, 2, 3, 4, 5);
    std::cout << "Final result = " << result << "\n\n";

    std::cout << "=== VARIADIC MyVector EXAMPLE ===\n";
    MyVector<int> vec;
    vec.push_back_all(10, 20, 30, 40);
    vec.print();

    return 0;
}
