# Existing implementations: source inspection

Scope: fetched source snapshots on 2026-10-30, not runtime validation on macOS 27.0.1. CONFIRMED below means confirmed in the named source, not confirmed working on the installed OS. Raw files, recursive trees and exact revisions are in `evidence/projects/`. Failed initial guesses for yabai `src/sa/*` returned 404; actual source resides in `src/osax/`. This is a bounded review, not an exhaustive audit.

## Revisions

| Project | Revision |
|---|---|
| AeroSpace | `8e7899629c396e2a735c7b5d9950565af239c706` |
| Hyprland | `4b9ac40d71ceeeb027cfa06992c9212e3c3077b6` |
| SketchyBar | `a18fc9488d14cba1f15f2c95e61822dc01b2d032` |
| darling | `3447ff5ab8e837ea54a13a506c60d34d77e2bd15` |
| smithay | `f581531d479cf2e3307bafb913c5d2bb12860bb3` |
| wlroots | `0855cdacb2eeeff35849e2e9c4db0aa996d78d10` |
| yabai | `dd845723416f5fe92af49fad5ebab00369e07edd` |

## yabai

CONFIRMED: ordinary geometry changes use AX position and size setters in `window_manager.c` (lines 421, 431). The same implementation creates SkyLight proxy windows, captures a target using `SLSHWCaptureWindowList` (line 521), and applies transaction transforms to proxies. This is decisive evidence that visible animation can be implemented using a captured proxy while native WindowServer remains the compositor. It is not evidence of ownership of live native render submissions.

The `osax/payload.m` component declares foreign-window operations, Space operations, and private transforms. The separate arm64 payload contains architecture-specific machinery. These operations must not be conflated with the capabilities of the ordinary AX process; deployment and authorization requirements require version-specific verification. No payload was loaded here. Reusable components: window bookkeeping, layout algorithms, private API declarations as ABI leads, and proxy animation design. Limitations: AX app behavior, capture artifacts, private version drift, and privilege boundary for enhanced actions.

