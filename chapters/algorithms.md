# STL Algorithms Definitive Cheatsheet

**Note:** Complexities below are for the average case and assume sane comparators/hash; many algorithms operate on ranges denoted as `[first, last)`.

---

### Legend

- Iterator requirements: I = Input, F = Forward, B = Bidirectional, R = Random Access, O = Output
- Stability: S = stable order preserved, ✖ = not stable

---

### Non‑modifying Sequence Operations

| Algorithm | Complexity | Notes | Iterators |
| :--- | :--- | :--- | :--- |
| `all_of`, `any_of`, `none_of` | $${\color{green}O(n)}$$ | Predicates over range | I |
| `for_each` | $${\color{green}O(n)}$$ | Applies function to each element | I |
| `find`, `find_if`, `find_if_not` | $${\color{green}O(n)}$$ | Linear search | I |
| `count`, `count_if` | $${\color{green}O(n)}$$ | Count matches | I |
| `mismatch` | $${\color{green}O(n)}$$ | First differing pair | I |
| `equal` | $${\color{green}O(n)}$$ | Range equality | I |
| `search`, `search_n` | $${\color{green}O(n \cdot m)}$$ | Subsequence search (naïve) | F |
| `adjacent_find` | $${\color{green}O(n)}$$ | First adjacent equal/predicate | F |
| `is_sorted`, `is_sorted_until` | $${\color{green}O(n)}$$ | Check sortedness | F |

---

### Modifying Sequence Operations

| Algorithm | Complexity | Notes | Iterators |
| :--- | :--- | :--- | :--- |
| `copy`, `copy_if` | $${\color{green}O(n)}$$ | Copy range (with filter) | I→O |
| `move`, `move_backward` | $${\color{green}O(n)}$$ | Move elements; backward for overlapping ranges | I/B→O |
| `transform` | $${\color{green}O(n)}$$ | Map 1 or 2 ranges to output | I→O |
| `fill`, `fill_n` | $${\color{green}O(n)}$$ | Assign value(s) | O |
| `generate`, `generate_n` | $${\color{green}O(n)}$$ | Fill via generator | O |
| `replace`, `replace_if`, `replace_copy(_if)` | $${\color{green}O(n)}$$ | In‑place or copy | F (copy: I→O) |
| `remove`, `remove_if` | $${\color{green}O(n)}$$ | Returns new logical end; use erase‑remove idiom | F |
| `unique`, `unique_copy` | $${\color{green}O(n)}$$ | Remove adjacent dups; requires sorted for true dedup | F (copy: I→O) |
| `swap_ranges` | $${\color{green}O(n)}$$ | Swap two ranges | F |
| `reverse`, `reverse_copy` | $${\color{green}O(n)}$$ | Reverse order | B (copy: I→O) |
| `rotate`, `rotate_copy` | $${\color{green}O(n)}$$ | Cyclic shift | F (copy: I→O) |
| `shuffle` | $${\color{green}O(n)}$$ | Uniform shuffle (needs RNG) | R |

> Erase‑remove idiom: `v.erase(std::remove(v.begin(), v.end(), x), v.end());`

---

### Partitioning

| Algorithm | Complexity | Stability | Notes | Iterators |
| :--- | :--- | :---: | :--- | :--- |
| `partition` | $${\color{green}O(n)}$$ | ✖ | True partition, predicate holds first | F |
| `stable_partition` | $${\color{orange}O(n \log n)}$$ | S | Preserves relative order; can use extra memory to get linear time | F |
| `is_partitioned`, `partition_point` | $${\color{green}O(n)}$$ / $${\color{green}O(\log n)}$$ | — | Check / find boundary on partitioned range | F |

---

### Sorting and Order Statistics

| Algorithm | Complexity | Stability | Notes | Iterators |
| :--- | :--- | :---: | :--- | :--- |
| `sort` | $${\color{orange}O(n \log n)}$$ | ✖ | Introsort; average/worst n log n | R |
| `stable_sort` | $${\color{orange}O(n \log n)}$$ | S | May use extra memory | R |
| `partial_sort` | $${\color{orange}O(n \log k)}$$ | S | Smallest k sorted in place | R |
| `nth_element` | $${\color{green}O(n)}$$ avg | ✖ | Places nth as if sorted; partitions around it | R |
| `inplace_merge` | $${\color{orange}O(n)}$$ | S | Merge two adjacent sorted ranges | B |

