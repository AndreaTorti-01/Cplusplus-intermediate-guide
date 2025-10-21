# C++ - An Intermediate Guide by Andrea Torti

A concise, practical guide for learning and experimenting with modern C++. Includes theory, code snippets, and small runnable projects

> Check out [The Cherno's C++ series](https://youtube.com/playlist?list=PLlrATfBNZ98dudnM48yfGUldqGD0S4FFb)

## Compilation Basics

The **compiler** only compiles **source files** (usually `.cpp`) into **object files** (`.obj` or `.o`).

Header files (`.h` or `.hpp`) are *optional* — they are **not compiled directly**, but rather **included** (literally copy-pasted) into source files.

Header contents (like function declarations) *could* be written directly in source files, though that’s bad practice for maintainability.

---

## Preprocessor

Preprocessor statements modify the source **before** compilation.
They always start with `#`.

Common examples:

```cpp
#if / #else / #endif
#include <iostream>
#define X 100
#undef X
#ifdef X
#ifndef X
#pragma once  // compiler-specific
```

* `#define` and `#undef` define or remove **macros**.
* `#ifdef` / `#ifndef` test whether a macro is defined.
* `#pragma` is **compiler-specific**, often used for optimization hints or include guards.

---

## Translation Units

* A **source file** is a `.cpp` file containing your C++ code.
* A **translation unit** is the *final product* after the preprocessor expands all `#include`s.
  It includes your `.cpp` plus everything it transitively includes.

---

## Static, Extern, and Linkage

### `static`

* **At file scope** → visible only inside its translation unit (the linker cannot see it).

  ```cpp
  static void helper() {}
  ```
* **Inside a class** → shared across all instances.

  ```cpp
  struct Foo { static int count; };
  ```
* Common convention: `s_variable` for static members.

Use `static` generously — it’s like making things `private` at file level.
It avoids namespace pollution and linking errors.

---

### `extern`

Declares that a variable or function **exists in another translation unit**.

```cpp
// file1.cpp
int counter = 0;

// file2.cpp
extern int counter;  // declaration only
```

This lets the linker match them later.

#### `extern "C"`

Disables C++ **name mangling**, making symbols compatible with C code or other languages.

```cpp
// mylib.h
#ifdef __cplusplus
extern "C" {
#endif

void initialize();
int compute(int x);

#ifdef __cplusplus
}
#endif
```

```cpp
// mylib.cpp
extern "C" void initialize() {
    // implementation
}

extern "C" int compute(int x) {
    return x * 2;
}
```

Use `extern "C"` when:
* Exposing C++ functions to C code
* Creating plugins or DLLs with stable ABIs
* Interfacing with other languages (Python, Rust, etc.)

---

## Inline, Templates, and Constants in Headers

### Inline Functions

Must be defined in headers — the compiler needs to **see the full body** to inline it.

### Templates

Also require full definitions in headers, because the compiler generates code for each type at compile time.

### Constants

`const`, `constexpr`, and `enum` values are fine in headers — they don’t allocate separate storage for each translation unit.

### Include Guards

Always protect headers from multiple inclusion:

```cpp
#ifndef HEAP_H
#define HEAP_H

// content

#endif
```

Or use:

```cpp
#pragma once
```

---

## Linking

* **Static linking**: object code is copied into the executable at build time.
* **Dynamic linking**: executable loads shared libraries (`.dll`, `.so`) at runtime.

---

## Miscellaneous Notes

* `5.5` → `double`
* `5.5f` → `float`

---

## Performance Notes

* **Branching (`if`)** is slow in tight loops — avoid in hot paths if possible.
  → Prefer *branchless programming* (math tricks or bitwise ops).
* **Branch predictor** helps, but mispredictions are costly.

---

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

> [Example usage of move with a vector](move.cpp)

---

### Rule of Five

If you define any of these, you should probably define all:

1. Destructor
2. Copy constructor
3. Move constructor
4. Copy assignment operator
5. Move assignment operator

Or use `= default` / `= delete` to be explicit about your intent.


---

## Implementation Details

Methods inside classes just have a **hidden parameter** — the `this` pointer.
It’s not magic; it’s just syntactic sugar for passing the instance as an argument.

```cpp
obj.method(); // actually like: method(&obj);
```

---
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

## Bit Fields Basics

Pack several boolean or small integer values into one unsigned integer.

Example:

```cpp
struct Status {
    unsigned ready : 1; // 0 or 1
    unsigned error : 1; // 0 or 1
    unsigned code  : 6; // 0..63
    unsigned retry : 4; // 0..15
};

void use() {
    Status s{};      // zero-initialized
    s.ready = 1;
    s.code  = 42;
    // &s.ready;     // not allowed: bit-fields have no address
}
```

Notes:
- Use unsigned base types; the number after : is the bit width.
- Do not rely on exact memory layout or bit ordering; it varies by compiler/ABI.
- Bit-fields are good for in-process packing, not for files or network data.

---

## Smart Pointers

Modern ownership tools that automate lifetime with minimal ceremony.

### Types and Ownership

- std::unique_ptr<T>
  - Sole ownership, move-only. Size is typically one pointer (8 bytes on 64-bit) with default deleter.
  - With a stateful/custom deleter, size may increase unless the deleter is empty and benefits from EBO.

- std::shared_ptr<T>
  - Shared ownership via a control block (strong/weak counts, deleter, allocator).
  - Copying increments an atomic refcount; destruction decrements it.
  - Typical size on 64-bit: 16 bytes (two pointers: object and control block).
  - One extra heap allocation for the control block unless using make_shared/allocate_shared.

- std::weak_ptr<T>
  - Non-owning observer that refers to the same control block.
  - lock() returns a std::shared_ptr if the object is still alive.

### Factory Helpers and Why

- std::make_unique<T>(args...)
  - Exception-safe construction without exposing new.
  - Prevents evaluation-order leaks and is shorter/clearer.
  - C++14 and later supports arrays: std::make_unique<T[]>(n).

- std::make_shared<T>(args...) / std::allocate_shared<T>(alloc, args...)
  - Single allocation for object + control block → fewer allocations, better locality.
  - Trade-off: with make_shared, memory for the control block cannot be freed until all weak_ptrs expire, so the object’s memory may be retained longer than with separate allocations.

### Overhead

- std::unique_ptr
  - No refcounting; essentially free at runtime (same cost as raw pointer) for default deleter.
  - Size can be >1 pointer if the deleter has state or is a function pointer.

- std::shared_ptr
  - Object: usually 16 bytes on 64-bit (pointer to object + pointer to control block).
  - Control block: holds two atomics (strong/weak), deleter, type-erased state; tens of bytes.
  - Operations that copy/reset incur atomic increments/decrements; measurable under contention.

- std::weak_ptr
  - Similar size to shared_ptr (typically 16 bytes on 64-bit).
  - lock() incurs atomic operations.

### Usage Patterns

- Prefer std::unique_ptr by default.
  - Return unique_ptr from factories to express exclusive ownership.
  - Transfer ownership by passing unique_ptr by value.
  - Use unique_ptr<T[]> for arrays.

- Use std::shared_ptr only when ownership is genuinely shared.
  - Pass by value to share ownership; pass T&/T* for non-owning access.
  - Avoid cyclical shared_ptr graphs; break cycles with std::weak_ptr.
    - Example: doubly linked nodes → next as shared_ptr, prev as weak_ptr.

- Observers:
  - Non-owning parameters: use T* or T& instead of shared_ptr to avoid unintended ownership.

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

> [Iterators Usage Example](iterators.cpp)

## STL Containers

**Note:** All the complexities below are for the **average case**.

## Sequence Containers

| Container | Implementation | Search (by value) | Insertion | Removal | Indexable? |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **`std::vector`** | Dynamic Array | $${\color{red}O(n)}$$| **End:**$${\color{green}O(1) \text{ amort.}}$$<br>**Middle:** $${\color{red}O(n)}$$| **End:**$${\color{green}O(1)}$$<br>**Middle:** $${\color{red}O(n)}$$ | ✅ |
| **`std::deque`** | List of Arrays | $${\color{red}O(n)}$$| **Front/Back:**$${\color{green}O(1) \text{ amort.}}$$<br>**Middle:** $${\color{red}O(n)}$$| **Front/Back:**$${\color{green}O(1)}$$<br>**Middle:** $${\color{red}O(n)}$$ | ✅ |
| **`std::list`** | Doubly-Linked List | $${\color{red}O(n)}$$|$${\color{green}O(1)}$$|$${\color{green}O(1)}$$ | ❌ |
| **`std::forward_list`** | Singly-Linked List | $${\color{red}O(n)}$$|$${\color{green}O(1)}$$|$${\color{green}O(1)}$$ | ❌ |
| **`std::array`** | Static Array | $${\color{red}O(n)}$$ | N/A (fixed size) | N/A (fixed size) | ✅ |

-----

## Associative Containers (Ordered)

| Container | Implementation | Search (by key) | Insertion | Removal | Indexable? |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **`std::set`** | Balanced BST¹ | $${\color{orange}O(\log n)}$$|$${\color{orange}O(\log n)}$$|$${\color{orange}O(\log n)}$$ | ❌ |
| **`std::map`** | Balanced BST¹ | $${\color{orange}O(\log n)}$$|$${\color{orange}O(\log n)}$$|$${\color{orange}O(\log n)}$$ | ✅ |
| **`std::multiset`** | Balanced BST¹ | $${\color{orange}O(\log n)}$$|$${\color{orange}O(\log n)}$$|$${\color{orange}O(\log n)}$$ | ❌ |
| **`std::multimap`** | Balanced BST¹ | $${\color{orange}O(\log n)}$$|$${\color{orange}O(\log n)}$$|$${\color{orange}O(\log n)}$$ | ❌ |

¹ *Typically a Red-Black Tree.*

-----

## Unordered Associative Containers (Hashed)

| Container | Implementation | Search (by key) | Insertion | Removal | Indexable? |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **`std::unordered_set`** | Hash Table | $${\color{green}O(1)}$$|$${\color{green}O(1)}$$|$${\color{green}O(1)}$$ | ❌ |
| **`std::unordered_map`** | Hash Table | $${\color{green}O(1)}$$|$${\color{green}O(1)}$$|$${\color{green}O(1)}$$ | ✅ |
| **`std::unordered_multiset`** | Hash Table | $${\color{green}O(1)}$$|$${\color{green}O(1)}$$|$${\color{green}O(1)}$$ | ❌ |
| **`std::unordered_multimap`** | Hash Table | $${\color{green}O(1)}$$|$${\color{green}O(1)}$$|$${\color{green}O(1)}$$ | ❌ |

-----

## Container Adapters

| Adapter | Default Container | Implementation | Insertion (`push`) | Removal (`pop`) | Access (`top`/`front`) |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **`std::stack`** | `std::deque` | LIFO | $${\color{green}O(1)}$$|$${\color{green}O(1)}$$|$${\color{green}O(1)}$$ |
| **`std::queue`** | `std::deque` | FIFO | $${\color{green}O(1)}$$|$${\color{green}O(1)}$$|$${\color{green}O(1)}$$ |
| **`std::priority_queue`** | `std::vector` | Binary Heap | $${\color{orange}O(\log n)}$$|$${\color{orange}O(\log n)}$$|$${\color{green}O(1)}$$ |

> [This toy orderbook](orderbook.cpp) uses some of them

> [This toy class](heap.h) implements a basic thread-safe min-heap

## TODO

* RVO / NRVO
* Networking in C++ & OSI lower levels recap
* Cache and paging
