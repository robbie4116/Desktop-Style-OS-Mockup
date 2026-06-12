# Adaptive UI Scaling Design

## Goal

Make every visible OS surface readable in the monitor-adaptive application window:
BIOS, splash/loading, desktop clock, taskbar, File Explorer, Settings, and Task Manager.

## Design

Compute one UI scale from the initial client dimensions relative to the original
1280x720 design baseline:

`scale = clamp(min(width / 1280, height / 720), 1.0, 1.5)`

At 1920x1080 this produces the approved balanced 150% density. Smaller windows
retain the original minimum density, and larger windows stop at 150% to avoid
oversized controls.

Store the scale in `AppContext`. At startup, apply it to ImGui's global font
scale and style metrics. Explicit sizes and custom `ImDrawList` geometry must
also multiply by the shared scale, because ImGui does not automatically scale
those values.

## Scope

- Global font, padding, spacing, title bars, controls, tabs, tables, and rows
- BIOS text and padding
- Splash text, loading indicator, and progress bar
- Desktop clock text box
- Taskbar height, buttons, icons, spacing, and status label
- Default File Explorer, Settings, and Task Manager window sizes
- Task Manager graph/table heights and File Explorer tree width

The wallpaper remains full-window and is not otherwise changed.

## Testing

Add pure unit tests for the scale calculation at the 1280x720 baseline, the
1920x1080 cap, small-window minimum, and non-16:9 limiting dimension. Build and
run the existing test suite, then launch the app and capture/inspect BIOS,
splash, desktop, and opened utility windows.
