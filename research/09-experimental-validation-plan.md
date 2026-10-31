# Experimental validation plan

Status: proposed experiments, not completed results. Target baseline: macOS 27.0.1 (26A434), arm64, SIP enabled. Re-run discovery for every OS build. See [binary evidence](02-skylight-api-analysis.md) and [surface boundaries](04-surface-rendering-pipeline.md).

## Decision sequence

```text
Export discovery → owned-client trace → native handoff boundary
                                             |
                   cooperative surface test --+→ redirect feasibility
                                             |
                   isolated protocol server → compatibility corpus
                                             |
                          separate lab display/session investigation
```

The highest-value first experiment is an owned minimal AppKit client's creation and first Core Animation commit trace, recording the connection and surface handoff. Symbol discovery is its prerequisite. A cooperative IOSurface demo alone cannot validate native compatibility.

## Common controls and artifacts

Record OS build, hardware model, SDK version, binary UUID/hash, signing and entitlement state, permissions, executable source hash, exact commands, timestamps and raw trace. Include a no-window baseline and repeated runs; identify call stacks rather than infer causality from nearby timestamps. Do not log unrelated window titles, keyboard input or surface contents. Stop on session instability. Do not attach to WindowServer or Dock, override their bootstrap services, disable SIP, or alter protected files. Those require a separately authorized isolated lab plan.

## A — Export and image discovery

- Hypothesis: client entry points remain dynamically available although ordinary framework files may reside in the dyld shared cache.
- Prerequisites/tools: current build inventory, `file`, `otool`, `nm`, `codesign`, cache metadata tools if already available, a tiny owned `dlopen`/`dlsym` probe.
- Procedure: inventory framework links/cache; query named symbols without invoking mutators; save `dlerror` and image identity. Compare with source declarations, explicitly separating ABI proposals from validated signatures.
- Expected observation: resolved or absent exports for this build.
- Success: reproducible availability inventory. Failure: inability to load an image or identify its cache; report that boundary, not removal of a function.
- Security: ordinary-process library loading only. Do not call unknown signatures.
- Architectural effect: selects adapter version; does not prove authority over foreign windows.

## B — Owned native window lifecycle (first decisive experiment)

- Hypothesis: a client-local trace identifies creation, commit and destruction boundaries without protected-process instrumentation.
- Prerequisites/tools: debug-signed owned AppKit test app, LLDB, matching SDK; no additional privileged entitlements. Approval of any OS debugger prompt belongs to the user.
- Procedure: run baseline NSApplication, then one NSWindow, add a layer-backed view, issue one update, close. Set breakpoints only at verified resolved functions in the owned client; record stack, arguments only after ABI validation, thread, return code and ordering. Repeat with CAMetalLayer and resize. Keep native window on an ordinary desktop without changing global state.
- Expected: connection initialization and native registration paths; CA may batch or defer updates and use other threads.
- Success: reproducible causal chain from owned window to connection/context and first submission. Failure: unavailable symbols or opaque calls; retain partial evidence rather than inventing sequence.
- Security: instrumentation solely of owned client. No event injection or unrelated capture.
- Architectural effect: locates interception candidate and determines whether SkyLight and CA require separate protocol work.

## C — IPC framing and discovery

- Hypothesis: owned-client call stacks and static stubs expose actual service lookup and request families.
- Prerequisites/tools: B trace, matching cache image/disassembly, local launchd plist evidence, LLDB.
- Procedure: inspect `bootstrap_look_up`/XPC and Mach send boundaries in owned client; correlate send rights, message IDs, complex descriptors, out-of-line memory, reply rights and notifications with one action at a time. Observe modern Mach trap variants as applicable. Do not assume every call uses MIG or a single connection.
- Expected: multiple transports or families; identifiers alone do not identify payload semantics.
- Success: validated request/reply pairs and a port-right ownership table for connection and one window operation. Failure: traffic cannot be causally correlated or opaque serialization remains unresolved.
- Security: no port theft, production endpoint substitution, or messages replayed against WindowServer.
- Architectural effect: estimates real compatibility scope; classifies messages independently of illustrative Rust traits.

## D — Native surface handoff

