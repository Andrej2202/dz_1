#include "unique_ptr.hpp"
#include "shared_ptr.hpp"
#include <iostream>
#include <cassert>
#include <string>
#include <memory>

using namespace smart_pointers;

class TestObject {
public:
    static int instance_count;
    int value;
    
    TestObject(int v = 0) : value(v) {
        ++instance_count;
    }
    
    TestObject(const TestObject& other) : value(other.value) {
        ++instance_count;
    }
    
    TestObject& operator=(const TestObject& other) {
        if (this != &other) {
            value = other.value;
        }
        return *this;
    }
    
    ~TestObject() {
        --instance_count;
    }
};

int TestObject::instance_count = 0;

struct Animal {
    virtual ~Animal() = default;
    virtual std::string speak() const = 0;
};

struct Dog : Animal {
    std::string speak() const override { return "Woof!"; }
};

struct Cat : Animal {
    std::string speak() const override { return "Meow!"; }
};

struct MoveOnly {
    int value;
    MoveOnly(int v) : value(v) {}
    MoveOnly(const MoveOnly&) = delete;
    MoveOnly& operator=(const MoveOnly&) = delete;
    MoveOnly(MoveOnly&& other) noexcept : value(other.value) { other.value = -1; }
    MoveOnly& operator=(MoveOnly&& other) noexcept {
        if (this != &other) {
            value = other.value;
            other.value = -1;
        }
        return *this;
    }
};


void test_unq_basic() {
    std::cout << "\n=== UnqPtr: basic ===\n";
    {
        UnqPtr<TestObject> p(new TestObject(42));
        assert(p);
        assert(p->value == 42);
        assert((*p).value == 42);
        assert(p.get() != nullptr);
    }
    assert(TestObject::instance_count == 0);
    std::cout << "OK\n";
}

void test_unq_move() {
    std::cout << "\n=== UnqPtr: move ===\n";
    {
        UnqPtr<TestObject> p1(new TestObject(100));
        UnqPtr<TestObject> p2 = std::move(p1);
        assert(!p1);
        assert(p2);
        assert(p2->value == 100);
        
        UnqPtr<TestObject> p3;
        p3 = std::move(p2);
        assert(!p2);
        assert(p3->value == 100);
    }
    assert(TestObject::instance_count == 0);
    std::cout << "OK\n";
}

void test_unq_self_move() {
    std::cout << "\n=== UnqPtr: self move assignment ===\n";
    {
        UnqPtr<TestObject> p(new TestObject(42));
        p = std::move(p);
        assert(p);
        assert(p->value == 42);
    }
    assert(TestObject::instance_count == 0);
    std::cout << "OK\n";
}

void test_unq_release_reset() {
    std::cout << "\n=== UnqPtr: release/reset ===\n";
    {
        UnqPtr<TestObject> p(new TestObject(7));
        TestObject* raw = p.release();
        assert(!p);
        assert(raw->value == 7);
        
        p.reset(raw);
        assert(p);
        assert(p->value == 7);
        
        p.reset(new TestObject(99));
        assert(p->value == 99);
    }
    assert(TestObject::instance_count == 0);
    std::cout << "OK\n";
}

void test_unq_edge_cases() {
    std::cout << "\n=== UnqPtr: edge cases ===\n";
    {
        UnqPtr<TestObject> p;
        p.reset();
        p.reset(nullptr);
        
        TestObject* raw = p.release();
        assert(raw == nullptr);
        
        UnqPtr<TestObject> p2(new TestObject(42));
        TestObject* raw2 = p2.release();
        p2.reset(raw2);  
        assert(p2->value == 42);
        
        p2.reset(nullptr);
        assert(!p2);
    }
    assert(TestObject::instance_count == 0);
    std::cout << "OK\n";
}

