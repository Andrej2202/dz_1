#include "unique_ptr.hpp"
#include "shared_ptr.hpp"
#include <iostream>
#include <iomanip>
#include <chrono>
#include <vector>
#include <memory>
#include <string>
#include <fstream>

using namespace smart_pointers;
using namespace std::chrono;

struct alignas(16) SmallObject {
    int value;
    SmallObject(int v = 0) : value(v) {}
};

template<typename Func>
double measure_ms(Func&& f) {
    auto start = high_resolution_clock::now();
    f();
    auto end = high_resolution_clock::now();
    return duration<double, std::milli>(end - start).count();
}

std::string fmt(std::size_t n) {
    if (n >= 1000000000) return "10^9";
    if (n >= 1000000)    return "10^6";
    if (n >= 1000)       return "10^3";
    return std::to_string(n);
}

double bench_raw_create_destroy(std::size_t N) {
    std::vector<SmallObject*> ptrs;
    ptrs.reserve(N);
    return measure_ms([&]() {
        for (std::size_t i = 0; i < N; ++i)
            ptrs.push_back(new SmallObject(static_cast<int>(i)));
        for (auto p : ptrs)
            delete p;
        ptrs.clear();
    });
}

double bench_my_unq_create_destroy(std::size_t N) {
    std::vector<UnqPtr<SmallObject>> ptrs;
    ptrs.reserve(N);
    return measure_ms([&]() {
        for (std::size_t i = 0; i < N; ++i)
            ptrs.emplace_back(new SmallObject(static_cast<int>(i)));
        ptrs.clear();
    });
}

double bench_std_unq_create_destroy(std::size_t N) {
    std::vector<std::unique_ptr<SmallObject>> ptrs;
    ptrs.reserve(N);
    return measure_ms([&]() {
        for (std::size_t i = 0; i < N; ++i)
            ptrs.push_back(std::make_unique<SmallObject>(static_cast<int>(i)));
        ptrs.clear();
    });
}

double bench_my_shrd_create_destroy(std::size_t N) {
    std::vector<ShrdPtr<SmallObject>> ptrs;
    ptrs.reserve(N);
    return measure_ms([&]() {
        for (std::size_t i = 0; i < N; ++i)
            ptrs.emplace_back(new SmallObject(static_cast<int>(i)));
        ptrs.clear();
    });
}


double bench_std_shrd_create_destroy(std::size_t N) {
    std::vector<std::shared_ptr<SmallObject>> ptrs;
    ptrs.reserve(N);
    return measure_ms([&]() {
        for (std::size_t i = 0; i < N; ++i)
            ptrs.push_back(std::make_shared<SmallObject>(static_cast<int>(i)));
        ptrs.clear();
    });
}

double bench_my_shrd_copy(std::size_t N) {
    ShrdPtr<SmallObject> original(new SmallObject(42));
    std::vector<ShrdPtr<SmallObject>> copies;
    copies.reserve(N);
    return measure_ms([&]() {
        for (std::size_t i = 0; i < N; ++i)
            copies.push_back(original);
        copies.clear();
    });
}

double bench_std_shrd_copy(std::size_t N) {
    std::shared_ptr<SmallObject> original = std::make_shared<SmallObject>(42);
    std::vector<std::shared_ptr<SmallObject>> copies;
    copies.reserve(N);
    return measure_ms([&]() {
        for (std::size_t i = 0; i < N; ++i)
            copies.push_back(original);
        copies.clear();
    });
}

double bench_my_unq_move(std::size_t N) {
    std::vector<UnqPtr<SmallObject>> src;
    src.reserve(N);
    for (std::size_t i = 0; i < N; ++i)
        src.emplace_back(new SmallObject(static_cast<int>(i)));

    std::vector<UnqPtr<SmallObject>> dst;
    dst.reserve(N);
    return measure_ms([&]() {
        for (std::size_t i = 0; i < N; ++i)
            dst.push_back(std::move(src[i]));
        dst.clear();
    });
}

double bench_std_unq_move(std::size_t N) {
    std::vector<std::unique_ptr<SmallObject>> src;
    src.reserve(N);
    for (std::size_t i = 0; i < N; ++i)
        src.push_back(std::make_unique<SmallObject>(static_cast<int>(i)));

    std::vector<std::unique_ptr<SmallObject>> dst;
    dst.reserve(N);
    return measure_ms([&]() {
        for (std::size_t i = 0; i < N; ++i)
            dst.push_back(std::move(src[i]));
        dst.clear();
    });
}

