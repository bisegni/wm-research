# E01 — Owned AppKit window lifecycle trace

Status: **SPECIFIED / NOT EXECUTED**. This is the first bounded P3 experiment in the [master TODO](MASTER_TODO.md). P1 recovery and the relevant P2 guest baseline checks must pass first. No VM setup, app implementation, or debugger execution is performed by this document.

## Question

Which client-side calls and resources connect one native AppKit window's creation, first visible update, resize, and destruction to Apple's guest WindowServer?

The output is a causal trace and a list of unresolved boundaries to guide protocol research. This experiment alone cannot recover the full protocol, demonstrate redirection, or complete P3.

```text
Host controller → guest GUI session → owned test application
                                             |
                        phase markers + client-only LLDB trace
                                             |
                              evidence + boundary report
```

## Scope and prerequisites

Use one dedicated UTM working guest, with Apple's WindowServer running normally. Record guest OS/build, SDK/toolchain, UTM/backend, source revision, app executable hash, signing/debugger state, GUI user/session, and VM UUID. Keep SIP enabled. Confirm independent SSH recovery and artifact transfer according to [VM_LAB.md](VM_LAB.md).

First verify that an owned debug application can be launched under LLDB in the intended guest GUI session. An SSH connection alone is not proof of correct GUI-session placement. If debugger permission requires a macOS prompt, the user handles that prompt; record it without changing security settings automatically.

## Proposed test application

Implement later as a minimal Objective-C AppKit executable linked to AppKit and QuartzCore, with debug symbols and a fixed source revision. Use a layer-backed custom view and explicit color changes; no private API calls or manual IOSurface export. Application actions run on the main thread.

| Phase marker | Action |
|---|---|
| `APP_READY` | Initialize NSApplication and enter the event loop. |
| `WINDOW_CREATE_BEGIN/END` | Construct a 640 × 480 point content window and show it. |
| `COLOR_RED_BEGIN/END` | Set the view's layer background to red in a transaction. |
| `COLOR_BLUE_BEGIN/END` | On a subsequent scheduled event-loop step, change it to blue. |
| `RESIZE_BEGIN/END` | Set content size to 800 × 600 points and let layout run. |
| `WINDOW_CLOSE_BEGIN/END` | Close the owned window. |
| `APP_EXIT` | Terminate normally after pending observations are collected. |

Markers contain a monotonic timestamp, sequence number, PID, thread identity, and requested state. A transaction or method returning is a submission observation, not proof of visible presentation. Keep the blue state visible for a configurable observation interval in the baseline; do not use a fixed timer as a debugger readiness signal.

Add a `no-window` control mode that initializes the same application/event loop and emits matching phase boundaries without creating a window. Omit Metal in E01; use it only in a later E02 variation once the AppKit trace is understood.

## Execution design — future automation

1. **Preflight:** validate explicit guest UUID/session identity, debugger access, writable evidence directory, and prerequisite results. Abort on missing identity or readiness; do not choose an arbitrary VM.
2. **Normal baseline:** run the owned app without LLDB. Collect action logs and exit status. Confirm red → blue, resize, and close visually. A screenshot may supplement this only if capture is available and consented; never infer presentation from application markers alone.
3. **Resolve candidates:** inspect symbols/images in the owned process on this guest. Record addresses, image UUIDs, resolution failures, and all actual breakpoint locations. Never invoke a private symbol to test it.
4. **Control trace:** run `no-window` under LLDB, logging candidate hits and stacks.
5. **Window trace:** run the same binary with window actions under LLDB, logging phase markers and relevant breakpoint hits. Repeat once on the same guest/build to distinguish stable behavior from per-run identities. Collect additional runs only to resolve a specific discrepancy.
6. **Correlate:** compare the control and window traces. Identify call stacks attributable to the controlled actions, separate asynchronous activity, and record gaps.
7. **Export:** preserve logs and a results table on the host. Leave the guest session intact; no production service modifications are needed.