void test_unq_swap() {
    std::cout << "\n=== UnqPtr: swap ===\n";
    {
        UnqPtr<TestObject> p1(new TestObject(1));
        UnqPtr<TestObject> p2(new TestObject(2));
        
        p1.swap(p2);
        assert(p1->value == 2);
        assert(p2->value == 1);
        
        
        UnqPtr<TestObject> p3; 
        p1.swap(p3);
        assert(!p1);
        assert(p3);
        assert(p3->value == 2);
        UnqPtr<TestObject> p_empty1; 
        UnqPtr<TestObject> p_empty2; 
        p_empty1.swap(p_empty2);
        assert(!p_empty1);
        assert(!p_empty2);
    }
    assert(TestObject::instance_count == 0);
    std::cout << "OK\n";
}

void test_unq_array() {
    std::cout << "\n=== UnqPtr: raw array ===\n";
    {
        UnqPtr<TestObject[]> arr(new TestObject[5]);
        for (int i = 0; i < 5; ++i) {
            arr[i].value = i * 10;
        }
        assert(arr[0].value == 0);
        assert(arr[4].value == 40);
    }
    assert(TestObject::instance_count == 0);
    std::cout << "OK\n";
}

void test_unq_array_reset() {
    std::cout << "\n=== UnqPtr: array reset ===\n";
    {
        UnqPtr<TestObject[]> arr(new TestObject[3]);
        arr[0].value = 1;
        
        arr.reset();
        assert(!arr);
        
        arr.reset(new TestObject[2]);
        assert(arr);
        arr[0].value = 10;
        arr[1].value = 20;
        assert(arr[0].value == 10);
        assert(arr[1].value == 20);
    }
    assert(TestObject::instance_count == 0);
    std::cout << "OK\n";
}

void test_unq_make() {
    std::cout << "\n=== UnqPtr: make_unq ===\n";
    {
        auto p = make_unq<TestObject>(123);
        assert(p->value == 123);
        
        auto arr = make_unq_arr<TestObject>(3);
        arr[0].value = 1;
        arr[2].value = 3;
        assert(arr[0].value == 1);
        assert(arr[2].value == 3);
    }
    assert(TestObject::instance_count == 0);
    std::cout << "OK\n";
}

void test_unq_subtype() {
    std::cout << "\n=== UnqPtr: subtyping ===\n";
    {
        UnqPtr<Dog> dog(new Dog());
        UnqPtr<Animal> animal = std::move(dog);
        assert(!dog);
        assert(animal);
        assert(animal->speak() == "Woof!");
        
        UnqPtr<Cat> cat(new Cat());
        UnqPtr<Animal> animal2;
        animal2 = std::move(cat);
        assert(!cat);
        assert(animal2);
        assert(animal2->speak() == "Meow!");
    }
    std::cout << "OK\n";
}

void test_unq_null() {
    std::cout << "\n=== UnqPtr: null handling ===\n";
    {
        UnqPtr<TestObject> p;
        assert(!p);
        assert(p == nullptr);
        assert(nullptr == p);
        assert(p.get() == nullptr);
        
        UnqPtr<TestObject> p2(new TestObject(42));
        assert(p2 != nullptr);
        assert(nullptr != p2);
        
        UnqPtr<TestObject> p3 = std::move(p);
        assert(!p3);
        
        p2 = nullptr;
        assert(!p2);
    }
    std::cout << "OK\n";
}

void test_unq_custom_deleter() {
    std::cout << "\n=== UnqPtr: custom deleter ===\n";
    {
        static bool deleter_called = false;
        deleter_called = false;
        
        struct CustomDeleter {
            void operator()(TestObject* ptr) const {
                deleter_called = true;
                delete ptr;
            }
        };
        
        {
            TestObject* raw = new TestObject(999);
            UnqPtr<TestObject, CustomDeleter> p(raw, CustomDeleter{});
            assert(p->value == 999);
        }
        assert(deleter_called);
    }
    assert(TestObject::instance_count == 0);
    std::cout << "OK\n";
}

