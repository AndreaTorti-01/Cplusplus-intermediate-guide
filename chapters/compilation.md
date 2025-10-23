# Compiler, Linker and header files basics

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

### Bonus:

* **Static linking**: object code is copied into the executable at build time.
* **Dynamic linking**: executable loads shared libraries (`.dll`, `.so`) at runtime.

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
