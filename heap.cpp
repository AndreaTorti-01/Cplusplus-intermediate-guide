#include "heap.h"
#include <iostream>
#include <vector>
#include <string>
#include <cassert>
#include <thread>
#include <chrono>
#include <random>
#include <atomic>

void testBasicOperations()
{
    std::cout << "=== Test 1: Basic Operations ===" << std::endl;
    Heap<int> heap;

    // Test adding elements
    heap.Add(5);
    heap.Add(3);
    heap.Add(7);
    heap.Add(1);
    heap.Add(9);
    heap.Add(2);

    std::cout << "Added elements: 5, 3, 7, 1, 9, 2" << std::endl;
    std::cout << "Heap size: " << heap.Size() << std::endl;

    // Note: PopImpl should return the minimum element each time
    std::cout << "Popping elements (should be in ascending order for min-heap):" << std::endl;
    for (int i = 0; i < 6; i++)
    {
        int val = heap.Pop();
        std::cout << "  Popped: " << val << std::endl;
    }
    std::cout << std::endl;
}

void testCopyConstructor()
{
    std::cout << "=== Test 2: Copy Constructor ===" << std::endl;
    Heap<int> heap1;
    heap1.Add(10);
    heap1.Add(5);
    heap1.Add(15);

    Heap<int> heap2(heap1);
    std::cout << "Original heap size: " << heap1.Size() << std::endl;
    std::cout << "Copied heap size: " << heap2.Size() << std::endl;

    std::cout << "Popping from copied heap:" << std::endl;
    while (heap2.Size() > 0)
    {
        std::cout << "  " << heap2.Pop() << std::endl;
    }
    std::cout << "Original heap still has size: " << heap1.Size() << std::endl;
    std::cout << std::endl;
}

void testMoveConstructor()
{
    std::cout << "=== Test 3: Move Constructor ===" << std::endl;
    Heap<int> heap1;
    heap1.Add(20);
    heap1.Add(10);
    heap1.Add(30);

    std::cout << "Original heap size before move: " << heap1.Size() << std::endl;
    Heap<int> heap2(std::move(heap1));
    std::cout << "New heap size after move: " << heap2.Size() << std::endl;
    std::cout << "Original heap size after move: " << heap1.Size() << std::endl;
    std::cout << std::endl;
}

void testCopyAssignment()
{
    std::cout << "=== Test 4: Copy Assignment ===" << std::endl;
    Heap<int> heap1;
    heap1.Add(8);
    heap1.Add(4);
    heap1.Add(12);

    Heap<int> heap2;
    heap2 = heap1;

    std::cout << "After copy assignment:" << std::endl;
    std::cout << "  Heap1 size: " << heap1.Size() << std::endl;
    std::cout << "  Heap2 size: " << heap2.Size() << std::endl;
    std::cout << std::endl;
}

void testMoveAssignment()
{
    std::cout << "=== Test 5: Move Assignment ===" << std::endl;
    Heap<int> heap1;
    heap1.Add(25);
    heap1.Add(15);
    heap1.Add(35);

    Heap<int> heap2;
    heap2 = std::move(heap1);

    std::cout << "After move assignment:" << std::endl;
    std::cout << "  New heap size: " << heap2.Size() << std::endl;
    std::cout << "  Original heap size: " << heap1.Size() << std::endl;
    std::cout << std::endl;
}

void testVectorConstructor()
{
    std::cout << "=== Test 6: Vector Constructor ===" << std::endl;
    std::vector<int> vec = {50, 30, 70, 20, 40, 60, 80};
    Heap<int> heap(vec);

    std::cout << "Created heap from vector: {50, 30, 70, 20, 40, 60, 80}" << std::endl;
    std::cout << "Heap size: " << heap.Size() << std::endl;
    std::cout << std::endl;
}

void testLargeHeap()
{
    std::cout << "=== Test 7: Large Heap ===" << std::endl;
    Heap<int> heap;

    // Add 100 elements in reverse order
    for (int i = 100; i > 0; i--)
    {
        heap.Add(i);
    }

    std::cout << "Added 100 elements (100 down to 1)" << std::endl;
    std::cout << "Heap size: " << heap.Size() << std::endl;

    std::cout << "First 10 elements popped:" << std::endl;
    for (int i = 0; i < 10; i++)
    {
        std::cout << "  " << heap.Pop();
    }
    std::cout << std::endl;
    std::cout << "Remaining heap size: " << heap.Size() << std::endl;
    std::cout << std::endl;
}

