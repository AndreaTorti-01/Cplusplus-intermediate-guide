{% raw %}

# Modern C++ Language Features

## Memory and References

### Memory Allocation

#### `new` and `delete`

C++ provides `new` and `delete` for dynamic memory allocation:

```cpp
// ---- Stack allocation ----
int stackInt;              // uninitialized (garbage value)
int stackInitInt = 42;     // initialized

// ---- Heap allocation ----
int* heapInt = new int;           // uninitialized (garbage value)
int* heapInitInt = new int(42);   // initialized to 42

delete heapInt;
delete heapInitInt;

// ---- Object on stack ----
class Widget {
public:
    Widget() { /* default constructor */ }
    Widget(int val) { /* custom constructor */ }
};

Widget stackObj;           // default constructor
Widget stackCustomObj(99); // custom constructor

// ---- Object on heap ----
Widget* heapObj = new Widget();      // default constructor
Widget* heapCustomObj = new Widget(99); // custom constructor

delete heapObj;
delete heapCustomObj;

// will touch on smart pointers below but putting this here anyway for reference
#include <memory>
// make_unique for int and Widget (heap-managed unique_ptr)
std::unique_ptr<int> uniqueInt = std::make_unique<int>(42);          // unique_ptr<int> initialized to 42
std::unique_ptr<Widget> uniqueWidget = std::make_unique<Widget>(99);    // unique_ptr<Widget> constructed with 99

// make_shared for int and Widget (heap-managed shared_ptr)
std::shared_ptr<int> sharedInt = std::make_shared<int>(42);          // shared_ptr<int> initialized to 42
std::shared_ptr<Widget>  sharedWidget = std::make_shared<Widget>(99);    // shared_ptr<Widget> constructed with 99
```

**Key differences from `malloc`/`free`:**

* `new`/`delete` **call constructors and destructors** automatically
* `malloc`/`free` only allocate/deallocate raw memory
* `new` returns a **typed pointer** (no cast needed)
* `malloc` returns `void*` (requires casting in C++)
* `new` throws `std::bad_alloc` on failure; `malloc` returns `nullptr`

**Rule:** Use `delete[]` for arrays, `delete` for single objects. Mismatching causes undefined behavior.

---

### References

A **reference** is an alias to an existing object. Once bound, it **cannot be reseated** — it always refers to the same object.

```cpp
int x = 10;
int& ref = x;   // ref is now an alias for x

ref = 20;       // modifies x
std::cout << x; // prints 20

int y = 30;
ref = y;        // does NOT rebind ref to y
                // instead, assigns y's value to x
std::cout << x; // prints 30
```

**References cannot be rebound:**

```cpp
void tryRebind(int& ref) {
    int local = 100;
    ref = local;  // does NOT make ref point to local
                  // it copies local's value into whatever ref refers to
}

int a = 5;
int& r = a;
tryRebind(r);
std::cout << a;  // prints 100 (a was modified, not r's binding)
```

**Const references:**

```cpp
const int& cref = x;  // cannot modify x through cref
// cref = 50;         // error: assignment of read-only reference

void print(const int& val) {
    std::cout << val;  // can read, cannot modify
}
```

**Key points:**

* Must be initialized when declared
* Cannot be `nullptr` (unlike pointers)
* Cannot be rebound to another object
* Safer than pointers for function parameters
* No overhead — compiled as pointers internally

---

## Lambda Expressions

Lambdas are anonymous functions defined inline. They're useful for short callbacks, algorithms, and local operations.

### Basic Syntax

```cpp
// Basic lambda: [] (parameters) -> return_type { body }
auto add = [](int a, int b) -> int { return a + b; };
int sum = add(5, 3); // 8

// Return type can be omitted (deduced)
auto multiply = [](int a, int b) { return a * b; };
int product = multiply(5, 3); // 15
```

### Capture Modes

```cpp
int x = 10;
int y = 20;

// Capture nothing
auto f1 = []() { /* cannot access x or y */ };

// Capture x by value
auto f2 = [x]() { return x; }; // gets a copy of x

// Capture all local variables by value
auto f3 = [=]() { return x + y; };

// Capture x by reference
auto f4 = [&x]() { x *= 2; }; // modifies the original x

// Capture all local variables by reference
auto f5 = [&]() { x *= 2; y *= 3; };

// Mixed captures
auto f6 = [x, &y]() { return x + (y *= 2); }; // x by value, y by reference
```

### Common Use Cases

```cpp
// With algorithms
std::vector<int> nums = {1, 2, 3, 4, 5};
std::for_each(nums.begin(), nums.end(), [](int& n) { n *= 2; });

// As a predicate
auto isEven = [](int n) { return n % 2 == 0; };
auto it = std::find_if(nums.begin(), nums.end(), isEven);

// Capturing object state in class methods
class Counter {
    int count = 0;
public:
    auto getCounterLambda() {
        // Captures 'this' pointer to access instance variables
        return [this]() { return ++count; };
    }
};
```

