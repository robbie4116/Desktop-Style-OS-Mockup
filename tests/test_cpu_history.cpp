#include "doctest.h"
#include "logic/cpu_history.h"
using namespace csopesy;

TEST_CASE("history never exceeds capacity and keeps newest order") {
    CpuHistory h(3);
    h.push(1.0f); h.push(2.0f); h.push(3.0f); h.push(4.0f);
    CHECK(h.size() == 3);
    const float* d = h.data();
    CHECK(d[0] == doctest::Approx(2.0f));
    CHECK(d[2] == doctest::Approx(4.0f));
}
TEST_CASE("starts empty and fills") {
    CpuHistory h(4);
    CHECK(h.size() == 0);
    h.push(5.0f);
    CHECK(h.size() == 1);
    CHECK(h.data()[0] == doctest::Approx(5.0f));
}
