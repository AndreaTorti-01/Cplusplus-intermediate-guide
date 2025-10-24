# Object-Oriented Programming in C++

A quick, example-first overview of core OOP principles in modern C++. Starting with the *Four Pillars*.
Throughout, we add Java comparisons — Java is often considered the “king” of OOP languages, so it’s a helpful yardstick.

---

## 1: Encapsulation

Hide data behind a clear API and keep invariants inside the class.

```cpp
#include <stdexcept>

class BankAccount {
private:
    double balance_ = 0.0;  // hidden state

public:
    explicit BankAccount(double initial)
        : balance_(initial) {}

    void deposit(double amt) {
        if (amt < 0) {
            throw std::invalid_argument("negative deposit");
        }
        balance_ += amt;
    }

    void withdraw(double amt) {
        if (amt < 0 || amt > balance_) {
            throw std::invalid_argument("invalid withdraw");
        }
        balance_ -= amt;
    }

    double balance() const {
        return balance_;
    }
};
```

Key points:
- Keep members private; expose intent via methods.
- Enforce class invariants at the boundary (constructors/methods).

---

## 2: Abstraction

Program to interfaces; hide implementation details behind pure virtual functions.

```cpp
#include <cmath>
#include <memory>

struct Shape {  // interface (abstract base)
    virtual ~Shape() = default;
    virtual double area() const = 0;
};

class Circle : public Shape {
private:
    double r_;

public:
    explicit Circle(double r)
        : r_(r) {}

    double area() const override {
        return M_PI * r_ * r_;
    }
};

// uses only the abstract interface
inline double print_area(const Shape& s) {
    return s.area();
}
```

Java comparison:
- This maps directly to Java interfaces and implementing classes:

```java
interface Shape {
    double area();
}

final class Circle implements Shape {
    private final double r;

    Circle(double r) { this.r = r; }

    @Override
    public double area() { return Math.PI * r * r; }
}
```
- C++ has abstract base classes with pure virtuals; Java has interfaces/abstract classes. Both use `override`/`@Override` to guarantee correct overriding. Java interfaces can have default methods; C++ has no direct equivalent but can use base class implementations.

> [More about that Base Class virtual Destructor](/chapters/miscellaneous?id=virtual-destructors)

---

## 3: Inheritance (is-a)

Use to model substitutable specializations of a common interface.

```cpp
struct Animal {
    virtual ~Animal() = default;

    virtual const char* speak() const {
        return "...";
    }
};

struct Dog : Animal {
    const char* speak() const override {
        return "woof";
    }
};

inline const char* say(const Animal& a) {
    return a.speak();
}
```

Tips:
- Prefer public inheritance only for true "is-a" relationships.
- Make base class destructors virtual when used polymorphically.

Java comparison:
- Java has single class inheritance (`extends`) and multiple interface inheritance (`implements`). All methods are virtual by default.
- Mark classes/methods `final` in Java to prevent extension/override; in C++ use `final` on classes/methods similarly.

---

## 4: Polymorphism

### Runtime (virtual dispatch)

```cpp
#include <memory>
#include <vector>

struct Drawable {
    virtual ~Drawable() = default;
    virtual void draw() const = 0;
};

struct Line : Drawable {
    void draw() const override {
        /* ... */
    }
};

struct Circle2 : Drawable {
    void draw() const override {
        /* ... */
    }
};

inline void draw_all(const std::vector<std::unique_ptr<Drawable>>& items) {
    for (const auto& d : items) {
        d->draw();
    }
}
```

Java comparison:
- Java mirrors this exactly with interfaces and a `List<Drawable>`; references are implicit (no pointers):

```java
interface Drawable { void draw(); }

final class Line implements Drawable {
    @Override public void draw() { /* ... */ }
}

final class Circle2 implements Drawable {
    @Override public void draw() { /* ... */ }
}

static void drawAll(java.util.List<Drawable> items) {
    for (var d : items) d.draw();
}
```

### Compile-time (templates / duck typing)

```cpp
// any type with .draw() works (no base class required)
template <typename T>
void draw_one(const T& t) {
    t.draw();
}
```

Java comparison:
- C++ templates are compile-time and generate code per type (no runtime overhead). Java generics are mostly type-erased at runtime.
- Java needs explicit bounds: `static <T extends Drawable> void drawOne(T t) { t.draw(); }` — no duck typing without an interface.

Notes:
- Runtime polymorphism: flexible, uniform interface; small indirection cost.
- Compile-time polymorphism: zero-overhead, checked at compile time.

---

## Composition over Inheritance

Model "has-a" with members; delegate behavior.

```cpp
struct Engine {
    void start() {
        /* spin */
    }
};

class Car {
private:
    Engine engine_;  // composed part

public:
    void start() {
        engine_.start();
    }
};
```

Why: Looser coupling, clearer ownership, fewer fragile base class problems.