Binary search family (on sorted ranges):

| Algorithm | Complexity | Notes | Iterators |
| :--- | :--- | :--- | :--- |
| `binary_search` | $${\color{orange}O(\log n)}$$ | Exists? | F |
| `lower_bound`, `upper_bound` | $${\color{orange}O(\log n)}$$ | First ≥/first > | F |
| `equal_range` | $${\color{orange}O(\log n)}$$ | Pair of bounds | F |

---

### Heap Algorithms (Binary Heap on `std::vector`)

| Algorithm | Complexity | Notes | Iterators |
| :--- | :--- | :--- | :--- |
| `make_heap` | $${\color{green}O(n)}$$ | Heapify | R |
| `push_heap`, `pop_heap` | $${\color{orange}O(\log n)}$$ | Adjust after push/pop | R |
| `sort_heap` | $${\color{orange}O(n \log n)}$$ | Heapsort | R |
| `is_heap`, `is_heap_until` | $${\color{green}O(n)}$$ | Check heap property | R |

---

### Set Algorithms (on Sorted Ranges)

| Algorithm | Complexity | Notes | Iterators |
| :--- | :--- | :--- | :--- |
| `merge` | $${\color{green}O(n)}$$ | Merge two sorted ranges | I→O |
| `includes` | $${\color{green}O(n)}$$ | Subset check | I |
| `set_union` | $${\color{green}O(n)}$$ | Union into output | I→O |
| `set_intersection` | $${\color{green}O(n)}$$ | Intersection into output | I→O |
| `set_difference` | $${\color{green}O(n)}$$ | A−B | I→O |
| `set_symmetric_difference` | $${\color{green}O(n)}$$ | XOR of sets | I→O |

Here, n = `distance(a) + distance(b)`.

---

### Min/Max and Comparisons

| Algorithm | Complexity | Notes | Iterators |
| :--- | :--- | :--- | :--- |
| `min_element`, `max_element`, `minmax_element` | $${\color{green}O(n)}$$ | Return iterator(s) | I |
| `lexicographical_compare` | $${\color{green}O(n)}$$ | Dictionary order | I |
| `clamp` | $${\color{green}O(1)}$$ | Bound a value to [lo, hi] | — |

---

### Numeric Algorithms (in `<numeric>`)

| Algorithm | Complexity | Notes |
| :--- | :--- | :--- |
| `accumulate` | $${\color{green}O(n)}$$ | Fold with initial value |
| `inner_product` | $${\color{green}O(n)}$$ | Dot product (custom ops allowed) |
| `partial_sum` | $${\color{green}O(n)}$$ | Prefix sums |
| `adjacent_difference` | $${\color{green}O(n)}$$ | Differences between neighbors |
| `reduce` (C++17) | $${\color{green}O(n)}$$ | Like `accumulate` (assoc/commutative expected) |
| `exclusive_scan` / `inclusive_scan` (C++17) | $${\color{green}O(n)}$$ | Parallel‑friendly scans |
| `transform_reduce` (C++17) | $${\color{green}O(n)}$$ | Map‑reduce in one pass |

---

### Execution Policies (C++17)

| Policy | What it means | Side‑effects |
| :--- | :--- | :--- |
| `std::execution::seq` | Sequential | Allowed |
| `std::execution::par` | Parallel threads | Functors must be thread‑safe; exceptions aggregated |
| `std::execution::par_unseq` | Parallel + vectorized | No inter‑iteration dependencies/UB; no throwing |

- Work complexity is unchanged; policies may improve runtime on large data.
- Avoid data races: no writes to the same element from multiple iterations.

---

### Quick Rules of Thumb

- Need ordering? Use `sort` + `binary_search`/`lower_bound` for $${\color{orange}O(\log n)}$$ queries.
- Deduplicate: `sort`, `unique`, then `erase` trailing tail for true dedup.
- Select kth (median/percentile): `nth_element` for linear average time.
- Removing by predicate from vector: `erase(remove_if(...), end)`.
- Heaps for streaming top‑k: maintain with `push_heap` / `pop_heap`.

---

> Examples from this repo:
>
> - Heap ops in [`heap.cpp`](../heap.cpp)
> - Iterator patterns in [`iterators.cpp`](../iterators.cpp)
> - Move/transform patterns in [`move.cpp`](../move.cpp)
