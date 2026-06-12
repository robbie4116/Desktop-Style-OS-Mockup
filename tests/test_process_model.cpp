#include "doctest.h"
#include "logic/process_model.h"
#include <string>
using namespace csopesy;

TEST_CASE("LCG jitter is deterministic for a given seed") {
    unsigned a = 12345, b = 12345;
    float va = nextUnit(a);
    float vb = nextUnit(b);
    CHECK(va == vb);
    CHECK(a == b);
}
TEST_CASE("updateProcess keeps cpu and memory within clamped ranges") {
    ProcessRow row{"test.exe", 1000, 20.0f, 100.0f, 99u};
    for (int i = 0; i < 5000; ++i) {
        updateProcess(row);
        CHECK(row.cpu >= 0.0f);
        CHECK(row.cpu <= 100.0f);
        CHECK(row.memMB >= 1.0f);
        CHECK(row.memMB <= 4096.0f);
    }
}
TEST_CASE("aggregateCpu averages the rows") {
    std::vector<ProcessRow> rows{
        {"a", 1, 10.0f, 0.0f, 1u}, {"b", 2, 30.0f, 0.0f, 2u}};
    CHECK(aggregateCpu(rows) == doctest::Approx(20.0f));
}
TEST_CASE("makeDefaultProcesses returns a non-empty believable list") {
    auto v = makeDefaultProcesses();
    CHECK(v.size() >= 8);
    CHECK(std::string(v.front().name).size() > 0);
}
TEST_CASE("aggregateMemMB sums the rows") {
    std::vector<ProcessRow> rows{{"a",1,0.0f,100.0f,1u},{"b",2,0.0f,200.0f,2u}};
    CHECK(aggregateMemMB(rows) == doctest::Approx(300.0f));
}