void test_unq_get_deleter() {
    std::cout << "\n=== UnqPtr: get_deleter ===\n";
    {
        struct CountingDeleter {
            int count = 0;
            void operator()(TestObject* ptr) const {
                delete ptr;
            }
        };
        
        UnqPtr<TestObject, CountingDeleter> p(new TestObject(42));
        auto& del = p.get_deleter();
        del.count = 100;
        assert(p.get_deleter().count == 100);
        
        const UnqPtr<TestObject, CountingDeleter>& cp = p;
        const auto& cdel = cp.get_deleter();
        assert(cdel.count == 100);
    }
    assert(TestObject::instance_count == 0);
    std::cout << "OK\n";
}

void test_unq_const_correctness() {
    std::cout << "\n=== UnqPtr: const correctness ===\n";
    {
        UnqPtr<TestObject> p(new TestObject(42));
        
        p->value = 100;
        assert(p->value == 100);
        
        const UnqPtr<TestObject>& cp = p;
        assert(cp->value == 100);
        assert(cp);
        assert(cp.get() != nullptr);
    }
    assert(TestObject::instance_count == 0);
    std::cout << "OK\n";
}

void test_unq_move_only_type() {
    std::cout << "\n=== UnqPtr: move-only type ===\n";
    {
        UnqPtr<MoveOnly> p1(new MoveOnly(42));
        assert(p1->value == 42);
        
        UnqPtr<MoveOnly> p2 = std::move(p1);
        assert(!p1);
        assert(p2->value == 42);
        
        auto p3 = make_unq<MoveOnly>(100);
        assert(p3->value == 100);
    }
    std::cout << "OK\n";
}

void test_unq_explicit_bool() {
    std::cout << "\n=== UnqPtr: explicit operator bool ===\n";
    {
        UnqPtr<TestObject> p1(new TestObject(42));
        UnqPtr<TestObject> p2;
        
        if (p1) {}
        if (!p2) {}
        
        
        assert(static_cast<bool>(p1) == true);
        assert(static_cast<bool>(p2) == false);
    }
    assert(TestObject::instance_count == 0);
    std::cout << "OK\n";
}


void test_shrd_basic() {
    std::cout << "\n=== ShrdPtr: basic ===\n";
    {
        ShrdPtr<TestObject> p1(new TestObject(50));
        assert(p1.use_count() == 1);
        assert(p1->value == 50);
        
        {
            ShrdPtr<TestObject> p2 = p1;
            assert(p1.use_count() == 2);
            assert(p2.use_count() == 2);
            assert(p2->value == 50);
            
            ShrdPtr<TestObject> p3 = p2;
            assert(p1.use_count() == 3);
        }
        
        assert(p1.use_count() == 1);
    }
    assert(TestObject::instance_count == 0);
    std::cout << "OK\n";
}

void test_shrd_move() {
    std::cout << "\n=== ShrdPtr: move ===\n";
    {
        ShrdPtr<TestObject> p1(new TestObject(77));
        ShrdPtr<TestObject> p2 = std::move(p1);
        assert(!p1);
        assert(p2);
        assert(p2.use_count() == 1);
        assert(p2->value == 77);
    }
    assert(TestObject::instance_count == 0);
    std::cout << "OK\n";
}

void test_shrd_assignment() {
    std::cout << "\n=== ShrdPtr: assignment ===\n";
    {
        ShrdPtr<TestObject> p1(new TestObject(1));
        ShrdPtr<TestObject> p2(new TestObject(2));
        assert(TestObject::instance_count == 2);
        
        p1 = p2;
        assert(TestObject::instance_count == 1);
        assert(p1->value == 2);
        assert(p1.use_count() == 2);
        
        p1 = nullptr;
        assert(p1.use_count() == 0);
        assert(p2.use_count() == 1);
    }
    assert(TestObject::instance_count == 0);
    std::cout << "OK\n";
}

void test_shrd_self_assignment() {
    std::cout << "\n=== ShrdPtr: self-assignment ===\n";
    {
        ShrdPtr<TestObject> p1(new TestObject(42));
        p1 = p1;
        assert(p1.use_count() == 1);
        assert(p1->value == 42);
    }
    assert(TestObject::instance_count == 0);
    std::cout << "OK\n";
}

