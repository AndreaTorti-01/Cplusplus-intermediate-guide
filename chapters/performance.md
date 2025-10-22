# Performance Tips and Techniques

## Various Performance Notes

* **Branching (`if`)** is slow in tight loops: avoid in hot paths if possible.
  → Prefer *branchless programming* (math tricks or bitwise ops).
* **Strings**: use them ONLY when interfacing with a human. NEVER use them as IDs and similar structures.
* **std::endl**, this infamous monster: don't use it if not strictly necessary. It's asking the OS to flush the output buffer, and this is SLOW!
* **References**: use them generously to avoid constantly copying memory around (especially when asking the OS to allocate some more on the heap) BUT be careful, your dear reference could be invalidated, either by yourself (eg in a loop) or by any other thread!

---

## Stack vs Heap Memory

* **Stack memory**:
  * Fast allocation (adjusting a pointer)
  * Automatic lifetime management (freed when scope ends)
  * Limited size (typically a few MB)
  * Variables declared inside functions are on the stack by default
  * Great for small, temporary objects

* **Heap memory**:
  * Dynamic size (limited by system RAM)
  * Manual management (in raw C++, must be freed explicitly)
  * Slower allocation (involves finding space)
  * Lives until explicitly freed
  * Objects created with `new` are on the heap

```cpp
// Stack allocation - automatically cleaned up when scope ends
{
    int stackArray[1000]; // lives on stack
    MyClass stackObj;     // constructor called, destructor called at end of scope
} // everything above is automatically freed here

// Heap allocation - lives until explicitly deleted
MyClass *heapObj = new MyClass(); // lives on heap until...
delete heapObj;                   // ...explicitly freed here
```

Use smart pointers(see Smart Pointers section) to manage heap memory safely and automatically.

---

## Return Value Optimization (RVO)

Compilers can eliminate unnecessary copying when returning objects from functions:

```cpp
class Widget {
public:
    Widget() { std::cout << "Constructor\n"; }
    Widget(const Widget&) { std::cout << "Copy constructor\n"; }
    ~Widget() { std::cout << "Destructor\n"; }
};

Widget createWidget() {
    Widget w;       // Constructor called
    return w;       // Copy constructor NOT called due to RVO
}

int main() {
    Widget w = createWidget(); // Only one constructor call total
}
```

**Key points:**
* RVO is active by default, even at `-O0` optimization level
* Forces the compiler to eliminate the copy/move from local variables returned by value
* To disable for testing: `-fno-elide-constructors` compiler flag
* Named RVO (NRVO) works with named return values
* C++17 guarantees copy elision in certain cases, making moves unnecessary

---

## Copy and Move Semantics

### L-values vs R-values

* **L-value** = has a name, has an address, persists beyond a single expression
    ```cpp
    int x = 10;        // x is an l-value
    int& ref = x;      // can bind l-value reference to x
    ```

* **R-value** = temporary, no name, typically the result of an expression
    ```cpp
    int y = 5 + 3;     // (5 + 3) is an r-value
    // int& ref = 5;   // error: cannot bind l-value reference to r-value
    int&& rref = 5;    // OK: r-value reference
    ```

**Key insight:** R-values are about to die — we can "steal" their resources safely.

---

### `std::move` vs Casting to R-value Reference

* **`std::move(x)`** is just a cast: `static_cast<T&&>(x)`
* It does **not move anything** — it only *marks* `x` as movable
* The actual move happens in the move constructor/assignment operator

```cpp
String source("Hello");
String dest = std::move(source);  // calls move constructor
dest = std::move(source) // this would call the move assignment operator instead!
```

**After `std::move(source)`:**
* `source` is in a valid but unspecified state
* Do not use `source` unless you reassign it

---

### Copy Constructor

**Default behavior:** Shallow copy (bitwise copy of all members).

**When to define your own:** When your class owns heap memory or other resources.

```cpp
class String {
        char* m_Data;
        size_t m_Size;

public:
        // Copy constructor
        String(const String& other)
                : m_Size(other.m_Size)
        {
                m_Data = new char[m_Size + 1];           // allocate new memory
                std::memcpy(m_Data, other.m_Data, m_Size + 1); // deep copy
        }
};

String source("Hello");
String dest = source;  // calls copy constructor
```

---

### Move Constructor

**Purpose:** Transfer ownership without copying.

**Requirements:**
1. Transfer resources from source
2. Nullify source pointers
3. Mark as `noexcept` (enables optimizations)

