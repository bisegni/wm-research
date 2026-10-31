# WindowServer replacement feasibility

Target: macOS 27.0.1 (26A434), arm64. **Decision: replacement is not yet justified by recovered compatibility evidence; it is not proven impossible.**

## Launch, identity and protection

**CONFIRMED:** The running job is a system LaunchDaemon with `-daemon`; the executable links SkyLight and libSystem. Launch services include windowserver, CARenderServer, CoreDisplay and HID names; see [IPC report](03-windowserver-ipc.md) and [local inspection](evidence/windowserver-local.txt). This demonstrates a broader service surface than the four-method illustrative window trait.

**CONFIRMED:** SIP and authenticated-root protection both report enabled. Apple documents that SIP protects system paths and that the signed system volume verifies system content. Replacing the protected executable in place is **BLOCKED under the current protection configuration**, based on those policies; no write was attempted. This is separate from writing an independently named experimental server. [Apple SIP filesystem protection](https://developer.apple.com/library/archive/documentation/Security/Conceptual/System_Integrity_Protection_Guide/FileSystemProtections/FileSystemProtections.html), [Apple signed system volume](https://support.apple.com/en-ca/guide/security/secd698747c9/web).

**CONFIRMED:** Its signature carries private entitlements, retained in [entitlement plist](evidence/windowserver-entitlements.plist), including:

- HID server/privileged access and IOHID event-system server.
- `com.apple.private.iokit.displayservice`, `com.apple.private.iomfb.set-block`, and set-block client profile `windowserver`.
- `com.apple.private.gpuwrangler`, `com.apple.private.iosurfaceinfo` and master-sync-generator access.
- QuartzCore secure mode, TCC authorization and audit-token checks.
- Exclaves conclave host and per-user launchd lookup.

These are observed granted privileges, not a proven minimal entitlement set. Copying keys into an ad-hoc signature does not establish that kernel/driver policy will grant the same authority. The precise enforcement point for each key remains UNKNOWN. Disabling SIP alone does not implement protocols, produce Apple signing identity, implement display drivers, or resolve GPU synchronization. Which individual protections could be relaxed on a dedicated research installation requires separate authorization and experiments.

## Five different replacement targets

| Target | Current evidence and boundary |
|---|---|
| Replace executable | Protected path and signed volume are verified; not attempted |
| Reimplement protocol | Possible research target; no working compatibility handshake yet |
| Run independent compositor | Feasible for deliberately shared surfaces; native arbitrary-app acquisition unresolved |
| Own physical presentation | No demonstrated path replacing WindowServer scanout scheduling on Apple Silicon |
| Replace graphical session | Requires login, bootstrap, input, rendering, display, power/security integration; no demonstrated alternative |

## Feasibility matrix

| Capability | Public APIs | Private APIs | Server replacement | Known blocker or uncertainty |
|---|---|---|---|---|
| Own-window lifecycle/geometry | AppKit | SLS entry points | Must implement full semantics | Foreign-window authority differs |
| Foreign layout/focus | Accessibility with permission | Additional SLS/Dock techniques | Could own policy if compatible | Ownership checks and behavior build-specific |
| Workspaces/fullscreen | Limited application behavior | Managed-space interfaces in projects | Must coordinate all state | Protocol/session semantics unknown |
| Decorations | Own window drawing | Own overlays and transformations | Could implement server policy | Native app titlebars/content may be client-rendered |
| Input routing | Event taps/posting under consent | HID/private event paths | Must replace trusted routing | Current executable has privileged HID entitlements |
| GPU effects | Metal in own surfaces | Private transformations | Own renderer conceivable | Native surface acquisition/transaction ownership unresolved |
| Surface management | IOSurface explicit sharing | CA/render-server protocols | Full resource protocol required | No arbitrary-app direct-surface handoff demonstrated |
| Multi-monitor configuration | CoreGraphics configuration APIs | CoreDisplay | Display backend required | Configuration is not scanout ownership |
| Physical synchronized presentation | Present own drawables | Displayservice/IOMFB candidates | Driver/session integration required | Authority and interface unknown |
| Shell panels/widgets | AppKit/Metal | SLS optional | Independent of compatibility proof | Conventional shell surfaces achievable |

## Compatibility and session dependencies

| Client class | Requirement beyond geometry | Status for replacement |
|---|---|---|
| AppKit | connection, events, regions, render contexts, display metrics | UNKNOWN |
| SwiftUI | AppKit hosting plus framework rendering transactions | UNKNOWN |
| Electron | Cocoa windows plus Chromium GPU/surface paths | UNKNOWN |
| Qt | Cocoa platform plugin plus chosen rendering backend | UNKNOWN |
| Metal | drawable lifecycle, synchronization, presentation feedback | UNKNOWN |
| Fullscreen | display/workspace transitions and focus | UNKNOWN |
| Multi-display | scale/color/timing/migration/hotplug | UNKNOWN |

These are compatibility test categories, not successful tests. Shared use of AppKit does not prove identical rendering protocols. Loginwindow, Dock and per-user session interactions need explicit tracing. Fast user switching is not evidence that a third party can start an independent replacement graphical session. A second server in a private namespace is a HYPOTHESIS until endpoint routing and hardware access are verified. No alternate session was started.

The current service/entitlement inventory supports a combination of protocol, privilege and graphics-driver obstacles. It does not support the stronger claim that SIP is the only obstacle or that reverse engineering cannot overcome any obstacle. The earliest decision-changing evidence would be a complete isolated AppKit handshake, then native render-transaction/resource submission to an experimental service, and finally documented display-present authority on a spare Apple Silicon machine.
