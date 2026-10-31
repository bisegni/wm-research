# Executive conclusions

## Decision: E — Further research required

A compatible native compositor cannot yet be justified by the evidence collected. This is an unresolved engineering boundary, not a finding of impossibility. **C — Advanced SkyLight-based window manager** is a credible narrower implementation track, but does not satisfy the original compositor-ownership requirement. Do not relabel it a completed solution to that requirement.

```text
CONFIRMED                    UNKNOWN
private exports exist        complete native rendering handoff
own shell windows            native protocol compatibility
cooperative shared surfaces  independent final display ownership
          \                         /
           policy prototype + targeted boundary research
```

This investigation combines local read-only discovery on macOS **27.0.1 (26A434), arm64**, an isolated symbol-resolution probe, static launcher inspection, SDK/source inspection and pinned open-source implementations. No production-server replacement, foreign-window mutation or protected-process injection was performed. The reports identify unresolved questions rather than filling them with historical assumptions.

## 1. Genuine window manager without replacing WindowServer?

**STRONGLY SUPPORTED: yes for policy and substantial management; no evidence of Wayland-equivalent authority.** yabai geometry/private operations, AeroSpace workspace policy and SketchyBar shell windows establish viable components. Authority differs between ordinary callers and privileged payloads; no blanket current-build permission claim is warranted. App ownership, native fullscreen/Spaces and server policy remain relevant. See [project source review](06-existing-projects-review.md) and [API analysis](02-skylight-api-analysis.md).

## 2. Independent composition of native windows?

**UNKNOWN for arbitrary unmodified applications. CONFIRMED for explicitly shared cooperative resources at the API/source level.** Sharing an IOSurface does not export a complete CA scene, its fences, damage, color semantics, lifecycle or presentation authority. Captured proxy animation is a secondary representation. A Metal compositor in a normal window still submits to Apple’s compositor. See [surface pipeline](04-surface-rendering-pipeline.md).

## 3. Compatible WindowServer replacement?

**HYPOTHESIS, not demonstrated.** A small Rust window-operation trait omits connection negotiation, rights, asynchronous events, CA contexts, rendering state and system/session dependencies. The local executable depends on cached SkyLight; inspecting the small executable is not reversing the server implementation. Existing inspected Darling stubs do not supply compatibility evidence. See [IPC report](03-windowserver-ipc.md), [replacement feasibility](05-windowserver-replacement-feasibility.md), and [source review](06-existing-projects-review.md).

## 4. What prevents complete rendering control?

**STRONGLY SUPPORTED:** missing demonstrated ownership contracts across native submissions, synchronization, input and final presentation; security checks add separate restrictions. **UNKNOWN:** precise current-build authorization and driver paths required by a replacement. SIP status alone neither proves impossibility nor makes a replacement feasible when disabled. Server replacement, protocol emulation, secondary composition and scanout ownership are distinct milestones. See reports [04](04-surface-rendering-pipeline.md) and [05](05-windowserver-replacement-feasibility.md).

## 5. Which limitations could reverse engineering overcome?

**HYPOTHESIS:** ABI declarations, request schemas, notification semantics, connection/context mapping and CA resource publication can potentially be reconstructed. These are research targets, not claimed recovered protocols. **UNKNOWN:** whether sufficient native redirection and hardware authority can be obtained under an acceptable deployment/security model. Stability across OS releases requires repeated evidence rather than one successful build. See [API gaps](02-skylight-api-analysis.md), [IPC gaps](03-windowserver-ipc.md), and [experiments](09-experimental-validation-plan.md).

## 6. Minimum viable implementation?

For the eventual management track: Rust policy/state core, versioned capability adapter, owned shell windows and explicit permission/capability reporting. Keep experimental Metal composition restricted to cooperative surfaces until native handoff is proven. For the actual compositor objective, the minimum viable **proof** is an owned native AppKit/CA/Metal lifecycle whose render submission can be consumed independently with correct lifetime and fences; it must distinguish cooperative test changes from unmodified-client compatibility. No full manager should precede that gate. See [design](08-proposed-system-design.md).

## 7. Most promising long-term architecture?

**Proposed, conditional:** modular native compatibility research plus independent policy, scene, rendering and display interfaces. Advance to a hybrid only after native-stream access and original-presentation suppression are demonstrated. Advance to replacement only after isolated native-client protocol compatibility and a separate display/session ownership gate. This preserves the strongest eventual control without assuming that the easiest policy prototype answers the research question. See [comparison](07-architecture-comparison.md) and [design](08-proposed-system-design.md).

## 8. First validating experiment?

Trace one **owned minimal AppKit client**, from connection/window creation through its first layer-backed update and Metal drawable presentation. Discover actual resource and port handoff, synchronization and context identity; use verified exports and client-local debugger instrumentation. The completed symbol probe is a prerequisite, not this lifecycle experiment. Do not start by injecting into WindowServer or writing a general tiler. See experiments B–D in [validation plan](09-experimental-validation-plan.md).

## What changes the recommendation

- **To B (hybrid):** reproducible native render-state export/redirection for unmodified applications, correct synchronization and suppression of duplicate presentation, with input and display limitations explicitly bounded.
- **To A (replacement):** an isolated server running unmodified native framework clients, a growing toolkit compatibility corpus, and demonstrated authorized display/session ownership.
- **To D (alternative session):** evidence that separate-session routing and hardware ownership are achievable while replacement of the existing session is materially harder; this has not been demonstrated here.
- **To C as the product decision:** explicit acceptance of Apple retaining composition and presentation. This would be a changed scope, not evidence that full composition has been solved.

No latency, power, memory or GPU performance figures are asserted: no compositor was implemented or benchmarked. macOS-version stability, full wire schemas and native-surface interception remain open research gates.
