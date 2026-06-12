#include "doctest.h"
#include "logic/clock_format.h"
#include <ctime>

static std::tm makeTm(int year, int mon, int mday, int hour, int min, int wday) {
    std::tm t{};
    t.tm_year = year - 1900; t.tm_mon = mon; t.tm_mday = mday;
    t.tm_hour = hour; t.tm_min = min; t.tm_wday = wday;
    return t;
}

TEST_CASE("formatClock renders the reference style") {
    std::tm t = makeTm(2026, 3, 30, 17, 25, 4);
    CHECK(csopesy::formatClock(t) == "Thursday, Apr 30, 2026 | 05:25 PM");
}

TEST_CASE("formatClock pads and uses AM correctly") {
    std::tm t = makeTm(2026, 0, 5, 9, 7, 1);
    CHECK(csopesy::formatClock(t) == "Monday, Jan 05, 2026 | 09:07 AM");
}
