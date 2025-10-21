#include <vector>
#include <cstdlib>
#include <cstdio>
#include <new>
#include <iostream>

void *operator new(std::size_t size)
{
    void *p = std::malloc(size);
    if (!p)
        throw std::bad_alloc();
    std::printf("[new]  allocated %zu bytes at %p\n", size, p);
    return p;
}

void operator delete(void *p) noexcept
{
    std::printf("[delete] freeing memory at %p\n", p);
    std::free(p);
}

void operator delete(void *p, std::size_t size) noexcept
{
    std::printf("[delete sized] freeing %zu bytes at %p\n", size, p);
    std::free(p);
}

std::vector<int> makeVector() {
    std::cout << "about to create vector\n";
    std::vector<int> v = {1, 2, 3, 4}; // expect allocation
    std::cout << "about to add an element\n";
    v.push_back(5); // expect new allocation and deletion of old
    std::cout << "added\n";
    return v;
}

int main() {
    std::cout << "about to call makevector\n";
    std::vector<int> a = makeVector();            // Case 1: return by value (RVO or move)
    std::cout << "about to copy\n";
    std::vector<int> b = a;                      // Case 2: copy: new allocation
    std::cout << "about to move\n";
    std::vector<int> c = std::move(a);           // Case 3: move: nothing happens
    std::cout << "size of c: " << c.size() << "; size of a: " << a.size() << "\n"; // a size is 0 but a is in valid state
    std::cout << "end\n";
}