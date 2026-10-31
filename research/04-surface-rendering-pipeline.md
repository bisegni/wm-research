# Native surface access and independent rendering

Target: macOS 27.0.1 / SDK 27.0, arm64. Evidence is SDK declarations and inspected upstream implementation, not a completed cross-process rendering experiment.

## Critical answer

**CONFIRMED, cooperative applications:** a producer can explicitly export an IOSurface Mach right and a consumer can import it and create a Metal texture over its storage. This does not require ScreenCaptureKit or screen capture.

**UNKNOWN, arbitrary unmodified applications:** no inspected interface establishes a general way to subscribe to all native window backing buffers, their CA layer semantics, commit ordering and producer fences, then replace their final composition. Access to one surface or a hosted CA context does not establish that capability. There is no demonstrated universal impossibility either. The specific missing capability is authoritative acquisition and redirection of the native submission stream, including lifetime/synchronization and presentation authority.

## Evidence retained

`evidence/surfaces/` contains SDK 27.0 copies of IOSurfaceRef.h, MTLDevice.h, MTLEvent.h, CAMetalLayer.h, CATransaction.h and three WebKit files at commit `d05eb32f6fc000907483128650e715b82b9629cc`. SDK provenance: `/Library/Developer/CommandLineTools/SDKs/MacOSX27.0.sdk/System/Library/Frameworks/`.

| Stage | API / inspected location | What it proves / does not prove |
|---|---|---|
| Allocation | `IOSurfaceCreate`, IOSurfaceRef.h | Producer chooses dimensions/format/properties; does not discover other apps' buffers. |
| Export | `IOSurfaceCreateMachPort`, IOSurfaceRef.h:338–343 | Right holds a surface reference and must be deallocated. Secure transfer avoids requiring global visibility. |
| Import | `IOSurfaceLookupFromMachPort`, same header:345–348; WebKit IOSurface.mm:129,160 | Receiver obtains retained object from an available right. No arbitrary acquisition of the right is implied. |
| Texture binding | `newTextureWithDescriptor:iosurface:plane:`, MTLDevice.h:711–717 | Shared storage can back Metal texture; format/plane/device requirements still apply. |
| GPU synchronization | `newSharedEvent`, `newSharedEventWithHandle`, MTLDevice.h:1005–1014; MTLEvent.h | Explicit cooperative synchronization mechanism; not evidence of access to native CA fences. |
| CPU access | `IOSurfaceLock/Unlock`, IOSurfaceRef.h:186–207 | Bounds CPU access and maintains modification seed; not a universal cross-process GPU frame scheduler. |
| Lifetime | IOSurfaceRef.h:390–420 | Per-process/system use accounting supports pooling; use count is not frame readiness. |
| Layer hosting | WebKit QuartzCoreSPI.h:86–120,208–209 | CAContext and CALayerHost support remote context relationships and fences. Hosting remains inside CA; does not decode a tree into independent Metal draws. |
| Remote layers | WebKit RemoteLayerTreeHost.mm, `applyDisplayOnlyImage` | WebKit cooperatively moves layer backing contents; sender ownership is checked in that code. This is WebKit policy, not proof of WindowServer authorization rules. |

## Implementable cooperative pipeline

```text
producer IOSurface → Metal texture → GPU write → signal shared event(value N)
       │ exported Mach right                            │
       ▼                                                ▼
consumer imported IOSurface → Metal texture → wait N → sample / compose
                                                       │
                                      signal consumption / recycle token
                                                       ▼
                              own CAMetalLayer → Apple compositor → display
```

A robust test must negotiate pixel format, plane layout, width/height/stride, color space, alpha convention, device identity, resize epochs and buffer generations. Transfer Mach rights via an owned XPC/Mach channel; authenticate the peer. Retain the surface and texture until GPU reads finish. Use at least a bounded ring or explicit ownership handshake; do not overwrite a buffer still sampled by the consumer. Export the shared event handle separately and increment monotonic event values per frame. CPU waits are acceptable as diagnostic controls, not a proposed low-latency shipping design. These are design requirements, not measured performance results.