void test_shrd_swap() {
    std::cout << "\n=== ShrdPtr: swap ===\n";
    {
        ShrdPtr<TestObject> p1(new TestObject(1));
        ShrdPtr<TestObject> p2(new TestObject(2));
        
        p1.swap(p2);
        assert(p1->value == 2);
        assert(p2->value == 1);
        assert(p1.use_count() == 1);
        assert(p2.use_count() == 1);
        
        ShrdPtr<TestObject> p3;
        p1.swap(p3);
        assert(!p1);
        assert(p3->value == 2);
        assert(p3.use_count() == 1);
    }
    assert(TestObject::instance_count == 0);
    std::cout << "OK\n";
}

void test_shrd_complex() {
    std::cout << "\n=== ShrdPtr: complex scenarios ===\n";
    
    {
        ShrdPtr<TestObject> p1(new TestObject(1));
        {
            ShrdPtr<TestObject> p2 = p1;
            ShrdPtr<TestObject> p3 = p2;
            ShrdPtr<TestObject> p4 = p3;
            assert(p1.use_count() == 4);
        }
        assert(p1.use_count() == 1);
    }
    assert(TestObject::instance_count == 0);
    
    {
        ShrdPtr<TestObject> a(new TestObject(10));
        ShrdPtr<TestObject> b(new TestObject(20));
        ShrdPtr<TestObject> c(new TestObject(30));
        
        a = b;
        assert(TestObject::instance_count == 2);
        assert(a.use_count() == 2);
        assert(a->value == 20);
        
        b = c;
        assert(TestObject::instance_count == 2);
        assert(a.use_count() == 1);
        assert(a->value == 20);
        assert(b.use_count() == 2);
        assert(b->value == 30);
        
        a = nullptr;
        assert(TestObject::instance_count == 1);
    }
    assert(TestObject::instance_count == 0);
    
    std::cout << "OK\n";
}
   

void test_shrd_subtype() {
    std::cout << "\n=== ShrdPtr: subtyping ===\n";
    {
        ShrdPtr<Dog> dog(new Dog());
        ShrdPtr<Animal> animal = dog;  
        assert(dog.use_count() == 2);
        assert(animal.use_count() == 2);
        assert(animal->speak() == "Woof!");
        
        ShrdPtr<Animal> animal2;
        animal2 = dog; 
        assert(dog.use_count() == 3);
    }
    std::cout << "OK\n";
}

void test_shrd_array() {
    std::cout << "\n=== ShrdPtr: raw array ===\n";
    {
        ShrdPtr<TestObject[]> arr(new TestObject[4]);
        for (int i = 0; i < 4; ++i) {
            arr[i].value = i + 1;
        }
        assert(arr[0].value == 1);
        assert(arr[3].value == 4);
        
        {
            ShrdPtr<TestObject[]> arr2 = arr;
            assert(arr.use_count() == 2);
        }
        
        assert(arr.use_count() == 1);
    }
    assert(TestObject::instance_count == 0);
    std::cout << "OK\n";
}

void test_shrd_make() {
    std::cout << "\n=== ShrdPtr: make_shrd ===\n";
    {
        auto p = make_shrd<TestObject>(555);
        assert(p->value == 555);
        
        auto arr = make_shrd_arr<TestObject>(3);
        arr[0].value = 10;
        arr[2].value = 30;
        assert(arr[0].value == 10);
        assert(arr[2].value == 30);
    }
    assert(TestObject::instance_count == 0);
    std::cout << "OK\n";
}

void test_shrd_null() {
    std::cout << "\n=== ShrdPtr: null handling ===\n";
    {
        ShrdPtr<TestObject> p;
        assert(!p);
        assert(p == nullptr);
        assert(nullptr == p);
        assert(p.get() == nullptr);
        assert(p.use_count() == 0);
        
        ShrdPtr<TestObject> p2(new TestObject(42));
        assert(p2 != nullptr);
        assert(nullptr != p2);
        
        ShrdPtr<TestObject> p3 = std::move(p);
        assert(!p3);
        
        p2 = nullptr;
        assert(!p2);
    }
    std::cout << "OK\n";
}