---

## C++ Classes are just fancy C Structs

Methods inside classes just have a hidden parameter — the `this` pointer. It’s syntactic sugar for “pass the object as the first argument.”

```cpp
obj.method();    // roughly becomes
method(&obj);    // implicit this → explicit pointer
```

Think “a C++ class is data like a C struct, plus functions that take a pointer to that data.” Non-virtual methods don’t add per-object overhead.

### Plain methods: C++ vs “C-style” equivalent

C++:
```cpp
#include <cmath>

struct Vec2 {
        float x, y;

        void translate(float dx, float dy) { x += dx; y += dy; }
        float length() const { return std::sqrt(x*x + y*y); }
};

int main() {
        Vec2 v{3, 4};
        v.translate(1, -2);
        float L = v.length(); // 5
}
```

Equivalent C-style:
```c
#include <math.h>

typedef struct {
        float x, y;
} Vec2;

void Vec2_translate(Vec2* self, float dx, float dy) {
        self->x += dx; self->y += dy;
}

float Vec2_length(const Vec2* self) {
        return sqrtf(self->x*self->x + self->y*self->y);
}

int main(void) {
        Vec2 v = {3, 4};
        Vec2_translate(&v, 1, -2);
        float L = Vec2_length(&v); // 5
}
```

Notes:
- A const method (e.g., float length() const) maps to a function taking a pointer to const: const Vec2* self.
- References in C++ (T&) map to pointers in C (T*).
- C++ class vs struct only changes default access (private vs public); layout is the same for plain data.

---

## Smart Pointers (RAII)

Smart pointers are wrapper objects that automate `new` and `delete` using RAII. They guarantee memory is freed when the pointer goes out of scope, preventing leaks.

### The Problem: Raw Pointers

```cpp
void do_work() {
    MyClass* raw_ptr = new MyClass();
    
    raw_ptr->doSomething();
    
    if (some_error_condition) {
        return; // LEAK! 'delete' is skipped.
    }
    
    delete raw_ptr;
    // delete raw_ptr; // CRASH! (Double free)
}
```

-----

### `std::unique_ptr` (Sole Ownership)

This is the **default, zero-cost** choice. It ensures **only one owner** for a resource. It's move-only, just like the `String` class example.

  * **Overhead:** None. Same size as a raw pointer. No refcounting.
  * **Best Practice:** Always create using `std::make_unique` (C++14+).

<!-- end list -->

```cpp
#include <memory>

void use_unique() {
    // 1. Creation
    std::unique_ptr<MyClass> ptr1 = std::make_unique<MyClass>();
    
    // 2. Usage (acts like a normal pointer)
    ptr1->doSomething();
    
    // 3. Ownership is exclusive
    // std::unique_ptr<MyClass> ptr2 = ptr1; // COMPILE ERROR: Cannot copy
    
    // 4. Transfer ownership using std::move
    std::unique_ptr<MyClass> ptr3 = std::move(ptr1);
    
    // ptr1 is now nullptr
    
} // ptr3 goes out of scope here, 'delete' is automatically called.
```

-----

### `std::shared_ptr` (Shared Ownership)

Use this **only when ownership must be shared** among multiple objects. It uses **atomic reference counting** to track how many "owners" exist. The resource is deleted when the *last* `shared_ptr` is destroyed.

  * **Overhead:** Has cost. `shared_ptr` is 2x the size of a raw pointer (16 bytes) and copying it requires a small atomic operation.
  * **Best Practice:** Always create using `std::make_shared`. This does **one heap allocation** (for the object *and* its counter) instead of two.

<!-- end list -->

```cpp
void use_shared() {
    std::shared_ptr<MyClass> s_ptr1;
    
    {
        // 1. Creation
        auto s_ptr2 = std::make_shared<MyClass>();
        
        // 2. Copying (reference count is now 2)
        s_ptr1 = s_ptr2;
        
        std::cout << s_ptr1.use_count(); // Prints 2
    
    } // s_ptr2 goes out of scope. Ref count is now 1.
    
    std::cout << s_ptr1.use_count(); // Prints 1
    
} // s_ptr1 goes out of scope. Ref count is 0. 'delete' is called.
```

-----

### `std::weak_ptr` (Breaking Cycles)

A `shared_ptr` cycle is a memory leak. `weak_ptr` is a **non-owning observer** that breaks these cycles. It does **not** increase the reference count.

#### The Problem: `shared_ptr` Cycle (Leak)