void testWithDoubles()
{
    std::cout << "=== Test 8: Heap with Doubles ===" << std::endl;
    Heap<double> heap;

    heap.Add(3.14);
    heap.Add(1.41);
    heap.Add(2.71);
    heap.Add(0.57);
    heap.Add(9.99);

    std::cout << "Added doubles: 3.14, 1.41, 2.71, 0.57, 9.99" << std::endl;
    std::cout << "Popping in order:" << std::endl;
    while (heap.Size() > 0)
    {
        std::cout << "  " << heap.Pop() << std::endl;
    }
    std::cout << std::endl;
}

void testReserve()
{
    std::cout << "=== Test 9: Reserve Capacity ===" << std::endl;
    Heap<int> heap;
    heap.Reserve(1000);

    std::cout << "Reserved capacity for 1000 elements" << std::endl;

    for (int i = 0; i < 50; i++)
    {
        heap.Add(i * 2);
    }

    std::cout << "Added 50 elements" << std::endl;
    std::cout << "Heap size: " << heap.Size() << std::endl;
    std::cout << "First 5 elements: ";
    for (int i = 0; i < 5; i++)
    {
        std::cout << heap.Pop() << " ";
    }
    std::cout << std::endl
              << std::endl;
}

void testMixedOperations()
{
    std::cout << "=== Test 10: Mixed Add/Pop Operations ===" << std::endl;
    Heap<int> heap;

    heap.Add(10);
    heap.Add(5);
    std::cout << "Added 10, 5" << std::endl;
    std::cout << "Pop: " << heap.Pop() << std::endl;

    heap.Add(3);
    heap.Add(8);
    std::cout << "Added 3, 8" << std::endl;
    std::cout << "Pop: " << heap.Pop() << std::endl;

    heap.Add(1);
    heap.Add(15);
    heap.Add(7);
    std::cout << "Added 1, 15, 7" << std::endl;

    std::cout << "Remaining elements:" << std::endl;
    while (heap.Size() > 0)
    {
        std::cout << "  " << heap.Pop() << std::endl;
    }
    std::cout << std::endl;
}

void testConcurrentAdds()
{
    std::cout << "=== Test 11: Concurrent Adds (Multiple Threads) ===" << std::endl;
    Heap<int> heap;
    const int NUM_THREADS = 4;
    const int ADDS_PER_THREAD = 25;

    std::vector<std::thread> threads;

    auto addTask = [&heap](int threadId, int count)
    {
        for (int i = 0; i < count; i++)
        {
            heap.Add(threadId * 1000 + i);
        }
    };

    std::cout << "Starting " << NUM_THREADS << " threads, each adding " << ADDS_PER_THREAD << " elements..." << std::endl;

    for (int i = 0; i < NUM_THREADS; i++)
    {
        threads.emplace_back(addTask, i, ADDS_PER_THREAD);
    }

    for (auto &t : threads)
    {
        t.join();
    }

    std::cout << "All threads completed" << std::endl;
    std::cout << "Final heap size: " << heap.Size() << " (expected: " << (NUM_THREADS * ADDS_PER_THREAD) << ")" << std::endl;
    std::cout << "First 10 elements popped:" << std::endl;
    for (int i = 0; i < 10 && heap.Size() > 0; i++)
    {
        std::cout << "  " << heap.Pop();
    }
    std::cout << std::endl
              << std::endl;
}

void testConcurrentPops()
{
    std::cout << "=== Test 12: Concurrent Pops (Multiple Threads) ===" << std::endl;
    Heap<int> heap;

    // Pre-populate the heap
    const int TOTAL_ELEMENTS = 100;
    for (int i = 0; i < TOTAL_ELEMENTS; i++)
    {
        heap.Add(i);
    }

    std::cout << "Pre-populated heap with " << TOTAL_ELEMENTS << " elements" << std::endl;

    const int NUM_THREADS = 4;
    std::vector<std::thread> threads;
    std::atomic<int> totalPopped(0);

    auto popTask = [&heap, &totalPopped](int threadId, int count)
    {
        int popped = 0;
        for (int i = 0; i < count; i++)
        {
            if (heap.Size() > 0)
            {
                heap.Pop();
                popped++;
            }
        }
        totalPopped += popped;
        std::cout << "  Thread " << threadId << " popped " << popped << " elements" << std::endl;
    };

    std::cout << "Starting " << NUM_THREADS << " threads to pop elements..." << std::endl;

    for (int i = 0; i < NUM_THREADS; i++)
    {
        threads.emplace_back(popTask, i, 25);
    }

    for (auto &t : threads)
    {
        t.join();
    }

    std::cout << "Total elements popped: " << totalPopped.load() << std::endl;
    std::cout << "Remaining heap size: " << heap.Size() << std::endl;
    std::cout << std::endl;
}

