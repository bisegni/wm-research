# Candidate architecture comparison

Evidence baseline: [source review](06-existing-projects-review.md), [native surfaces](04-surface-rendering-pipeline.md), [IPC](03-windowserver-ipc.md), and [replacement](05-windowserver-replacement-feasibility.md). Assessments concern architectural capability, not measured performance. No compositor benchmarks were run.

| Dimension | A: supported API policy manager | B: private SkyLight manager | C: custom composition beside WindowServer | D: compatible replacement |
|---|---|---|---|---|
| Unmodified native applications | Existing native stack retained; AX participation varies | Existing native stack retained; foreign-window authorization varies | Normal execution retained, but direct live surface intake UNKNOWN | UNKNOWN until native client conformance demonstrated |
| Window policy | Geometry/focus through exposed app interfaces | More ordering, transforms, Spaces and shell operations where authorized | Policy still requires A/B or a new server arrangement | Potentially complete; protocol and semantics unresolved |
| Rendering control | Own UI only | Server-executed transforms/effects; captured proxies demonstrated | Own renderer confirmed for cooperative buffers; arbitrary native buffers UNKNOWN | Potentially complete after CA/Metal interoperability |
| Input | Observers/taps and permitted injection; not an authoritative seat | Additional private routing leads; no proven complete ownership | Must reconcile hit testing and coordinate transforms with native server | Must implement routing, focus, grabs, keyboard layouts, IME and secure paths |
| Presentation | WindowServer | WindowServer | Secondary native window output remains WindowServer-composited | Display device access and timing contract unresolved |
| Performance | No extra full-scene render required | Proxy captures may add allocations/copies and latency | May duplicate composition; shared memory alone does not remove synchronization overhead | No measured result; could remove a composition stage only if direct presentation succeeds |
| Security | AX/input permissions as applicable | Some actions possible in client; privileged extensions are separate | Sharing permissions plus any attempted foreign pipeline access | Launch/session/display authority and protected components |
| Complexity | App interoperability and policy | A plus unstable ABI and capability tests | B plus surface discovery, fences, lifetime, occlusion and presentation | Entire compatibility stack, including non-window services |
| Maintenance | App quirks and supported API evolution | High version sensitivity | High, with multiple private subsystems | Very high compatibility burden per OS/client generation |
| Apple Silicon | Normal native frameworks | Source has arm64-specific support, host acceptance still needed | Metal/IOSurface viable building blocks; takeover unproved | GPU/display protocol and driver authority unresolved |

## Interpret the choices precisely

**A is CONFIRMED as a policy architecture**, but does not meet the requested compositor objective. Native decorations remain app/server produced. Existing applications can refuse or constrain geometry; creation is observed rather than authorized by this manager.

**B is STRONGLY SUPPORTED as the greatest immediately evidenced native control.** yabai's capture/proxy animation and SketchyBar's own-window creation establish more than simple tiling. They still do not confer interception of every window creation, live foreign layer-tree ownership, authoritative input dispatch, or independent scanout. Calling this a Wayland-equivalent compositor would be incorrect.

**C has two materially different meanings.** A nested renderer consuming voluntarily exported IOSurfaces is technically credible; a renderer for arbitrary unmodified application windows is UNKNOWN. Capturing windows and drawing them in a fullscreen native window yields a secondary representation and leaves native routing/visibility problems. Neither establishes control of the original pipeline. A viable hybrid must demonstrate continuous frames including occlusion, resize, fullscreen, Metal and CA animations, and explain suppression of original presentation without stalling the producer.

**D remains a research hypothesis.** An empty AppKit window is already more than four CRUD requests: connection setup, events, notifications, geometry, surfaces, transactions and synchronization participate. Replacing the executable, impersonating its Mach endpoint, implementing CA render transport, and obtaining physical display ownership are separate milestones. Success at one cannot be used as evidence for another.

## Decision rule

For a deliverable policy manager today, choose B with per-action capability checks and a supported fallback. For the user's full native compositor objective, choose E (further research) until C's surface ownership boundary is resolved. Do not commit to A (full replacement recommendation) merely because symbols can be resolved. Do not rule out D merely because it is undocumented.

Evidence that changes the decision: a repeatable isolated unmodified AppKit/Metal client publishing live native content to an independently controlled endpoint, with correct buffer release and input semantics. For a complete compositor claim, add independent display presentation or an explicitly defined Apple presentation-only service. No arbitrary numeric scores are assigned because bandwidth, latency, compatibility coverage and maintenance effort have not been measured.