- Hypothesis: the owned client's CA/Metal path exposes a transferable resource together with synchronization and lifecycle metadata.
- Prerequisites/tools: B/C, owned layer-backed and Metal-backed variants, SDK IOSurface/Metal declarations.
- Procedure: correlate drawable acquisition, GPU completion, present, CA commit, surface rights and recycling; include double buffering, resize and occlusion. Inspect only owned allocations. Determine whether resources are full-window images, layer tiles, remote contexts, or indirect handles.
- Expected: allocation and presentation may be decoupled; a visible layer can depend on server-side composition.
- Success: exact resource provenance, producer ownership, acquire/release synchronization and damage/lifetime contract. Failure: only IOSurface IDs or snapshots are obtained.
- Security: no global enumeration of other applications' resources.
- Architectural effect: necessary but insufficient gate for architecture B/hybrid; inability to reproduce metadata keeps native redirection UNKNOWN.

## E — Cooperative independent composition control

- Hypothesis: two owned processes can share an IOSurface and render it using Metal with explicit synchronization.
- Prerequisites/tools: two isolated apps, IOSurface Mach-port transfer, Metal textures, supported shared-event capability checks.
- Procedure: producer writes frame numbers/color patterns; send a legitimate surface right over an owned endpoint; consumer waits on a defined producer completion contract, samples texture and renders into its own drawable. Exercise resize, stale handles, producer exit and buffer reuse; compare frame IDs. Start with serialized completion before optimizing.
- Expected: consumer can render shared pixels; final display remains owned by Apple's presentation path.
- Success: correct ordered frames and cleanup without CPU readback requirement. Failure: import unsupported format/device, rights denial, synchronization defects.
- Security: cooperative sharing only; deallocate rights and retain objects for actual lifetimes.
- Architectural effect: proves renderer mechanics, explicitly NOT unmodified-app interception or physical scanout ownership. Apple documents [Mach-port surface sharing](https://developer.apple.com/documentation/iosurface/iosurfacecreatemachport(_:)).

## F — Isolated protocol compatibility

- Hypothesis: a recovered subset supports one owned native client in an isolated namespace without talking to the production server for those operations.
- Prerequisites/tools: verified C schema, separate explicitly scoped bootstrap namespace, minimal experimental service, owned native client. First establish namespace redirection safely; do not override production labels.
- Procedure: initially use a synthetic protocol client to validate rights/reply handling. Then run the native client against the isolated endpoint only if its lookup can be redirected without system changes. Inventory every additional dependency. Test connect/disconnect, create/destroy, commit, resize and failure recovery.
- Expected: AppKit initialization may require more services than the recovered window family.
- Success: unmodified framework code completes target lifecycle against experimental service, with trace evidence excluding fallback to real WindowServer. Failure: bootstrap redirection unavailable or missing protocol prevents initialization. A synthetic client pass is not native compatibility proof.
- Security: no impersonation in active namespaces; no session replacement. Stop if isolation cannot be demonstrated.
- Architectural effect: first meaningful support for replacement D; only a compatibility corpus across toolkits can justify a project commitment.

## G — Native redirection and presentation gates

- Hypothesis: a lawful client-side or server-mediated mechanism can export complete unmodified application render state with fences and suppress original presentation.
- Prerequisites: D results and concrete mechanism, reviewed lab environment. Not authorized for active-session injection.
- Procedure: first static analysis; subsequently a separately approved lab test comparing normal and redirected AppKit, SwiftUI, Electron, Qt, Metal, video, fullscreen and multi-display clients. Measure pixels, damage, frame timing, input coordinate mapping and accessibility continuity. Protected content must be tracked separately.
- Success: complete native stream and independent composition policy, no original double presentation, correct synchronization. Failure: capture-only images, cooperative changes, missing layers or production-server dependence for final ordering.
- Security: any protected-process modification or display/session takeover requires explicit authorization.
- Architectural effect: passes support hybrid B; otherwise preserve E/further research or C management-only scope.

A separate physical-display gate must establish authorized driver/user-client access, timing, modes, HDR/color management, cursor planes, hotplug, power transitions and recovery. Rendering into an ordinary fullscreen Metal window cannot satisfy this gate. No performance or compatibility results are claimed by this plan.