void testConcurrentMixedOperations()
{
    std::cout << "=== Test 13: Concurrent Mixed Operations ===" << std::endl;
    Heap<int> heap;

    // Pre-populate
    for (int i = 0; i < 50; i++)
    {
        heap.Add(i);
    }

    std::cout << "Initial heap size: " << heap.Size() << std::endl;

    const int NUM_THREADS = 6;
    std::vector<std::thread> threads;
    std::atomic<int> totalAdded(0);
    std::atomic<int> totalPopped(0);

    auto mixedTask = [&heap, &totalAdded, &totalPopped](int threadId)
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 1);

        for (int i = 0; i < 20; i++)
        {
            if (dis(gen) == 0)
            {
                // Add
                heap.Add(threadId * 10000 + i);
                totalAdded++;
            }
            else
            {
                // Pop (if not empty)
                if (heap.Size() > 0)
                {
                    heap.Pop();
                    totalPopped++;
                }
            }
            std::this_thread::sleep_for(std::chrono::microseconds(10));
        }
    };

    std::cout << "Starting " << NUM_THREADS << " threads doing random adds/pops..." << std::endl;

    for (int i = 0; i < NUM_THREADS; i++)
    {
        threads.emplace_back(mixedTask, i);
    }

    for (auto &t : threads)
    {
        t.join();
    }

    std::cout << "Operations completed:" << std::endl;
    std::cout << "  Total added: " << totalAdded.load() << std::endl;
    std::cout << "  Total popped: " << totalPopped.load() << std::endl;
    std::cout << "  Final heap size: " << heap.Size() << std::endl;
    std::cout << "  Expected size: " << (50 + totalAdded.load() - totalPopped.load()) << std::endl;
    std::cout << std::endl;
}

void testHighContentionScenario()
{
    std::cout << "=== Test 14: High Contention Stress Test ===" << std::endl;
    Heap<int> heap;

    const int NUM_THREADS = 8;
    const int OPS_PER_THREAD = 1000;
    std::vector<std::thread> threads;
    std::atomic<int> errors(0);

    auto stressTask = [&heap, &errors](int threadId, int ops)
    {
        try
        {
            for (int i = 0; i < ops; i++)
            {
                if (i % 2 == 0)
                {
                    heap.Add(threadId * 100000 + i);
                }
                else if (heap.Size() > 0)
                {
                    heap.Pop();
                }
            }
        }
        catch (...)
        {
            errors++;
        }
    };

    std::cout << "Starting " << NUM_THREADS << " threads with " << OPS_PER_THREAD << " operations each..." << std::endl;

    auto startTime = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < NUM_THREADS; i++)
    {
        threads.emplace_back(stressTask, i, OPS_PER_THREAD);
    }

    for (auto &t : threads)
    {
        t.join();
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

    std::cout << "Stress test completed in " << duration.count() << " ms" << std::endl;
    std::cout << "Errors encountered: " << errors.load() << std::endl;
    std::cout << "Final heap size: " << heap.Size() << std::endl;
    std::cout << std::endl;
}

int main()
{
    std::cout << "========================================" << std::endl;
    std::cout << "      HEAP CLASS COMPREHENSIVE TEST     " << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;

    testBasicOperations();
    testCopyConstructor();
    testMoveConstructor();
    testCopyAssignment();
    testMoveAssignment();
    testVectorConstructor();
    testLargeHeap();
    testWithDoubles();
    testReserve();
    testMixedOperations();

    // Concurrency tests
    std::cout << "========================================" << std::endl;
    std::cout << "      CONCURRENCY TESTS                 " << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;

    testConcurrentAdds();
    testConcurrentPops();
    testConcurrentMixedOperations();
    testHighContentionScenario();

    std::cout << "========================================" << std::endl;
    std::cout << "      ALL TESTS COMPLETED SUCCESSFULLY  " << std::endl;
    std::cout << "========================================" << std::endl;

    return 0;
}
