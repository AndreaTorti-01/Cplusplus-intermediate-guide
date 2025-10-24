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

#### Prefer brace initialization ({}) to avoid narrowing

Brace initialization performs list-initialization, which rejects narrowing conversions at compile time. Using `=` with `{}` (e.g., `int x = {42};`) is redundant—both forms are list-initialization. Prefer `T x{...};` for clarity and safety.

```cpp
int a = 3.14;    // OK: narrows to 3 (may only warn)
int b{3.14};     // error: narrowing conversion from double to int
int c = {3.14};  // error: same as above (list-initialization via =)

double d{3};     // OK: no narrowing
std::vector<int> v{1, 2, 3.0};   // error: 3.0 is double (narrowing)
std::vector<int> v2 = {1, 2, 3}; // OK: '=' is redundant here
```

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

### `std::shared_ptr` (Shared Ownership)

Use this **only when ownership must be shared** among multiple objects. It uses **atomic reference counting** to track how many "owners" exist. The resource is deleted (its destructor gets called) when the *last* `shared_ptr` is destroyed.

  * **Overhead:** Has cost. `shared_ptr` is 2x the size of a raw pointer (16 bytes): it stores the raw pointer to the referenced object, and the pointer to a structure called a *control block*. The control block contains:
    * either a pointer to the managed object or the managed object itself (sound strange? Well each shared pointer could point at a different part of the same managed object. each one would have a different raw pointer, but the same pointer to the managed object)
    * the deleter (type-erased because all shared_ptrs to the same object must have the same type regardless of their deleter type, allowing assignment and shared ownership between shared_ptrs created with different deleters)
    * the allocator (type-erased too)
    * the number of shared_ptrs that own the managed object
    * the number of weak_ptrs that refer to the managed object
  * **Best Practice:** Always create using `std::make_shared`. This does **one heap allocation** (for the object *and* its counter) instead of two.


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

### Key Advantages of boost/net modern approach over POSIX

| Feature          | POSIX                       | Boost.Asio / `std::net`                   |
| ---------------- | --------------------------- | ----------------------------------------- |
| Platform support | Linux/Unix only             | Cross-platform (Windows, macOS, Linux)    |
| Error handling   | Return codes                | Exceptions / error codes                  |
| RAII cleanup     | Manual (`close()`)          | Automatic (RAII)                          |
| Async support    | Manual threads & `select()` | Built-in async (`async_read`, coroutines) |
| TCP_NODELAY      | `setsockopt()`              | `socket.set_option(tcp::no_delay(true))`  |

### Future-Proof: C++23 / C++26 Networking TS

When your compiler supports it, you can replace Boost.Asio with `std::net` almost **1:1**:

```cpp
#include <experimental/net>
namespace net = std::experimental::net;
using net::ip::tcp;
```

Everything else — `io_context`, `tcp::socket`, `tcp::acceptor` will remain identical.

---

## Future, Promise, Async

Imagine you launch a background thread to do some heavy work:

```cpp
std::thread t([] {
    // do heavy stuff
    return 42;
});
```

But — wait — `std::thread` **can’t return a value**.
It just runs and exits; you can only `join()` it, not “get a result”.

We need a mechanism that lets one thread *produce* a result and another thread *consume* it.

That’s where **promises**, **futures**, and **async** come in.

Think of `std::promise` as a **write end** of a one-time communication channel, and `std::future` as the **read end**.

```cpp
#include <iostream>
#include <thread>
#include <future>

int main() {
    std::promise<int> p;              // producer
    std::future<int> f = p.get_future();  // consumer (associated with that promise)

    std::thread worker([&p]() {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        p.set_value(42);              // fulfill the promise
    });

    std::cout << "Waiting...\n";
    int result = f.get();             // blocks until set_value() is called
    std::cout << "Result = " << result << "\n";

    worker.join();
}
```

* `p` and `f` share a hidden state.
* `set_value()` fills that state.
* `f.get()` waits until that happens and retrieves the value.

It’s **thread-safe**, **synchronizing**, and one-time only (you can’t set twice or get twice).

### What about `std::async`?

1. Creates a promise/future pair,
2. Launches a new thread (by default),
3. Fulfills the promise when the thread finishes.

So instead of manually wiring `promise` and `future`, you just do:

```cpp
#include <iostream>
#include <future>

int heavy_task(int a, int b) {
    std::this_thread::sleep_for(std::chrono::seconds(2));
    return a + b;
}

int main() {
    std::future<int> f = std::async(std::launch::async, heavy_task, 10, 32);
    // async automatically runs heavy_task(10,32) in another thread

    std::cout << "Doing other stuff...\n";
    std::cout << "Result = " << f.get() << "\n";  // waits for result
}
```

`std::async` takes a **policy**:

* `std::launch::async` → always runs in a new thread.
* `std::launch::deferred` → runs **lazily** when you call `get()` or `wait()`.
* (default: may choose either depending on implementation)

Example:

```cpp
auto f = std::async(std::launch::deferred, [] {
    std::cout << "Running now!\n";
    return 123;
});

std::this_thread::sleep_for(std::chrono::seconds(2));
f.get(); // only here the lambda runs
```

---

## Variadic Templates...

They're useful when we expect a variable number of inputs to a function.

We use a template parameter pack, and inside the actual function we have a function parameter pack.

For example...

```cpp
// ---- Recursive pattern (C++11/14) ----

// base case
template <typename T>
T Adder(T v) {
    return v;
}

// recursive case
template <typename T, typename... Args>
auto Adder(T first, Args... args) {
    return first + Adder(args...);
}
```

> For mixed types, prefer a common accumulator type to avoid narrowing:
```cpp
#include <type_traits>

template <typename... Ts>
auto sum_recursive(Ts... xs) {
    using R = std::common_type_t<Ts...>;
    // base case + recursion usually require overloads; prefer folds below instead
    return (R{} + ... + static_cast<R>(xs)); // C++17 fold; identity R{} handles 0 args
}
```

### Fold Expressions (C++17+)

Folds are the modern way to “reduce” a parameter pack without recursion.

```cpp
// Sum all arguments (works for ints, doubles, etc.)
template <typename... Ts>
auto sum(Ts... xs) {
    using R = std::common_type_t<Ts...>;
    return (R{} + ... + static_cast<R>(xs)); // identity avoids empty-pack issues
}

// Multiply all arguments (1 is the multiplicative identity)
template <typename... Ts>
auto product(Ts... xs) {
    using R = std::common_type_t<Ts...>;
    return (R{1} * ... * static_cast<R>(xs));
}

// Print all arguments, space-separated
template <typename... Ts>
void print_all(Ts&&... xs) {
    // left fold over operator<< with a separator
    ((std::cout << xs << ' '), ...);
    std::cout << '\n';
}
```

Notes:
- Left vs right fold: (a op ... op z) vs (a op (... op z)). For associative ops (+, *) it’s fine; for non-associative, pick consistently.
- Provide an identity (e.g., 0 for +, 1 for *) if you want to support zero arguments.

### Perfect Forwarding with Packs

Use forwarding references to avoid copies and support move-only types.

```cpp
#include <utility>
#include <vector>

// forward arguments into a container (construct in-place)
template <typename T, typename... Args>
T& append_emplaced(std::vector<T>& v, Args&&... args) {
    return v.emplace_back(std::forward<Args>(args)...);
}

// apply a callable to each arg
template <typename F, typename... Args>
void for_each_arg(F&& f, Args&&... args) {
    (f(std::forward<Args>(args)), ...); // guaranteed left-to-right since C++17
}
```

### Edge Cases and Tips

- Zero arguments:
  - Recursive functions need a base case.
  - Folds can use an identity: ``(R{} + ... + xs)`` returns ``R{}`` when no args are passed.
- Evaluation order:
  - Since C++17, folds evaluate left-to-right. Pre‑C++17 you’d use an initializer-list trick: ``int unused[] = { (f(xs), 0)... };``
- Type promotion:
  - Prefer ``std::common_type_t`` to avoid narrowing and to get a stable accumulator type for mixed args.
- Move-only types:
  - Use forwarding references (``T&&`` with template type deduction) and ``std::forward`` to pass through rvalues.
- Constraining templates:
  - C++17 (SFINAE): ``template <typename... Ts, std::enable_if_t<(std::conjunction_v<std::is_arithmetic<Ts>...>), int> = 0>``  
  - C++20 (requires): ``template <typename... Ts> requires (std::is_arithmetic_v<Ts> && ...) …``

