#include "doctest.h"
#include "logic/boot_state.h"
using namespace csopesy;

static BootTimings T{3.0, 2.5};

TEST_CASE("bios advances to splash on timeout") {
    CHECK(advanceBootState(AppState::Bios, 2.9, false, T) == AppState::Bios);
    CHECK(advanceBootState(AppState::Bios, 3.0, false, T) == AppState::Splash);
}
TEST_CASE("bios advances to splash on skip") {
    CHECK(advanceBootState(AppState::Bios, 0.1, true, T) == AppState::Splash);
}
TEST_CASE("splash advances to desktop on timeout or skip") {
    CHECK(advanceBootState(AppState::Splash, 2.4, false, T) == AppState::Splash);
    CHECK(advanceBootState(AppState::Splash, 2.5, false, T) == AppState::Desktop);
    CHECK(advanceBootState(AppState::Splash, 0.0, true, T) == AppState::Desktop);
}
TEST_CASE("desktop is terminal") {
    CHECK(advanceBootState(AppState::Desktop, 999.0, true, T) == AppState::Desktop);
}
