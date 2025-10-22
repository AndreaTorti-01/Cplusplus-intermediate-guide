# STL Containers Definitive Cheatsheet

**Note:** All the complexities below are for the **average case**.

{% raw %}

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

> [This toy orderbook](https://github.com/AndreaTorti-01/Cplusplus-intermediate-guide/blob/main/orderbook.cpp) uses some of them

> [This toy class](https://github.com/AndreaTorti-01/Cplusplus-intermediate-guide/blob/main/heap.h) implements a basic thread-safe min-heap

{% endraw %}