double bench_raw_access(std::size_t N) {
    std::vector<SmallObject*> ptrs;
    ptrs.reserve(N);
    for (std::size_t i = 0; i < N; ++i)
        ptrs.push_back(new SmallObject(static_cast<int>(i)));

    volatile int sum = 0;
    double t = measure_ms([&]() {
        for (std::size_t i = 0; i < N; ++i)
            sum += ptrs[i]->value;
    });
    for (auto p : ptrs) delete p;
    return t;
}

double bench_my_unq_access(std::size_t N) {
    std::vector<UnqPtr<SmallObject>> ptrs;
    ptrs.reserve(N);
    for (std::size_t i = 0; i < N; ++i)
        ptrs.emplace_back(new SmallObject(static_cast<int>(i)));

    volatile int sum = 0;
    double t = measure_ms([&]() {
        for (std::size_t i = 0; i < N; ++i)
            sum += ptrs[i]->value;
    });
    return t;
}

double bench_std_unq_access(std::size_t N) {
    std::vector<std::unique_ptr<SmallObject>> ptrs;
    ptrs.reserve(N);
    for (std::size_t i = 0; i < N; ++i)
        ptrs.push_back(std::make_unique<SmallObject>(static_cast<int>(i)));

    volatile int sum = 0;
    double t = measure_ms([&]() {
        for (std::size_t i = 0; i < N; ++i)
            sum += ptrs[i]->value;
    });
    return t;
}

double bench_my_shrd_access(std::size_t N) {
    std::vector<ShrdPtr<SmallObject>> ptrs;
    ptrs.reserve(N);
    for (std::size_t i = 0; i < N; ++i)
        ptrs.emplace_back(new SmallObject(static_cast<int>(i)));

    volatile int sum = 0;
    double t = measure_ms([&]() {
        for (std::size_t i = 0; i < N; ++i)
            sum += ptrs[i]->value;
    });
    return t;
}

double bench_std_shrd_access(std::size_t N) {
    std::vector<std::shared_ptr<SmallObject>> ptrs;
    ptrs.reserve(N);
    for (std::size_t i = 0; i < N; ++i)
        ptrs.push_back(std::make_shared<SmallObject>(static_cast<int>(i)));

    volatile int sum = 0;
    double t = measure_ms([&]() {
        for (std::size_t i = 0; i < N; ++i)
            sum += ptrs[i]->value;
    });
    return t;
}

template<typename CreateFunc>
double bench_iterative(std::size_t total, std::size_t batch, CreateFunc create) {
    std::size_t batches = total / batch;
    return measure_ms([&]() {
        for (std::size_t b = 0; b < batches; ++b) {
            auto local = create(batch);
        }
    });
}

void print_header() {
    std::cout << "\n"
        << std::setw(22) << std::left << "Type"
        << std::setw(8)  << "N"
        << std::setw(18) << "Create+Del (ms)"
        << std::setw(16) << "Copy (ms)"
        << std::setw(16) << "Move (ms)"
        << std::setw(16) << "Access (ms)"
        << "\n" << std::string(96, '-') << "\n";
}

void print_row(const std::string& name, std::size_t N,
               double cd, double cp, double mv, double ac) {
    auto f = [](double v) -> std::string {
        if (v < 0) return "N/A";
        char buf[32];
        snprintf(buf, sizeof(buf), "%.2f", v);
        return buf;
    };
    std::cout << std::setw(22) << std::left << name
              << std::setw(8)  << fmt(N)
              << std::setw(18) << f(cd)
              << std::setw(16) << f(cp)
              << std::setw(16) << f(mv)
              << std::setw(16) << f(ac)
              << "\n";
}