[yabai: `src/window_manager.c`](https://github.com/koekeishiya/yabai/blob/dd845723416f5fe92af49fad5ebab00369e07edd/src/window_manager.c); [yabai: `src/osax/payload.m`](https://github.com/koekeishiya/yabai/blob/dd845723416f5fe92af49fad5ebab00369e07edd/src/osax/payload.m); [yabai: `src/osax/arm64_payload.m`](https://github.com/koekeishiya/yabai/blob/dd845723416f5fe92af49fad5ebab00369e07edd/src/osax/arm64_payload.m).

## AeroSpace

CONFIRMED: `MacWindow.setAxFrame` delegates to its application AX adapter; `hideInCorner` moves windows to monitor corners and preserves prior position. The code includes an application-specific Zoom offset workaround. Thus its workspace abstraction is not a replacement for the compositor or a new native Space protocol. This illustrates both reusable policy/state design and app-specific compatibility costs. Rendering stays with macOS. AX authorization is the relevant permission for its geometry route; this review did not inspect every entitlement or startup path.

[AeroSpace: `Sources/AppBundle/tree/MacWindow.swift`](https://github.com/nikitabobko/AeroSpace/blob/8e7899629c396e2a735c7b5d9950565af239c706/Sources/AppBundle/tree/MacWindow.swift); [AeroSpace: `Sources/AppBundle/tree/Workspace.swift`](https://github.com/nikitabobko/AeroSpace/blob/8e7899629c396e2a735c7b5d9950565af239c706/Sources/AppBundle/tree/Workspace.swift).

## SketchyBar

CONFIRMED: `window.c` creates its own SkyLight windows with a CGContext, sets tags/opacity/resolution, uses SkyLight transactions, moves windows between managed Spaces and orders its windows. This establishes substantial custom shell capability, not foreign application surface control. Its CGContext clear/flush path illustrates a shell render target under WindowServer ownership. Reusable: panel window lifecycle and Space association. Risks: private API ABI, tags and transaction behavior. Working on the current host has not been tested.

[SketchyBar: `src/window.c`](https://github.com/FelixKratz/SketchyBar/blob/a18fc9488d14cba1f15f2c95e61822dc01b2d032/src/window.c); [SketchyBar: `src/misc/extern.h`](https://github.com/FelixKratz/SketchyBar/blob/a18fc9488d14cba1f15f2c95e61822dc01b2d032/src/misc/extern.h).

## Smithay

CONFIRMED: Wayland compositor handlers retain pending/current buffer state, damage, scale, transform, and buffer release behavior; the DRM compositor and renderer are separate modules. These are the correct conceptual seams for a native design: acquire buffer, synchronize, composite, present, release. However a Wayland `wl_surface` commit is not a SkyLight/CA transaction. Smithay's Linux DRM backend does not provide Apple display ownership or native AppKit wire compatibility. Reuse architecture and potentially independent policy code; treat a macOS protocol and Metal backend as new work, not a backend flag.

[smithay: `src/wayland/compositor/handlers.rs`](https://github.com/Smithay/smithay/blob/f581531d479cf2e3307bafb913c5d2bb12860bb3/src/wayland/compositor/handlers.rs); [smithay: `src/backend/drm/compositor/mod.rs`](https://github.com/Smithay/smithay/blob/f581531d479cf2e3307bafb913c5d2bb12860bb3/src/backend/drm/compositor/mod.rs); [smithay: `src/backend/renderer/mod.rs`](https://github.com/Smithay/smithay/blob/f581531d479cf2e3307bafb913c5d2bb12860bb3/src/backend/renderer/mod.rs).

## wlroots

CONFIRMED: the inspected GitHub snapshot has explicit DRM atomic/legacy selection and CRTC commit/page-flip control. These expose a presentation boundary not demonstrated for an ordinary macOS process. `wlr_compositor.c` supplies Wayland compositor infrastructure. This snapshot is historical; the GitHub repository is not used here as evidence of current upstream development. Useful lessons: output test/commit, buffer lifetime and damage accounting. Linux DRM/KMS implementation cannot be transferred to Apple Silicon macOS by renaming APIs.

[wlroots: `backend/drm/drm.c`](https://github.com/swaywm/wlroots/blob/0855cdacb2eeeff35849e2e9c4db0aa996d78d10/backend/drm/drm.c); [wlroots: `types/wlr_compositor.c`](https://github.com/swaywm/wlroots/blob/0855cdacb2eeeff35849e2e9c4db0aa996d78d10/types/wlr_compositor.c).

## Hyprland

CONFIRMED: this revision's renderer calls Aquamarine output presentation modes, a monitor commit coordinator, and explicit-sync submission. It must not be described as merely a current wlroots frontend. `renderMonitor` coordinates damage, frame scheduling and final output commits. Its effects are possible because the system controls a scene and output contract. Those prerequisites remain missing for arbitrary native macOS applications. Reusable: damage/effect/presentation design ideas; not native macOS protocol compatibility.

[Hyprland: `src/render/Renderer.cpp`](https://github.com/hyprwm/Hyprland/blob/4b9ac40d71ceeeb027cfa06992c9212e3c3077b6/src/render/Renderer.cpp); [Hyprland: `src/output/Monitor.cpp`](https://github.com/hyprwm/Hyprland/blob/4b9ac40d71ceeeb027cfa06992c9212e3c3077b6/src/output/Monitor.cpp).

## Darling and the danger of symbol inventories

CONFIRMED: the inspected Darling SkyLight source has `SLSMainConnectionID` and `SLSNewConnection` stubs returning NULL. Their existence in a compatibility tree does not demonstrate a working Apple WindowServer-compatible endpoint. The generated untyped signatures are not authoritative ABI declarations. This source is a useful symbol checklist only for these functions. It does not establish that all Darling graphics work is stubbed; this finding is scoped to the inspected file.

[darling: `src/private-frameworks/SkyLight/src/SkyLight.m`](https://github.com/darlinghq/darling/blob/3447ff5ab8e837ea54a13a506c60d34d77e2bd15/src/private-frameworks/SkyLight/src/SkyLight.m).

## Why macOS projects retain WindowServer

STRONGLY SUPPORTED: the source paths collectively show a combination of API, ownership and compatibility constraints: AX/private calls manipulate server-owned windows; custom shell windows are server clients; captured proxies remain server windows. Linux compositor sources instead receive client commits and own output commits. No inspected macOS project supplies an equivalent end-to-end native render and display contract. This is a bounded negative finding, not proof that reverse engineering cannot supply one. Security alone is not the explanation: removing a permission check would still leave CA state, fences, lifetime, input and display semantics to implement.

Further investigation should inspect current Core Animation transport and live Metal drawable publication before expanding the window-policy implementation. See reports 03 and 04 for the protocol and surface evidence.