```cpp
class String {
        char* m_Data;
        size_t m_Size;

public:
        // Move constructor
        String(String&& other) noexcept
                : m_Data(other.m_Data)    // steal the pointer
                , m_Size(other.m_Size)    // copy the size
        {
                other.m_Data = nullptr;   // nullify source
                other.m_Size = 0;         // reset size
        }
};

String source("Hello");
String dest = std::move(source);  // calls move constructor
// source.m_Data is now nullptr
```

---

### Copy Assignment Operator

**Pattern:** Copy-and-swap or manual implementation.

```cpp
class String {
        char* m_Data;
        size_t m_Size;

public:
        // Copy assignment
        String& operator=(const String& other) {
                if (this == &other) return *this;     // handle self-assignment
                
                delete[] m_Data;                      // free current resources
                
                m_Size = other.m_Size;                // copy size
                m_Data = new char[m_Size + 1];        // allocate new memory
                std::memcpy(m_Data, other.m_Data, m_Size + 1); // deep copy
                
                return *this;
        }
};

String source("Hello");
String dest("World");
dest = source;  // calls copy assignment
```

---

### Move Assignment Operator

**Requirements:**
1. Check self-assignment
2. Free current resources
3. Transfer ownership from source
4. Nullify source

```cpp
class String {
        char* m_Data;
        size_t m_Size;

public:
        // Move assignment
        String& operator=(String&& other) noexcept {
                if (this == &other) return *this;     // handle self-assignment
                
                delete[] m_Data;                      // free current resources
                
                m_Data = other.m_Data;                // steal pointer
                m_Size = other.m_Size;                // copy size
                
                other.m_Data = nullptr;               // nullify source
                other.m_Size = 0;                     // reset size
                
                return *this;
        }
};

String source("Hello");
String dest("World");
dest = std::move(source);  // calls move assignment
// source.m_Data is now nullptr
```

---

### Complete Example

```cpp
class String {
        char* m_Data;
        size_t m_Size;

public:
        // Constructor
        String(const char* str) {
                m_Size = strlen(str);
                m_Data = new char[m_Size + 1];
                std::memcpy(m_Data, str, m_Size + 1);
        }

        // Destructor
        ~String() {
                delete[] m_Data;
        }

        // Copy constructor
        String(const String& other)
                : m_Size(other.m_Size)
        {
                m_Data = new char[m_Size + 1];
                std::memcpy(m_Data, other.m_Data, m_Size + 1);
        }

        // Move constructor
        String(String&& other) noexcept
                : m_Data(other.m_Data)
                , m_Size(other.m_Size)
        {
                other.m_Data = nullptr;
                other.m_Size = 0;
        }

        // Copy assignment
        String& operator=(const String& other) {
                if (this == &other) return *this;
                delete[] m_Data;
                m_Size = other.m_Size;
                m_Data = new char[m_Size + 1];
                std::memcpy(m_Data, other.m_Data, m_Size + 1);
                return *this;
        }

        // Move assignment
        String& operator=(String&& other) noexcept {
                if (this == &other) return *this;
                delete[] m_Data;
                m_Data = other.m_Data;
                m_Size = other.m_Size;
                other.m_Data = nullptr;
                other.m_Size = 0;
                return *this;
        }
};

// Usage
String a("Hello");
String b = a;              // copy constructor
String c = std::move(a);   // move constructor (a is now empty)
String d("World");
d = b;                     // copy assignment
d = std::move(c);          // move assignment (c is now empty)
```