void run_full(std::size_t N) {
    std::cout << "\n>>> N = " << fmt(N) << " (" << N << ")\n";

    print_header();

    print_row("raw new/delete", N,
        bench_raw_create_destroy(N), -1, -1, bench_raw_access(N));

    print_row("my UnqPtr", N,
        bench_my_unq_create_destroy(N), -1,
        bench_my_unq_move(N), bench_my_unq_access(N));

    print_row("std::unique_ptr", N,
        bench_std_unq_create_destroy(N), -1,
        bench_std_unq_move(N), bench_std_unq_access(N));

    print_row("my ShrdPtr", N,
        bench_my_shrd_create_destroy(N), bench_my_shrd_copy(N),
        -1, bench_my_shrd_access(N));

    print_row("std::shared_ptr", N,
        bench_std_shrd_create_destroy(N), bench_std_shrd_copy(N),
        -1, bench_std_shrd_access(N));
}

void run_iterative_1e9() {
    const std::size_t TOTAL = 1000000000ULL;
    const std::size_t BATCH = 1000000;

    std::cout << "\n>>> N = 10^9 (итеративный, батчи по 10^6)\n";
    std::cout << "    Это займёт несколько минут. Подождите...\n\n";

    std::cout << std::setw(22) << std::left << "Type"
              << std::setw(18) << "Total (ms)"
              << std::setw(18) << "Per 10^6 (ms)"
              << std::setw(18) << "Per op (ns)"
              << "\n" << std::string(76, '-') << "\n";

    auto print = [&](const std::string& name, double ms) {
        double per_batch = ms / (TOTAL / BATCH);
        double per_op_ns = (ms * 1e6) / TOTAL;
        char buf1[32], buf2[32], buf3[32];
        snprintf(buf1, sizeof(buf1), "%.1f", ms);
        snprintf(buf2, sizeof(buf2), "%.2f", per_batch);
        snprintf(buf3, sizeof(buf3), "%.1f", per_op_ns);
        std::cout << std::setw(22) << std::left << name
                  << std::setw(18) << buf1
                  << std::setw(18) << buf2
                  << std::setw(18) << buf3
                  << "\n";
    };

    std::cout << "  [1/5] raw new/delete...\n";
    print("raw new/delete", bench_iterative(TOTAL, BATCH,
        [](std::size_t n) {
            std::vector<SmallObject*> v;
            v.reserve(n);
            for (std::size_t i = 0; i < n; ++i) v.push_back(new SmallObject(0));
            for (auto p : v) delete p;
            return 0;
        }));

    std::cout << "  [2/5] my UnqPtr...\n";
    print("my UnqPtr", bench_iterative(TOTAL, BATCH,
        [](std::size_t n) {
            std::vector<UnqPtr<SmallObject>> v;
            v.reserve(n);
            for (std::size_t i = 0; i < n; ++i)
                v.emplace_back(new SmallObject(0));
            return 0;
        }));

    std::cout << "  [3/5] std::unique_ptr...\n";
    print("std::unique_ptr", bench_iterative(TOTAL, BATCH,
        [](std::size_t n) {
            std::vector<std::unique_ptr<SmallObject>> v;
            v.reserve(n);
            for (std::size_t i = 0; i < n; ++i)
                v.push_back(std::make_unique<SmallObject>(0));
            return 0;
        }));

    std::cout << "  [4/5] my ShrdPtr...\n";
    print("my ShrdPtr", bench_iterative(TOTAL, BATCH,
        [](std::size_t n) {
            std::vector<ShrdPtr<SmallObject>> v;
            v.reserve(n);
            for (std::size_t i = 0; i < n; ++i)
                v.emplace_back(new SmallObject(0));
            return 0;
        }));

    std::cout << "  [5/5] std::shared_ptr...\n";
    print("std::shared_ptr", bench_iterative(TOTAL, BATCH,
        [](std::size_t n) {
            std::vector<std::shared_ptr<SmallObject>> v;
            v.reserve(n);
            for (std::size_t i = 0; i < n; ++i)
                v.push_back(std::make_shared<SmallObject>(0));
            return 0;
        }));
}

int main() {
    std::cout << "========================================\n";
    std::cout << "  SMART POINTERS BENCHMARK\n";
    std::cout << "========================================\n";

    run_full(1000);

    run_full(1000000);

    std::cout << "\n>>> Запустить тест на 10^9? (y/n): ";
    char c;
    std::cin >> c;
    if (c == 'y' || c == 'Y') {
        run_iterative_1e9();
    } else {
        std::cout << "Пропущено.\n";
    }

    std::cout << "\n========================================\n";
    std::cout << "  BENCHMARK COMPLETE\n";
    std::cout << "========================================\n";

    return 0;
}