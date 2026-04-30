#include <iostream>
#include <string>

void run_aos_vs_soa();

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: ./perf_playground <benchmark>\n";
        std::cout << "Available benchmarks:\n";
        std::cout << "  aos_vs_soa\n";
        std::cout << "  all\n";
        return 1;
    }

    std::string benchmark = argv[1];

    if (benchmark == "aos_vs_soa") {
        run_aos_vs_soa();
    } else if (benchmark == "all") {
        run_aos_vs_soa();
    } else {
        std::cout << "Unknown benchmark: " << benchmark << "\n";
        return 1;
    }

    return 0;
}