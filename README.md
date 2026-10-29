# Auto Deco — Geode mod source project

This is a build-ready **source project**, not an already compiled `.geode` mod.
The `.geode` package needs a compiled binary for the target platform (Windows or Android, etc.),
which must be built with the matching Geode SDK and Geometry Dash version.

## Build
1. Install the Geode SDK/toolchain for your platform and the matching Geometry Dash version.
2. Open this folder in the Geode CLI/CMake workflow.
3. Configure and build with the Geode SDK environment.
4. The Geode build step (`setup_geode_mod`) should produce the installable `.geode` artifact.
5. Install that artifact using Geode's mod installer for your platform.

## Configure the mod
In Geometry Dash's Geode mod settings, enter comma-separated object IDs for:
- `edge-ids`: decorations placed along exposed solid-block edges
- `fill-ids`: random decorations scattered on solid blocks
- `hazard-ids`: decorations placed beside hazards

Adjust the three density settings to control placement chance.

## Notes
- The uploaded manifest targets Geode 4.0.0 and Geometry Dash 2.2081.
- Build separately for each supported platform; a Windows binary is not an Android binary.
- The developer field and mod ID are still the values from the supplied manifest (`YourName` and `yourname.autodeco`).