Initial candidate breakpoints are `SLSMainConnectionID`, `SLSNewConnection`, `SLSNewWindow`, `SLSNewWindowWithOpaqueShapeAndContext`, and `SLSReleaseWindow`, plus public AppKit window lifecycle and `CATransaction` commit methods. These are investigation candidates from [report 02](../research/02-skylight-api-analysis.md), not a verified call path. Absence or zero hits is a finding, not evidence that an operation does not occur.

Begin with hits, thread IDs, timestamps, and bounded backtraces. Read private arguments/return values only after validating the exact guest ABI. Raw integers are not automatically window, context, surface, or port identities. Breakpoints on broad Mach send/receive paths are a separately bounded follow-up after a relevant stack identifies the transport; avoid collecting every unrelated client message.

Future automation must enumerate pending/resolved breakpoints before running, persist errors, and continue after logging only at explicitly configured tracepoints. Use a 120-second wall-clock timeout per unattended run; an interactive permission prompt pauses the workflow for user handling instead of triggering a retry loop. On timeout, interrupt and terminate only the owned app/debugger, export partial evidence, and mark the run failed. Never restart guest WindowServer to recover this experiment.

## Who does what

| Step | Expected owner | Verification |
|---|---|---|
| Initial guest setup/login | User interaction may be needed | Guest GUI and SSH readiness |
| macOS permission prompts | User if prompted | Explicit permission outcome recorded |
| Build and launch owned app | Agent automation, after implementation authorization | Source/hash, command, exit status |
| Configure LLDB and collect traces | Agent automation | Resolved locations, logs, timeout outcome |
| Red/blue, resize, close visual check | User initially; automate later if validated capture is available | Explicit observation; otherwise visual result remains UNVERIFIED |
| Correlation and report | Agent | Claim-by-claim evidence links |

The experiment should not require manual breakpoint stepping or repeated hand-driven window actions. Fully unattended visual validation remains conditional on a tested guest capture path.

## Evidence contract

Future runs live under `research/evidence/vm/<run-id>/e01/`. Preserve:

- `manifest.json`: identities, prerequisites, exact build/launch/debugger commands, timestamps, source/binary hashes, timeout configuration, and permission outcomes.
- `baseline.jsonl`, `control.jsonl`, and `window.jsonl`: application markers and requested state.
- Separate raw LLDB stdout/stderr per run, symbol-resolution inventory, breakpoint-hit counts, image identities, and backtraces.
- `observations.md`: visual confirmation, lifecycle-to-stack mapping, any validated resource identities and provenance, repeatability comparison, failures, and unknowns.

Do not pre-create successful result artifacts. Record identifiers as observed values with a stated source; missing resource/transport evidence remains UNKNOWN.

## Acceptance and next decision

| Criterion | Required evidence |
|---|---|
| Baseline app behavior | Normal exit, ordered markers, and explicit visual confirmation for color/resize/close |
| Trace coverage | Resolved breakpoint inventory and action-attributed stacks, including initialization and close paths |
| Causality | No-window comparison and second window trace; distinguish observed ordering from proven causation |
| Resource boundary | Validated connection/window/context/surface observations where accessible; unknown fields explicitly listed |
| Reproducibility | Fixed source/build identities, complete commands, raw logs, and repeat outcome |

Mark **E01 PASS** only when baseline behavior and a repeatable creation/update/close call-chain observation are supported. If a boundary remains opaque, report **PARTIAL** with the exact missing evidence; debugger/GUI failures are **FAIL**, and unperformed checks are **UNVERIFIED**. E01 PASS does not satisfy P3's protocol/resource-lifetime and isolated-routing gates.

Next select the smallest missing observation: a guest-matching client stub inspection, a targeted transport trace, or E02's CAMetalLayer variation. Do not start implementing a server based only on symbol names or nearby timestamps.

## Review boundary

The design intentionally leaves breakpoint addresses and private ABI details unresolved until guest discovery. Debugger stops distort timing, so this is protocol/lifecycle research rather than a performance benchmark. Implementation and VM execution remain pending a request to begin the prerequisite execution phases.
