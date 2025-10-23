# Miscellaneous Notes

## Mind your Doubles and Floats
* `5.5` → `double`
* `5.5f` → `float`

## Where are my pure functions? I miss my java interfaces...

```cpp
virtual MyCoolMethod(T myArg) = 0 // implements a pure function (interface) in C++
```

## Virtual Destructors

Always mark base class destructors as `virtual` when a class is meant to be derived from:

```cpp
class Base {
public:
    Base() = default;
    
    // BAD: Non-virtual destructor in a base class
    ~Base() {
        std::cout << "Base destroyed\n";
    }
};

class Derived : public Base {
private:
    int* data;
public:
    Derived() : data(new int(42)) {}
    
    // Will never be called if deleted through Base pointer
    ~Derived() {
        delete data; // Memory leak if Base::~Base() is not virtual!
        std::cout << "Derived destroyed\n";
    }
};

// Incorrect usage:
void incorrectWay() {
    Base* ptr = new Derived();
    delete ptr; // MEMORY LEAK! Only ~Base() gets called, not ~Derived()
}

// The fix:
class FixedBase {
public:
    FixedBase() = default;
    
    // GOOD: Virtual destructor allows proper cleanup
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
        delete data; // Now properly called
        std::cout << "FixedDerived destroyed\n";
    }
};
```

**Rule of thumb**:
* **Virtual** functions (and destructors) → **resolved by *dynamic type*** at **runtime** (via vtable).
* **Non-virtual** functions (and destructors) → **resolved by *static type*** at **compile time**.

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

## Explicit Constructors

The `explicit` keyword prevents implicit conversions in constructors that take a single argument:

```cpp
class Implicit {
public:
    // Allows implicit conversion from int to Implicit
    Implicit(int value) : m_value(value) {}
    int getValue() const { return m_value; }
private:
    int m_value;
};

class Explicit {
public:
    // Requires explicit construction from int to Explicit
    explicit Explicit(int value) : m_value(value) {}
    int getValue() const { return m_value; }
private:
    int m_value;
};

void process(const Implicit& obj) { std::cout << obj.getValue() << std::endl; }
void process(const Explicit& obj) { std::cout << obj.getValue() << std::endl; }

int main() {
    process(Implicit(42));  // Works fine: explicit construction
    process(42);            // Also works: implicit conversion from int to Implicit
    
    process(Explicit(42));  // Works fine: explicit construction
    // process(42);         // Error: cannot convert 'int' to 'const Explicit&'
    process(static_cast<Explicit>(42)); // Works: explicit cast
}
```

**Best practice:** Use `explicit` for constructors taking a single argument to prevent unexpected implicit conversions and make your code's intent clearer.
