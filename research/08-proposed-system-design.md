# Proposed system design: policy core with independently gated native compositor research

This is a design proposal, not implemented software. The source-backed practical path is an advanced native manager; the intended compositor remains gated on proof of native surface ingress and presentation authority. [Architecture comparison](07-architecture-comparison.md) explains why.

```text
                 immutable observations / capability results
Native adapters ----------------------------------------------> policy core
  AX / SkyLight <----------------------------------------------- desired actions
       |                    reconciler and capability checks
       v
Apple WindowServer                research-only surface adapter
       |                                    |
native display output             SurfaceLease + synchronization
                                            |
                                   Metal scene renderer
                                            |
                             nested presentation | future display backend
```

The right-hand branch is not a proven path for arbitrary native windows. Do not draw it as a production connection from WindowServer until experiment D demonstrates that connection.

## Modules

```text
custom-macos-wm/
  policy/              windows, layout, workspaces, focus intent
  reconcile/           observed state versus desired state; retry/error rules
  platform/macos/      AppKit/AX adapters and public display observation
  platform/skylight/   isolated versioned private ABI and capability probes
  protocol-research/   Mach traces, schemas, compatibility fixtures
  surfaces/            typed ownership, lease, ready/release synchronization
  renderer/metal/      scene, damage, effects, color, resource lifetime
  presentation/        nested native output; future independent output contract
  input/               observation, actions, coordinate mapping, security states
  shell/               own panels, launcher, widgets and desktop windows
  research/            evidence, experiments, version matrix, decisions
```

Use Rust for deterministic state, protocol parsing and resource lifetimes; use a narrow Objective-C/C bridge where AppKit/CA runtime interaction requires it. Metal rendering is appropriate only after an input buffer contract exists. No architectural requirement to recreate SwiftUI/AppKit itself has been established.

## Contracts

**Native compatibility adapter:** emits application/connection/window observations, with origin, timestamp and generation. A window ID alone is not a stable lifetime identity. Represent `(connection generation, window ID, observation generation)` internally until the true server lifecycle is established. Never assume AX IDs, SkyLight connection IDs, IOSurface IDs and CA context IDs are interchangeable.

**Policy core:** consumes normalized observations and emits desired geometry, stacking, focus and workspace membership. It knows no `SLS*` symbols. Keep native Spaces separate from logical workspaces; AeroSpace's corner-hiding implementation shows why they are not synonyms. The reconciler reports applied, denied, unsupported, stale or failed actions rather than silently claiming success.

**Capability adapter:** returns capabilities per operation and target ownership, including OS build, ABI provenance, permission state and test result. Own-window success does not enable foreign-window mutation. Private FFI lives in one versioned boundary; unknown signatures are not called. No automatic protected-process payload loading or SIP changes.

**Surface management:** a proposed `SurfaceLease` includes producer identity, dimensions, format, color space, scale, crop, content generation, ready synchronization, release obligation and ownership origin. A raw IOSurface number is insufficient. Buffer reuse must wait for consumer GPU completion, with bounded queues and backpressure. Distinguish captured image, cooperative surface, native transaction surface and renderable layer tree in the type system.

**Scene and GPU composition:** accepts only acquired, validated leases. Applies transform/opacity/clip/effects and tracks damage. Layer-tree reconstruction cannot be replaced by assuming one flat texture per window. HDR/EDR, protected content, subsurfaces, video overlays, cursor, shadows and native decorations require explicit representation or an explicit unsupported state.

**Presentation:** `NestedNativeWindow` is the first proposed backend. Its completion is not physical scanout completion. A future `IndependentDisplay` backend must expose mode validation, commit, frame timing, presentation feedback, hotplug and recovery. It cannot be implemented by renaming `CAMetalDrawable.present` as direct display ownership.

**Input:** keep observation, permitted event synthesis and authoritative routing as distinct capabilities. A transformed visual representation requires inverse-coordinate hit testing but that alone does not make native focus, grabs, IME, accessibility, secure input or drag-and-drop correct. Unknown routing remains a release blocker for a full compositor.

**Shell:** ordinary native clients for panels and widgets. Implement only after the core feasibility gates; shell polish cannot compensate for missing surface ingress.

## Staged implementation contract

1. Read-only inventory and reproducible probes; preserve exact OS/build and evidence.
2. Trace an owned native AppKit/Metal process to locate actual connection and submission boundaries: the first decisive experiment.
3. Isolated cooperative producer/consumer validates GPU lifetime and synchronization only.
4. Resolve arbitrary-native surface intake and suppression of original presentation. Until then no claim of native compositor ownership.
5. Build the minimum policy core and own-window shell if B is accepted as an interim product.
6. Only after protocol proof, create a nested compatible server in an isolated bootstrap namespace. No production endpoint replacement.
7. Investigate physical presentation independently, requiring separate authorization for disruptive experiments.

## Acceptance and rollback

Each backend reports its tested build and capabilities. Disabling a private backend must preserve the policy model and release owned resources. Never leave foreign windows hidden after a failed proxy animation; use a recovery journal only where the reverse operation is verified. Reject stale window generations. Bound IPC payload sizes and validate Mach descriptors before mapping memory. A failed research transport must not affect the login session.

Minimum native-compositor acceptance requires unmodified AppKit and Metal clients, resize/occlusion/fullscreen correctness, frame lifetime correctness, input/focus correctness and documented presentation ownership. Passing cooperative IOSurface sharing alone satisfies none of the unmodified-client compatibility gates. See [validation plan](09-experimental-validation-plan.md).