```cpp
struct Node {
    std::shared_ptr<Node> other; // Points to another Node
    ~Node() { std::cout << "Node destroyed\n"; }
};

void cause_a_leak() {
    auto n1 = std::make_shared<Node>(); // n1 ref count = 1
    auto n2 = std::make_shared<Node>(); // n2 ref count = 1
    
    n1->other = n2; // n2 ref count = 2
    n2->other = n1; // n1 ref count = 2
}
// n1 and n2 go out of scope.
// n1 ref count -> 1 (held by n2)
// n2 ref count -> 1 (held by n1)
// MEMORY LEAK! Neither destructor is called.
```

#### The Solution: Use `weak_ptr`

```cpp
struct FixedNode {
    std::shared_ptr<FixedNode> next;
    std::weak_ptr<FixedNode> prev; // Use weak_ptr to break the cycle
    ~FixedNode() { std::cout << "FixedNode destroyed\n"; }
};

void fix_the_leak() {
    auto n1 = std::make_shared<FixedNode>();
    auto n2 = std::make_shared<FixedNode>();
    
    n1->next = n2;
    n2->prev = n1; // 'prev' is weak, does NOT increase n1's ref count
}
// n1 goes out of scope. Ref count -> 0. n1 is destroyed.
// n1's destructor destroys 'next' (s_ptr2). n2 ref count -> 0. n2 is destroyed.
// NO LEAK!
```

#### How to Use a `weak_ptr`

You cannot use a `weak_ptr` directly. You must `.lock()` it to see if the object is still alive.

```cpp
std::weak_ptr<MyClass> w_ptr = s_ptr1; // s_ptr1 is an existing shared_ptr

// ... later ...

// Try to "promote" the weak_ptr to a shared_ptr
if (std::shared_ptr<MyClass> locked_ptr = w_ptr.lock()) {
    // SUCCESS: The object is still alive.
    // 'locked_ptr' is a valid shared_ptr we can use.
    locked_ptr->doSomething();
} else {
    // FAILURE: The object was already destroyed.
}
```

---

## Structured Bindings

Introduced in C++17, structured bindings provide a clean syntax for unpacking tuples, pairs, and class members into separate variables:

```cpp
#include <tuple>
#include <map>
#include <string>

// With tuples
std::tuple<int, std::string, double> getData() {
    return {42, "hello", 3.14};
}

void tupleExample() {
    auto [id, name, value] = getData();
    // id = 42, name = "hello", value = 3.14
}

// With pairs (such as map entries)
void mapExample() {
    std::map<std::string, int> scores = {{"Alice", 95}, {"Bob", 87}, {"Carol", 92}};
    
    // Iterate with structured binding
    for (const auto& [name, score] : scores) {
        std::cout << name << ": " << score << std::endl;
    }
    
    // Insert with structured binding
    if (auto [iter, inserted] = scores.insert({"Dave", 88}); inserted) {
        std::cout << "Dave added successfully" << std::endl;
    }
}

// With custom structs/classes (public members)
struct Point {
    int x;
    int y;
    int z;
};

void structExample() {
    Point p{1, 2, 3};
    auto [x, y, z] = p;
    // x = 1, y = 2, z = 3
}
```

Structured bindings make code more readable by eliminating temporary variables and explicit member access, especially useful with:
- Tuple returns from functions
- Iterating through maps
- Working with algorithm results returning std::pair
- Multiple return values

---

## Iterators  

Iterators are **objects that let you walk through the elements of a container** without exposing its internal representation.  

- They work for any container, even those that **don’t support random indexing** (e.g., `std::map`, `std::unordered_map`).  
- Because they behave like *pseudo‑pointers*, you can use the familiar `*it` and `++it` syntax.  

### Core operations  

| Operator | Meaning | Typical return type |
|----------|---------|---------------------|
| `*it`    | dereference – gives a reference to the element | `T&` |
| `++it`   | advance to the next element (pre‑increment) | `Iterator&` |
| `it != end` | inequality test – stops the loop | `bool` |

These three are the **minimum** you must provide for a *forward iterator* to be usable with range‑based `for` loops and most STL algorithms.

### Common member functions  

| Function | Description |
|----------|-------------|
| `begin()` | returns an iterator to the first element |
| `end()`   | returns an iterator **one past** the last element |
| `rbegin()`| returns a reverse iterator to the last element |
| `rend()`  | returns a reverse iterator **one before** the first element |

### Example: a simple iterator for a singly‑linked list  

```cpp
template <typename T>
class List {
    struct Node {
        T data;
        Node* next;
    };
    Node* head = nullptr;

public:
    // forward iterator
    class iterator {
        Node* cur;
    public:
        explicit iterator(Node* p) : cur(p) {}
        T& operator*() const { return cur->data; }
        iterator& operator++() { cur = cur->next; return *this; }
        bool operator!=(const iterator& other) const { return cur != other.cur; }
    };

    iterator begin()  const { return iterator(head); }
    iterator end()    const { return iterator(nullptr); }
};
```