void test_shrd_custom_deleter() {
    std::cout << "\n=== ShrdPtr: custom deleter ===\n";
    {
        static bool deleter_called = false;
        deleter_called = false;
        
        {
            TestObject* raw = new TestObject(999);
            ShrdPtr<TestObject> p(raw, [](TestObject* ptr) {
                deleter_called = true;
                delete ptr;
            });
            assert(p->value == 999);
        }
        assert(deleter_called);
    }
    assert(TestObject::instance_count == 0);
    std::cout << "OK\n";
}
// что такое в сути своей std::function
void test_shrd_const_correctness() {
    std::cout << "\n=== ShrdPtr: const correctness ===\n";
    {
        ShrdPtr<TestObject> p(new TestObject(42));
        
        p->value = 100;
        assert(p->value == 100);
        
        const ShrdPtr<TestObject>& cp = p;
        assert(cp->value == 100);
        assert(cp.use_count() == 1);
        assert(cp);
    }
    assert(TestObject::instance_count == 0);
    std::cout << "OK\n";
}

void test_shrd_explicit_bool() {
    std::cout << "\n=== ShrdPtr: explicit operator bool ===\n";
    {
        ShrdPtr<TestObject> p1(new TestObject(42));
        ShrdPtr<TestObject> p2;
        
        if (p1) { /* OK */ }
        if (!p2) { /* OK */ }
        
        assert(static_cast<bool>(p1) == true);
        assert(static_cast<bool>(p2) == false);
    }
    assert(TestObject::instance_count == 0);
    std::cout << "OK\n";
}

void test_circular_reference_demo() {
    std::cout << "\n=== Circular reference demo (expected leak) ===\n";
    
    struct Node {
        ShrdPtr<Node> next;
        int value;
        Node(int v) : value(v) {}
        ~Node() { std::cout << "  Node " << value << " destroyed\n"; }
    };
    
    {
        auto a = make_shrd<Node>(1);
        auto b = make_shrd<Node>(2);
        a->next = b;
        b->next = a;
        
        std::cout << "  a.use_count() = " << a.use_count() << "\n";
        std::cout << "  b.use_count() = " << b.use_count() << "\n";
    }
    
    std::cout << "  (Nodes NOT destroyed — это проблема shared_ptr)\n";
    std::cout << "  Решение: WeakPtr\n";
}


int main() {
    std::cout << "========================================\n";
    std::cout << "  Running functional tests...\n";
    std::cout << "========================================\n";
    
    test_unq_basic();
    test_unq_move();
    test_unq_self_move();
    test_unq_release_reset();
    test_unq_edge_cases();
    test_unq_swap();
    test_unq_array();
    test_unq_array_reset();
    test_unq_make();
    test_unq_subtype();
    test_unq_null();
    test_unq_custom_deleter();
    test_unq_get_deleter();
    test_unq_const_correctness();
    test_unq_move_only_type();
    test_unq_explicit_bool();
    
    test_shrd_basic();
    test_shrd_move();
    test_shrd_assignment();
    test_shrd_self_assignment();
    test_shrd_swap();
    test_shrd_complex();
    test_shrd_subtype();
    test_shrd_array();
    test_shrd_make();
    test_shrd_null();
    test_shrd_custom_deleter();
    test_shrd_const_correctness();
    test_shrd_explicit_bool();
    
    test_circular_reference_demo();
    
    std::cout << "\n========================================\n";
    std::cout << "  ALL FUNCTIONAL TESTS PASSED\n";
    std::cout << "========================================\n";
    
    return 0;
}

// g++ -std=c++20 -O3 -Wall -Wextra bench.cpp -o benchmark
// g++ -std=c++20 -Wall -Wextra -O2 tests.cpp -o test_pointers

// cd /mnt/c/Users/user/Desktop/dz_1/
// g++ -std=c++20 -O0 -g tests.cpp -o tests_valgrind
// valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./tests_valgrind