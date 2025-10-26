# Miscellaneous Notes
## Mind your Doubles and Floats
- Use `5.5` for a `double`.
- Use `5.5f` for a `float`.

---

## Where are my pure functions? I miss my Java interfaces...
A pure virtual function in C++ provides an interface-like contract:

```cpp
// Pure virtual (interface) example
template<typename T>
struct IMyInterface {
    virtual void MyCoolMethod(T myArg) = 0; // pure virtual
    virtual ~IMyInterface() = default;
};
```

---

## Virtual Destructors
Always make base class destructors `virtual` when a class is designed to be derived from.

```cpp
#include <iostream>

class Base {
public:
    Base() = default;
    // BAD: non-virtual destructor in a base class
    ~Base() {
        std::cout << "Base destroyed\n";
    }
};

class Derived : public Base {
private:
    int* data;
public:
    Derived() : data(new int(42)) {}
    ~Derived() {
        delete data; // Will NOT be called if deleted via Base*
        std::cout << "Derived destroyed\n";
    }
};

void incorrectWay() {
    Base* ptr = new Derived();
    delete ptr; // MEMORY LEAK: only ~Base() is invoked
}

// The fix:
class FixedBase {
public:
    FixedBase() = default;
    // GOOD: virtual destructor allows proper cleanup
    virtual ~FixedBase() {
        std::cout << "FixedBase destroyed\n";
    }
};

class FixedDerived : public FixedBase {
private:
    int* data;
public:
    FixedDerived() : data(new int(42)) {}
    ~FixedDerived() override {
        delete data; // Properly called
        std::cout << "FixedDerived destroyed\n";
    }
};
```

**Rule of thumb**
- **Virtual** functions/destructors → resolved by *dynamic type* at runtime (vtable).
- **Non-virtual** functions/destructors → resolved by *static type* at compile time.

---

## Bit Fields Basics
Pack several boolean or small integer values into a single unsigned integer.

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
- Use unsigned base types; the number after `:` is the bit width.
- Do not rely on exact memory layout or bit ordering; it varies by compiler/ABI.
- Bit-fields are good for in-process packing, not for files or network formats.

---

## Explicit Constructors
`explicit` prevents implicit conversions for single-argument constructors.

```cpp
#include <iostream>

class Implicit {
public:
    Implicit(int value) : m_value(value) {}
    int getValue() const { return m_value; }
private:
    int m_value;
};

class Explicit {
public:
    explicit Explicit(int value) : m_value(value) {}
    int getValue() const { return m_value; }
private:
    int m_value;
};

void process(const Implicit& obj) { std::cout << obj.getValue() << '\n'; }
void process(const Explicit& obj) { std::cout << obj.getValue() << '\n'; }

int main() {
    process(Implicit(42));         // explicit construction
    process(42);                   // implicit conversion to Implicit

    process(Explicit(42));         // explicit construction
    // process(42);                // ERROR: cannot convert 'int' to 'const Explicit&'
    process(static_cast<Explicit>(42)); // OK: explicit cast
}
```

Best practice: mark single-argument constructors `explicit` to avoid accidental implicit conversions.

---

## Small String Optimization
Many `std::string` implementations store short strings inside the string object (on the stack) to avoid heap allocation. Internals are typically implemented using a union-like layout:

```cpp
// illustrative only — real std::string is implementation-defined
class string {
    size_t capacity;
    union {
        struct {
            char* ptr;
            size_t size;
        } heapbuf;
        char stackbuf[sizeof(heapbuf)];
    };
};
```

Notes:
- Threshold for SSO is implementation-defined.
- Do not rely on SSO behavior in portability-sensitive code.
- SSO improves performance for short strings by avoiding heap allocations.
