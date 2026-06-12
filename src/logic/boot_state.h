#pragma once
namespace csopesy {
enum class AppState { Bios, Splash, Desktop };
struct BootTimings { double biosDuration; double splashDuration; };
AppState advanceBootState(AppState current, double elapsedInState,
                          bool skipRequested, const BootTimings& t);
}