> [Example usage of move with a vector](https://github.com/AndreaTorti-01/Cplusplus-intermediate-guide/blob/main/move.cpp)

---

### Rule of Five

If you define any of these, you should probably define all:

1. Destructor
2. Copy constructor
3. Move constructor
4. Copy assignment operator
5. Move assignment operator

Or use `= default` / `= delete` to be explicit about your intent.

## Custom Allocators

Allocators can reserve a large memory region, manage it manually, and free it all at once.

Useful for:

* Trading systems
* Real-time engines
* Reducing fragmentation


> Check out the [Andreas Weis 2018 code::dive talk](https://youtu.be/FcpmMmyNNv8)

---

### Types of Custom Allocators

#### 1. Linear/Arena Allocator

**Concept:** Bump a pointer forward with each allocation. Never free individual objects — free everything at once.

```
┌─────────────────────────────────────┐
│  ████████████░░░░░░░░░░░░░░░░░░░░░  │  ← Arena
│              ↑                      │
│           offset                    │
└─────────────────────────────────────┘
allocate → move offset forward
reset    → offset = 0
```

**Core idea:**
```cpp
void* allocate(size_t bytes) {
    void* ptr = buffer + offset;
    offset += bytes;
    return ptr;
}
```

**Use case:** Frame allocations in game engines, temporary string building.

---

#### 2. Stack Allocator

**Concept:** Like a linear allocator, but deallocations must happen in **reverse order** (LIFO).

```
┌─────────────────────────────────────┐
│  ███ A ███ B ███ C ███░░░░░░░░░░░░  │
│                  ↑                  │
│                 top                 │
└─────────────────────────────────────┘
push(C)  → allocate
pop()    → free C (must be last)
pop()    → free B
```

**Core idea:**
```cpp
void deallocate(size_t bytes) {
    top -= bytes;  // must match last allocation
}
```

**Use case:** Nested scope allocations, call stacks, expression evaluation.

---

#### 3. Pool Allocator

**Concept:** Pre-allocate fixed-size chunks. Fast allocation/deallocation of same-sized objects.

```
Single fixed-size pool
┌────────────────────────────────────┐
│ [▓][▓][░][▓][░][░][▓][░][░]        │
│       ↑↓    ↑↓ ↑↓    ↑↓ ↑↓         │
│  head─┘└────┘└─┘└────┘└─┘└─null    │
│                                    │
└────────────────────────────────────┘
▓ = allocated
░ = free (singly linked free list)
```

**Core idea:** Maintain a linked list of free blocks.

- Use multiple pools with differently-sized chunks (size classes). Pick the smallest pool that fits the request; return blocks to the same pool.

**Use case:** Object pools (particles, enemies, bullets), network packet buffers.

---

#### 4. Free List Allocator

**Concept:** Like `malloc` but in a pre-allocated buffer. Maintains a list of free blocks of varying sizes.

```
┌─────────────────────────────────────────────┐
│ [used 32][free 64][used 16][free 128]...    │
│           ↓                 ↓               │
│       free node          free node          │
└─────────────────────────────────────────────┘
allocate   → find best-fit free block
deallocate → merge adjacent free blocks
```

**Strategies:**

* **First-fit:** Use first block large enough
* **Best-fit:** Use smallest block that fits (less fragmentation)
* **Worst-fit:** Use largest block

**Use case:** General-purpose allocator, game asset managers, custom `malloc`.

---

### When to Use Custom Allocators

**Consider using when:**
* You allocate many objects of the same size (pool)
* You allocate/free in bulk (linear/arena)
* You need deterministic performance (avoid `new`/`delete` overhead)
* You want to reduce heap fragmentation

**Important reality check:**

Custom allocators are **not** a general solution. They add complexity and make code harder to maintain. In most applications, the default allocator is fine and well-optimized by years of engineering.

Only reach for custom allocators when you have:
* **Profiled evidence** that allocation is a bottleneck
* **Specific allocation patterns** that fit one of these models
* **Real-world performance requirements** that justify the added complexity

For the vast majority of code, `new`/`delete` or `malloc`/`free` are the right choice. Custom allocators are specialized tools for specialized problems — not a performance silver bullet.

---

## Branchless Programming

In hot loops, unpredictable branches can stall the pipeline. Prefer forms the compiler can turn into branchless instructions.

Instead of:

```cpp
if (x > 0) y = 1; else y = 0;
```

Use a boolean-to-int conversion (often emits a single set instruction):

```cpp
y = (x > 0);  // y becomes 1 when x > 0, otherwise 0
```

A ternary is equally clear and typically optimized to the same branchless code:

```cpp
y = (x > 0) ? 1 : 0;  // same meaning; compilers usually lower this without a branch
```

Bit trick (sign-bit extraction) for negativity checks:

```cpp
// 32-bit example: 1 if x < 0, 0 otherwise
y = static_cast<unsigned int>(x) >> 31;
```

- Explanation:
    - The boolean form compares x to 0 and writes 1 for true, 0 for false.
    - The ternary expresses the same intent explicitly; modern compilers commonly generate branchless code for simple integral cases.
    - The sign-bit trick shifts the most significant bit into the least significant position; it answers “is x negative?”, not “is x > 0?”. It assumes two’s-complement and a 32-bit int; generalize the shift by using sizeof if needed.

Notes:
- Prefer clarity first; the boolean or ternary versions are portable and readable.
- Always measure; compilers already optimize these patterns aggressively.

> Check out the [Fedor Pikus 2021 CppCon talk](https://youtu.be/g-WPhYREFjk)

---

## Mutexes vs. Atomics

### `std::mutex` (Mutual Exclusion)

A mutex protects a **critical section** (a block of code) so that only one thread can execute it at a time.

  * **How it works:** Threads "lock" the mutex. If another thread tries to lock it, that thread **blocks** (sleeps) and yields its time to the OS.
  * **Cost:** The blocking/waking is a context switch managed by the OS, which can be "heavy" (slow).
  * **Best Practice:** Use `std::lock_guard` or `std::scoped_lock`. They use **RAII** to automatically unlock the mutex when the scope ends, even if an exception is thrown.

<!-- end list -->

```cpp
#include <mutex>
#include <vector>
#include <thread>

std::vector<int> g_myVector;
std::mutex g_myMutex; // Protects g_myVector

void thread_func() {
    // lock_guard automatically locks g_myMutex
    std::lock_guard<std::mutex> lock(g_myMutex);
    
    // --- CRITICAL SECTION ---
    // This block is now thread-safe.
    g_myVector.push_back(1);
    // --- END CRITICAL SECTION ---

} // g_myMutex is automatically unlocked here
```

### Other Mutex Types

  * **`std::recursive_mutex`:** Allows the *same thread* to lock the mutex multiple times. Useful for complex recursive functions. Slower than a normal mutex.
  * **`std::shared_mutex` (C++17):** A **reader-writer** lock.
      * Allows **many threads** to read at the same time (`std::shared_lock`).
      * Allows only **one thread** to write at a time (`std::unique_lock`).
      * Faster than a regular mutex if you have many more reads than writes.

<!-- end list -->

```cpp
#include <shared_mutex>
#include <map>

std::map<int, std::string> g_myMap;
std::shared_mutex g_myMapMutex;

std::string read_from_map(int key) {
    // Many threads can enter this function at once
    std::shared_lock<std::shared_mutex> lock(g_myMapMutex);
    auto it = g_myMap.find(key);
    if (it == g_myMap.end()) return "";
    return it->second;
}

void write_to_map(int key, const std::string& val) {
    // Only one thread can enter this function at a time
    std::unique_lock<std::shared_mutex> lock(g_myMapMutex);
    g_myMap[key] = val;
}
```

-----

### `std::atomic`

Provides **lock-free**, indivisible (atomic) operations on single types (like `int`, `bool`, pointers).

  * **How it works:** Uses special CPU instructions (e.g., `lock add`) that cannot be interrupted. Other threads do *not* block or sleep.
  * **Why faster?** No OS context switch. No thread blocking. This is *much* faster than a mutex if many threads are fighting for the same variable (high contention).
  * **Why limited?** It only protects a *single* operation on a *single* variable. It **cannot** protect a *block* of code (e.g., finding in a map *and then* inserting).

<!-- end list -->

```cpp
#include <atomic>
#include <thread>

std::atomic<int> g_counter(0); // This integer is now thread-safe

void increment_counter() {
    for (int i = 0; i < 10000; ++i) {
        g_counter++; // This is an atomic operation.
                     // It's like g_counter.fetch_add(1);
    }
}
```

### Atomic Compare-and-Swap (CAS)

This is the heart of most lock-free programming. It atomically does this:
"Check if `variable` still has `expected_value`. If yes, set it to `new_value`. Tell me if I succeeded."

```cpp
std::atomic<int> g_value(10);

void update_value() {
    int expected = 10;
    int desired = 20;

    // Try to change g_value from 10 to 20
    // This is one, indivisible hardware operation
    bool succeeded = g_value.compare_exchange_strong(expected, desired);
    
    if (succeeded) {
        // We successfully changed 10 to 20
    } else {
        // We failed. Some other thread changed g_value first.
        // `expected` is now updated to the *current* value.
    }
}
```
---

## Memory Coalescence and Data Layout

**Memory Coalescence** is a term (most common in GPGPU/CUDA) that describes how hardware groups memory accesses. The main idea applies to CPUs as well:

**Hardware *never* fetches just one byte.**

When you read a 4-byte `int`, the CPU *actually* fetches an entire **cache line** (e.g., 64 bytes).

  * **Coalesced Access:** Your *next* memory access is *also* in that 64-byte chunk. You use the full bandwidth you paid for.
  * **Uncoalesced (Scattered) Access:** Your next access is somewhere else in memory. The CPU fetches *another* 64-byte chunk, even though you only used 4 bytes of the first one. This *wastes* memory bandwidth.

### The SoA (Struct-of-Arrays) vs. AoS (Array-of-Structs) Pattern

This is the classic C++ example. Imagine processing a million particles, but you *only* need their `x` velocity.

**Bad: Array-of-Structs (AoS)**
This is the "natural" way to write it, but it's terrible for coalescence.

```cpp
struct Particle {
    float x, y, z;      // 12 bytes
    float vx, vy, vz;   // 12 bytes
    float mass;         // 4 bytes
    // ... 32 more bytes of other data
}; // Total: 60 bytes

std::vector<Particle> particles; // [P1, P2, P3, ...]

// Uncoalesced access:
double total_vx = 0;
for (const auto& p : particles) {
    total_vx += p.vx; // Jumps 60 bytes every time!
}
```

To read 4 bytes (`p.vx`), the CPU has to fetch a 64-byte cache line containing `p.x`, `p.y`, `p.z`, `p.vx`, `p.vy`, etc. It then *throws away* 60 of those bytes, only to fetch the *next* 64-byte chunk for the next particle.

**Good: Struct-of-Arrays (SoA)**
This layout is *perfect* for coalescence.

```cpp
struct ParticleData {
    std::vector<float> x, y, z;
    std::vector<float> vx, vy, vz;
    std::vector<float> mass;
    // ...
};

ParticleData particles; // All vx values are side-by-side

// Coalesced access:
double total_vx = 0;
for (float v : particles.vx) {
    total_vx += v; // Reads [vx1, vx2, vx3, ...]
}
```

Now, when the CPU fetches a 64-byte cache line, it gets 16 `vx` values (`64 / sizeof(float)`). Every single byte loaded is used by the loop. This **maximizes memory bandwidth** and is dramatically faster.

-----

## Taking Advantage of the CPU Cache

The **CPU cache** (L1, L2, L3) is a small, *extremely* fast memory that sits between the CPU and slow main RAM. Accessing data in the L1 cache can be **\~100x faster** than accessing RAM.

A **"cache miss"** (forcing a fetch from RAM) is one of the biggest performance killers. We avoid them by writing *cache-friendly* code, which relies on two principles:

1.  **Temporal Locality:** If you use data, you'll probably use it *again* soon.
2.  **Spatial Locality:** If you use data, you'll probably use data *near* it soon.

(The SoA pattern above is a perfect example of *spatial locality*).

### Example 1: Spatial Locality (Row-Major Access)

C++ 2D arrays are stored in **row-major** order. This means `array[row][col]` and `array[row][col+1]` are right next to each other in memory.

```cpp
int matrix[1000][1000];

// GOOD: Spatial Locality
// Accesses memory contiguously:
// [0,0], [0,1], [0,2]... [1,0], [1,1]...
long long sum = 0;
for (int r = 0; r < 1000; ++r) {
    for (int c = 0; c < 1000; ++c) {
        sum += matrix[r][c]; // Cache hits almost every time
    }
}

// BAD: No Spatial Locality
// Accesses memory by jumping:
// [0,0], [1,0], [2,0]... [0,1], [1,1]...
long long sum = 0;
for (int c = 0; c < 1000; ++c) {
    for (int r = 0; r < 1000; ++r) {
        sum += matrix[r][c]; // Cache miss on almost every access!
    }
}
```

The "BAD" version can be **10-50x slower** than the "GOOD" version, *purely* due to cache misses.

### Example 2: Data Structure Choice (std::vector vs. std::list)

This is the single most important cache-related decision you can make.

  * `std::vector`: Stores all elements in **one, big, contiguous block of memory**.

      * **Iteration is perfect for the cache.** When you access element `[i]`, the cache *also* pulls in `[i+1]`, `[i+2]`, etc. (spatial locality). The prefetcher sees this linear access and starts loading data *before you even ask for it*.

  * `std::list`: Stores elements in **separate nodes, scattered all over the heap**.

      * **Iteration is a cache-miss disaster.** To get to the next element, you *pointer-chase* (`node->next`). That next node could be anywhere in RAM. This results in a **cache miss for almost every single element** in the list.

**Rule:** When performance matters, **always prefer contiguous data structures** (like `std::vector`, `std::deque`, or even a flat `std::array`) over node-based ones (like `std::list`, `std::map`, or `std::set`).