```cpp
List<int> lst;
/* … fill the list … */
for (int& x : lst) {          // uses begin() / end()
    x *= 2;                   // modify each element
}
```

### Using STL algorithms  

Because the iterator satisfies the required interface, you can plug it into any standard algorithm:

```cpp
#include <algorithm>
#include <iostream>

std::vector<int> v = {5, 2, 9, 1};
std::sort(v.begin(), v.end());          // sort in ascending order
auto it = std::find(v.begin(), v.end(), 9);
if (it != v.end())
    std::cout << "found 9 at position " << (it - v.begin()) << '\n';
```

### Why implement your own?  

- **Custom data structures** (e.g., a B‑tree, a rope, a graph adjacency list) often need a *tailored traversal* that the standard iterators can’t provide.  
- Implementing the minimal set (`*`, `++`, `!=`) lets you **reuse the entire STL algorithm library** without writing bespoke loops.  

*In short, a well‑designed iterator turns any container into a first‑class citizen of the C++ ecosystem.*

> [Iterators Usage Example](https://github.com/AndreaTorti-01/Cplusplus-intermediate-guide/blob/main/iterators.cpp)

## Networking (Transport Layer)

Modern C++ applications typically interact at **Layer 4 (Transport)** through the **socket API**, which sits above the IP layer.
While the POSIX socket API is still the foundation, modern code prefers **cross-platform abstractions** like **Boost.Asio** or the **Networking TS (upcoming `std::net`)**.

You can choose between reliability and speed:

| Protocol                | Type     | Reliability  | Order       | Typical Use                          |
| ----------------------- | -------- | ------------ | ----------- | ------------------------------------ |
| **TCP** (`SOCK_STREAM`) | Stream   | ✅ Reliable   | ✅ Ordered   | File transfer, APIs, trading systems |
| **UDP** (`SOCK_DGRAM`)  | Datagram | ❌ Unreliable | ❌ Unordered | Games, telemetry, real-time feeds    |

**Tip – Low Latency with TCP:**
For real-time applications (e.g., trading, chat, games), disable **Nagle’s algorithm** using the `TCP_NODELAY` option.
This forces TCP to send packets *immediately*, instead of buffering them.

---

### Cross-Platform Blocking TCP Server (C++17+, Boost.Asio)

Here’s a **modern**, **blocking** TCP server written in portable C++.
It behaves like the traditional POSIX version, but works on Linux, Windows, and macOS with no code changes.

```cpp
#include <boost/asio.hpp>
#include <iostream>
#include <string>
#include <array>

using boost::asio::ip::tcp;

int main() {
    try {
        boost::asio::io_context io;

        // 1. Create a TCP endpoint (IPv4, port 8080)
        tcp::endpoint endpoint(tcp::v4(), 8080);

        // 2. Create a TCP acceptor (listens for connections)
        tcp::acceptor acceptor(io, endpoint);

        std::cout << "Waiting for connection..." << std::endl;

        // 3. Block until a client connects
        tcp::socket socket(io);
        acceptor.accept(socket);
        std::cout << "Client connected!" << std::endl;

        // 4. Disable Nagle’s algorithm to reduce latency
        boost::asio::ip::tcp::no_delay option(true);
        socket.set_option(option);

        // 5. Receive data (blocking)
        std::array<char, 1024> buffer;
        boost::system::error_code ec;
        std::size_t bytes = socket.read_some(boost::asio::buffer(buffer), ec);

        if (!ec) {
            std::string msg(buffer.data(), bytes);
            std::cout << "Received: " << msg << std::endl;
        } else if (ec == boost::asio::error::eof) {
            std::cout << "Connection closed cleanly.\n";
        } else {
            std::cerr << "Error: " << ec.message() << std::endl;
        }

        // Socket and acceptor auto-clean up via RAII
    }
    catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
}
```

---

### Key Advantages of boost/net modern approach over POSIX

| Feature          | POSIX                       | Boost.Asio / `std::net`                   |
| ---------------- | --------------------------- | ----------------------------------------- |
| Platform support | Linux/Unix only             | Cross-platform (Windows, macOS, Linux)    |
| Error handling   | Return codes                | Exceptions / error codes                  |
| RAII cleanup     | Manual (`close()`)          | Automatic (RAII)                          |
| Async support    | Manual threads & `select()` | Built-in async (`async_read`, coroutines) |
| TCP_NODELAY      | `setsockopt()`              | `socket.set_option(tcp::no_delay(true))`  |

---

### Future-Proof: C++23 / C++26 Networking TS

When your compiler supports it, you can replace Boost.Asio with `std::net` almost **1:1**:

```cpp
#include <experimental/net>
namespace net = std::experimental::net;
using net::ip::tcp;
```

Everything else — `io_context`, `tcp::socket`, `tcp::acceptor` will remain identical.

{% endraw %}