#include "doctest.h"
#include "logic/ui_scale.h"

using namespace csopesy;

TEST_CASE("ui scale preserves the original 1280 by 720 baseline") {
    CHECK(calculateUiScale(1280, 720) == doctest::Approx(1.0f));
}

TEST_CASE("ui scale caps at the balanced 150 percent density") {
    CHECK(calculateUiScale(1920, 1080) == doctest::Approx(1.5f));
    CHECK(calculateUiScale(2560, 1440) == doctest::Approx(1.5f));
}

TEST_CASE("ui scale never drops below the original density") {
    CHECK(calculateUiScale(800, 500) == doctest::Approx(1.0f));
}

TEST_CASE("ui scale uses the limiting window dimension") {
    CHECK(calculateUiScale(1920, 720) == doctest::Approx(1.0f));
    CHECK(calculateUiScale(1280, 1080) == doctest::Approx(1.0f));
}

TEST_CASE("taskbar icons retain a readable size at each density") {
    CHECK(calculateTaskbarIconSize(1.0f) == doctest::Approx(11.0f));
    CHECK(calculateTaskbarIconSize(1.5f) == doctest::Approx(16.5f));
}
