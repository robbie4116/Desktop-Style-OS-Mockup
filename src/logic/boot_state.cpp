#include "logic/boot_state.h"
namespace csopesy {
AppState advanceBootState(AppState current, double elapsedInState,
                          bool skipRequested, const BootTimings& t) {
    switch (current) {
        case AppState::Bios:
            return (skipRequested || elapsedInState >= t.biosDuration)
                       ? AppState::Splash : AppState::Bios;
        case AppState::Splash:
            return (skipRequested || elapsedInState >= t.splashDuration)
                       ? AppState::Desktop : AppState::Splash;
        case AppState::Desktop:
        default:
            return AppState::Desktop;
    }
}
}