---

## SOLID Principles (brief)

### Single Responsibility Principle (SRP)
One reason to change per class.
```cpp
struct Order { /* data & invariants only */ };
struct OrderSerializer { std::string to_json(const Order&) const; };
```

### Open/Closed Principle (OCP)
Open for extension, closed for modification.
```cpp
struct Renderer { virtual ~Renderer()=default; virtual void render() const = 0; };
struct SvgRenderer : Renderer { void render() const override {/* ... */} };
// Add PngRenderer without touching existing code that consumes Renderer
```

### Liskov Substitution Principle (LSP)
Subtypes must be usable via the base without breaking expectations.
```cpp
struct Flyable { virtual ~Flyable()=default; virtual void fly() = 0; };
struct Sparrow : Flyable { void fly() override {/* ok */} };
// Don't derive Penguin : Flyable; it can't fly → violates LSP
// Instead, separate interfaces: Bird (eat, walk) vs Flyable
```

### Interface Segregation Principle (ISP)
Prefer small, specific interfaces over fat ones.
```cpp
struct Printer { virtual ~Printer()=default; virtual void print() = 0; };
struct Scanner { virtual ~Scanner()=default; virtual void scan()  = 0; };
struct MFD : Printer, Scanner { void print() override {/*...*/} void scan() override {/*...*/} };
```

Java comparison:
- Multiple interface inheritance is a core Java strength: `class MFD implements Printer, Scanner { ... }`.

### Dependency Inversion Principle (DIP)
Depend on abstractions; inject concrete deps at the boundary.
```cpp
struct IStorage { virtual ~IStorage()=default; virtual void save(std::string_view) = 0; };

class ReportService {
    IStorage& storage_;
public:
    explicit ReportService(IStorage& s) : storage_(s) {}
    void export_report(std::string_view data) { storage_.save(data); }
};
```

---

## Object Lifetime and Virtual Destructors

When deleting via a base pointer, the base must have a virtual destructor.

```cpp
struct Base {
    virtual ~Base() = default;
};

struct Derived : Base {
    ~Derived() {
        /* release resources */
    }
};

inline void destroy(Base* p) {
    delete p;  // calls Derived::~Derived correctly via virtual dtor
}
```

- In C++, RAII destructors run at scope exit deterministically — powerful for resource safety.

---

## Rule of 0/3/5 (special member functions)

- Rule of 0: Prefer types that manage resources via RAII members (std::string, std::vector) → no custom special members needed.
- Rule of 3: If you need any of destructor/copy-ctor/copy-assign, you likely need all three.
- Rule of 5: Add move-ctor and move-assign when movable.

```cpp
#include <cstring>

class Buffer {
private:
    char*       p_ = nullptr;
    std::size_t n_ = 0;

public:
    Buffer() = default;

    explicit Buffer(std::size_t n)
        : p_(new char[n]{}), n_(n) {}

    ~Buffer() {
        delete[] p_;
    }

    // copy
    Buffer(const Buffer& other)
        : Buffer(other.n_) {
        std::memcpy(p_, other.p_, n_);
    }

    // copy assign
    Buffer& operator=(const Buffer& rhs) {
        if (this != &rhs) {
            Buffer tmp(rhs);
            swap(tmp);
        }
        return *this;
    }

    // move
    Buffer(Buffer&& other) noexcept
        : p_(other.p_), n_(other.n_) {
        other.p_ = nullptr;
        other.n_ = 0;
    }

    // move assign
    Buffer& operator=(Buffer&& rhs) noexcept {
        swap(rhs);
        return *this;
    }

    void swap(Buffer& other) noexcept {
        std::swap(p_, other.p_);
        std::swap(n_, other.n_);
    }
};
```

---

## Object Slicing (avoid)

Assigning a Derived to a Base by value removes the derived part. Use references or pointers for polymorphism.

```cpp
struct B {
    virtual ~B() = default;
    virtual int id() const { return 1; }
};

struct D : B {
    int id() const override { return 2; }
};

inline int f(B b) {
    return b.id();  // BAD: by value → slices
}

inline int g(const B& b) {
    return b.id();  // GOOD: preserves dynamic type
}
```

---

## Composition, Aggregation, Association (quick view)

- Association: knows about another object (non-owning pointer/ref).
- Aggregation: whole-part, lifetime independent (std::vector<T*> parts).
- Composition: whole-part, lifetime tied (member objects).

```cpp
struct Wheel {};
struct Frame {};

struct Bike {                 // composition
    Wheel front, rear;        // Bike owns wheels (same lifetime)
    Frame frame;
};
```

---

## Notes

- Prefer composition and interfaces; use inheritance sparingly.
- Mark overrides with `override`; use `final` to prevent further override if needed.
- Avoid raw `new`/`delete` in user code; prefer smart pointers and RAII.