> check out [this](https://github.com/AndreaTorti-01/Cplusplus-intermediate-guide/blob/main/variadic_templates.cpp) code for a more comprehensive example

---

## The Curious Case of `std::vector<bool>`

`std::vector<bool>` is a **template specialization** that behaves differently from all other `std::vector` types. Instead of storing individual `bool` values, it **packs bits** to save memory — but this comes with serious tradeoffs.

```cpp
#include <vector>

int main()
{
    std::vector<bool> vec = {true, false, true, false};
    // should occupy 4 bytes, occupies half a byte instead (1 byte in memory)...
    bool bool_copy = vec[0]; // copy, ok
    bool& bool_ref = vec[0]; // error!
    std::vector<bool>::reference bool_ref_crazy = vec[0]; // correct!
    auto bool_ref_clean = vec[0]; // how it's done in reality
}
```

---

## Basics of Concurrency in C++

Modern C++ provides powerful synchronization primitives for thread-safe code. Here are the most important facilities you'll use regularly.

### `std::mutex` - The Foundation

The fundamental mutual exclusion primitive. Protects shared data from concurrent access:

```cpp
#include <mutex>
#include <thread>

std::mutex mtx;
int shared_counter = 0;

// BAD: Manual lock/unlock is dangerous
void dangerousIncrement() {
    mtx.lock();
    ++shared_counter;
    // Exception here = DEADLOCK! Mutex never unlocks
    mtx.unlock();
}

// GOOD: RAII wrapper handles unlock automatically
void safeIncrement() {
    std::lock_guard<std::mutex> lock(mtx);
    ++shared_counter;
    // Unlocks even if exception is thrown
}
```

**Never call `lock()` and `unlock()` manually.** Always use RAII wrappers for exception safety.

### `std::lock_guard` and `std::unique_lock` (C++11)

Two RAII wrappers with different trade-offs:

```cpp
#include <mutex>

std::mutex mtx;
int data = 0;

// lock_guard: Simple, fast, inflexible
void simpleLocking() {
    std::lock_guard<std::mutex> lock(mtx);
    ++data;
    // That's it - locks on construction, unlocks on destruction
}

// unique_lock: Flexible, required for condition variables
void flexibleLocking() {
    std::unique_lock<std::mutex> lock(mtx, std::defer_lock);
    
    // Do work without lock
    
    lock.lock();      // Lock when needed
    ++data;
    lock.unlock();    // Unlock early to reduce contention
    
    // Do more work without lock
    
    lock.lock();      // Can lock again
    data *= 2;
    // Auto-unlocks on destruction
}
```

Use `lock_guard` by default. Switch to `unique_lock` only when you need flexibility or condition variables.

### `std::scoped_lock` (C++17) - Deadlock Prevention

The modern way to lock multiple mutexes atomically:

```cpp
#include <mutex>

std::mutex mtx1, mtx2;
int account1 = 1000, account2 = 500;

// BAD: Classic deadlock scenario
void deadlockProne() {
    std::lock_guard<std::mutex> lock1(mtx1);  // Thread A gets mtx1
    std::lock_guard<std::mutex> lock2(mtx2);  // Thread B gets mtx2
    // If another thread locks in opposite order = DEADLOCK
}

// GOOD: Atomic multi-lock prevents deadlock
void transfer(int amount) {
    std::scoped_lock lock(mtx1, mtx2);  // Locks both atomically
    account1 -= amount;
    account2 += amount;
}

// Works with single mutex too (replaces lock_guard in C++17+)
void singleLock() {
    std::scoped_lock lock(mtx1);
    ++account1;
}
```

**Always use `scoped_lock` for multiple mutexes.** It uses a deadlock-avoidance algorithm under the hood. In C++17+, prefer it over `lock_guard` for consistency.

### `std::shared_mutex` (C++17) - Reader-Writer Locks

Allows multiple readers OR one writer. Perfect for read-heavy workloads:

```cpp
#include <shared_mutex>
#include <map>

std::shared_mutex cache_mtx;
std::map<int, std::string> cache;

// Many readers can run simultaneously
std::string readCache(int key) {
    std::shared_lock lock(cache_mtx);  // Shared access
    return cache[key];
    // Multiple threads can hold shared_lock concurrently
}

// Writer gets exclusive access
void writeCache(int key, std::string value) {
    std::unique_lock lock(cache_mtx);  // Exclusive access
    cache[key] = value;
    // No other thread (reader or writer) can access
}

// BAD: Regular mutex serializes all readers
std::mutex bad_mtx;
std::string inefficientRead(int key) {
    std::lock_guard lock(bad_mtx);
    return cache[key];  // Only ONE reader at a time!
}
```

Think of `shared_mutex` as: **many readers can share, writers need exclusivity.** Use regular `mutex` for write-heavy or balanced workloads since it's faster.

### `std::condition_variable` (C++11) - Thread Synchronization

Allows threads to wait for specific conditions without busy-waiting:

```cpp
#include <condition_variable>
#include <mutex>
#include <queue>

std::mutex mtx;
std::condition_variable cv;
std::queue<int> tasks;
bool shutdown = false;

void producer() {
    for (int i = 0; i < 100; ++i) {
        {
            std::unique_lock lock(mtx);
            tasks.push(i);
        }  // Unlock before notify for better performance
        cv.notify_one();
    }
    
    std::unique_lock lock(mtx);
    shutdown = true;
    cv.notify_all();
}

void consumer() {
    while (true) {
        std::unique_lock lock(mtx);
        
        // Predicate prevents spurious wakeups
        cv.wait(lock, []{ return !tasks.empty() || shutdown; });
        
        if (tasks.empty()) break;  // Shutdown and no work
        
        int task = tasks.front();
        tasks.pop();
        lock.unlock();  // Process outside critical section
        
        // Process task...
    }
}
```

**Key points:** Condition variables require `unique_lock` (not `lock_guard`). Always use the predicate form of `wait()` to handle spurious wakeups correctly.

### `std::counting_semaphore` and `std::binary_semaphore` (C++20)

Control access to limited resources. Lighter weight than mutexes for simple counting:

```cpp
#include <semaphore>
#include <thread>

// Limit to 5 concurrent database connections
std::counting_semaphore<5> db_pool(5);

void queryDatabase() {
    db_pool.acquire();  // Wait if all 5 slots taken
    // Execute database query
    db_pool.release();  // Free up a slot
}

// Binary semaphore: signal between threads
std::binary_semaphore signal(0);  // Starts at 0

void waiter() {
    signal.acquire();  // Blocks until released
    // Proceed after signal
}

void notifier() {
    // Do some work
    signal.release();  // Wake up waiter
}

// Try without blocking
void tryQuery() {
    if (db_pool.try_acquire()) {
        // Got a connection immediately
        db_pool.release();
    } else {
        // All busy, try later
    }
}
```

Semaphores don't have ownership semantics like mutexes. Use them for resource pools, producer-consumer patterns, or simple signaling between threads.

### `std::latch` and `std::barrier` (C++20)

Synchronization points for coordinating multiple threads:

```cpp
#include <latch>
#include <barrier>
#include <thread>
#include <vector>

// LATCH: One-time countdown (cannot be reused)
void parallelProcessing() {
    std::vector<int> data(1000);
    std::latch workers_done(4);  // Wait for 4 threads
    
    auto worker = [&](int start, int end) {
        for (int i = start; i < end; ++i) {
            data[i] = i * i;
        }
        workers_done.count_down();  // Signal completion
    };
    
    std::vector<std::thread> threads;
    for (int i = 0; i < 4; ++i) {
        threads.emplace_back(worker, i * 250, (i + 1) * 250);
    }
    
    workers_done.wait();  // Block until all 4 finish
    // Now safe to use data
    
    for (auto& t : threads) t.join();
}

// BARRIER: Reusable synchronization point
void multiPhaseAlgorithm() {
    std::vector<double> results(100);
    std::barrier phase_sync(4);  // 4 threads sync at each phase
    
    auto worker = [&](int id) {
        // Phase 1: Initialize
        for (int i = 0; i < 25; ++i) { /* work */ }
        phase_sync.arrive_and_wait();  // All threads sync here
        
        // Phase 2: Process (can use results from phase 1)
        for (int i = 0; i < 25; ++i) { /* work */ }
        phase_sync.arrive_and_wait();  // Barrier resets automatically
        
        // Phase 3: Finalize
        for (int i = 0; i < 25; ++i) { /* work */ }
    };
    
    std::vector<std::thread> threads;
    for (int i = 0; i < 4; ++i) {
        threads.emplace_back(worker, i);
    }
    for (auto& t : threads) t.join();
}
```

**Choose wisely:** Use `latch` for simple "wait until N tasks complete" patterns. Use `barrier` when threads need to synchronize at multiple points in a loop or multi-phase algorithm.

### `std::call_once` and `std::once_flag` (C++11)

Thread-safe one-time initialization:

```cpp
#include <mutex>
#include <memory>

class ExpensiveResource {
private:
    static std::unique_ptr<ExpensiveResource> instance;
    static std::once_flag init_flag;
    
    ExpensiveResource() { /* expensive setup */ }
    
public:
    static ExpensiveResource& get() {
        std::call_once(init_flag, []() {
            instance = std::make_unique<ExpensiveResource>();
        });
        return *instance;
    }
};

std::unique_ptr<ExpensiveResource> ExpensiveResource::instance;
std::once_flag ExpensiveResource::init_flag;

// Can also use for configuration loading
std::once_flag config_loaded;
void loadConfig() {
    std::call_once(config_loaded, []() {
        // Load configuration files
        // This runs exactly once, no matter how many threads call it
    });
}
```

Much simpler and more reliable than hand-rolled double-checked locking. The initialization function runs exactly once across all threads, guaranteed.

### Quick Reference

**Choose the right tool:**

* **Basic protection** → `std::lock_guard` or `std::scoped_lock`
* **Early unlock needed** → `std::unique_lock`
* **Multiple mutexes** → `std::scoped_lock` (C++17)
* **Read-heavy data** → `std::shared_mutex` (C++17)
* **Thread notification** → `std::condition_variable`
* **Resource limiting** → `std::counting_semaphore` (C++20)
* **Wait for tasks** → `std::latch` (C++20)
* **Multi-phase sync** → `std::barrier` (C++20)
* **One-time init** → `std::call_once`

**Core principles:** Always use RAII. Minimize time holding locks. Prefer standard library primitives over custom solutions. Use C++17/20 features when available for cleaner, safer code. AVOID locking and unlocking in tight loops.