The consumer's final CAMetalLayer is still composed by WindowServer. This proves independent rendering of shared content, not ownership of physical scanout or replacement of native window composition.

## Core Animation and Metal distinctions

**CONFIRMED API:** CATransaction groups layer changes; CAMetalLayer drawables are presented to CA. A layer may contain a surface, sublayers, masks, filters, transforms, animations and remote contexts. A native window may have many buffers and no single stable pixel resource. **HYPOTHESIS:** a replacement might reconstruct enough transaction semantics to independently render native layers; current serializer, resource-right transfer and synchronization have not been reconstructed here.

`CAContext`/`CALayerHost` is promising for embedding cooperating remote content. It leaves rendering under CA and needs context access. A context ID is not evidence of permission to import arbitrary foreign content. `IOSurfaceID`, `CGWindowID` and `CAContext.contextId` must never be treated as interchangeable.

Protected content, HDR metadata, color transforms, direct presentation/overlay paths, display changes, occlusion throttling and fullscreen require separate experiments. This investigation establishes no general surface-access entitlement and no blanket ban either. Do not infer a security rule from a failed lookup without testing ownership, lifetime, sandbox and object kind.

## CoreDisplay and presentation

Display configuration, display timing callbacks and final presentation ownership are different powers. Public display discovery or timing does not establish a right to replace the system compositor. The exact macOS 27 CoreDisplay call graph, kernel user-client authorization, DCP scheduling and takeover protocol remain **UNKNOWN**. Asahi documents DCP hardware behavior, but that evidence is from an alternative OS and cannot certify access while macOS WindowServer is active.

## Highest-value experiments

1. **Owned native client boundary trace:** create an AppKit layer-backed window and Metal view in an isolated debug client. Record CA commit/drawable calls, Mach send call stacks, resource names and timing. Success: identify concrete submission/fence families and ownership, not merely a Mach send count. Failure: no attributable resource stream; use static cache analysis rather than claiming absence. No server attachment or interception.
2. **Cooperative IOSurface baseline:** two owned processes, explicit Mach-right/event transfer, producer frame counter and consumer GPU sampling. Success: correct frames through resize and bounded reuse with no read/write races. Failure: document import, format or synchronization error separately. This validates rendering machinery only.
3. **Remote CA context control:** two owned clients compare CALayerHost output with explicit IOSurface composition. Success: establish whether tree semantics remain opaque and whether fences are accessible. Do not enumerate or import unrelated application content.
4. **Submission redirection feasibility:** after recovering actual protocols, propose an isolated bootstrap namespace/client harness. Success requires unmodified test-client APIs emitting usable native transactions to the experimental service. Authorization and feasibility of alternate routing must precede execution. A hand-written client speaking a custom protocol is not native compatibility proof.

No experiment above was executed in this report. See report 09 for the integrated gate sequence.

## Primary references

* [Apple IOSurfaceCreateMachPort](https://developer.apple.com/documentation/iosurface/iosurfacecreatemachport(_:)) and local SDK declarations.
* [Apple Metal drawables](https://developer.apple.com/library/archive/documentation/3DDrawing/Conceptual/MTLBestPracticesGuide/Drawables.html).
* [WebKit IOSurface.mm](https://github.com/WebKit/WebKit/blob/d05eb32f6fc000907483128650e715b82b9629cc/Source/WebCore/platform/graphics/cocoa/IOSurface.mm), `createFromSendRight` and `createSendRight` implementation.
* [WebKit QuartzCoreSPI.h](https://github.com/WebKit/WebKit/blob/d05eb32f6fc000907483128650e715b82b9629cc/Source/WebCore/PAL/pal/spi/cocoa/QuartzCoreSPI.h).
* [Chromium IOSurface engineering notes](https://www.chromium.org/developers/design-documents/iosurface-meeting-notes/): historical synchronization pitfalls, not modern ABI evidence.
* [Asahi display controllers](https://asahilinux.org/docs/hw/soc/display-controllers/).
