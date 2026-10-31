# WindowServer IPC architecture

Target: macOS 27.0.1 build 26A434, arm64. This is a discovery report, not a recovered wire specification.

## Verified service topology

**CONFIRMED:** `/System/Library/LaunchDaemons/com.apple.WindowServer.plist` launches `/System/Library/PrivateFrameworks/SkyLight.framework/Resources/WindowServer -daemon`. `launchctl print system/com.apple.WindowServer` reported a running system LaunchDaemon and active endpoints. [Plist](evidence/windowserver-launch.plist), [runtime service inventory](evidence/windowserver-local.txt).

Declared MachServices on this build:

- `com.apple.windowserver`, `com.apple.windowserver.active`, `com.apple.windowserver.watchdog`
- `com.apple.CARenderServer`
- `com.apple.CoreDisplay.master`, `com.apple.CoreDisplay.Notification`
- `com.apple.iohideventsystem`, `com.apple.backboard.hid-services.xpc`
- `com.apple.VirtualDisplay`, `com.apple.SpatialConnect.host`

The plist marks several services HideUntilCheckIn and some ResetAtClose. Registration does not establish which endpoint a particular application uses or its protocol. `com.apple.windowserver` is verified here rather than copied from historical research. The plist also contains `_Conclave = com.apple.backboardd.conclave`; the exact relationship to Exclaves is UNKNOWN beyond that declaration.

```text
App process                               system launchd domain
  SkyLight client -- lookup? ------------ registered windowserver ports
  QuartzCore      -- lookup? ------------ CARenderServer port
  input client    -- transport? --------- HID service ports
                           all registered by WindowServer job
```

Question marks denote untraced client edges. Multiple endpoint names do not prove separate processes or independently replaceable services.

## Message families and unresolved structures

| Family | Evidence | What is established | What remains UNKNOWN |
|---|---|---|---|
| Connection lifecycle | SLSNewConnection/MainConnectionID exports | Client entry points exist | bootstrap namespace selection, audit negotiation, returned rights, reconnect |
| Window create/release | SLSNewWindow/ReleaseWindow exports + external declarations | API accepts connection/window identities | request ID, packed fields, region serialization, validation |
| Geometry/stacking | bounds/move/order/level entry points | Client operations exist | batching, caching, async completion, cross-owner policy |
| Notifications | RegisterConnectionNotifyProc export | Callback registration API exists | delivery port/shared queue, event enum, payload lifetime and thread |
| Render transaction | CARenderServer registration | Service registered by job | layer serialization, resource references, fences, commit acknowledgement |
| Display configuration | CoreDisplay service registrations | Endpoint names | protocol and authority checks |
| Input | HID service registrations and signed entitlements | Server has HID roles | event queues, focus routing, trust checks |

No Mach message IDs, MIG subsystem numbers or SLS request structures were recovered. Do not invent them by treating API arguments as wire fields. It is UNKNOWN whether any particular SLS operation uses MIG, hand-written Mach messages, XPC, shared memory, or a combination. MIG-generated stubs must be recognized in disassembly before being labeled as such.

Mach's public substrate is distinct from the private protocol. XNU defines message headers, rights, descriptors and trailers in [`osfmk/mach/message.h`](https://github.com/apple-oss-distributions/xnu/blob/main/osfmk/mach/message.h); those structures alone do not specify WindowServer operations. Port names are task-local handles, not global service IDs. A connection integer or window number must not be cast to a Mach port or IOSurface identifier.

## Independent server compatibility

**HYPOTHESIS:** An independent process could implement a version-specific subset once request formats and initialization dependencies are reconstructed. **UNKNOWN:** Whether an unmodified AppKit client can be redirected entirely to that process in an isolated namespace while satisfying session identity and service discovery. No application has run against a replacement in this investigation.

The illustrative create/update/destroy trait omits transport rights, region serialization, notification delivery, render contexts, transaction order, display metadata, input, failure recovery and security identity. A successful `create_window` response alone would not demonstrate AppKit compatibility. Required acceptance: stock AppKit API client establishes connection, opens/closes a window, renders and presents changing contents, receives focus/input, survives resize and server disconnect, without falling back to native server endpoints. QuartzCore and Metal traffic must be accounted for separately.

## Read-only observation plan

An ordinary developer-owned client can be debugged from launch using LLDB; record stack traces at resolved SLS calls and `mach_msg`/`mach_msg2` boundaries, inspect only its messages, and correlate with controlled window changes. Do not attach to WindowServer. Debugger timing distorts latency; tracing is protocol evidence, not performance evidence. Do not log unrelated applications' contents. Trace bootstrap lookup and port-right transfer, preserve trailers and out-of-line descriptors, then compare a second run to separate IDs from constants. Hardware/OS policy may still restrict individual tracing mechanisms; no live tracing permission was tested here.

First implement a test-only Mach echo endpoint to validate the observation toolchain, then observe the real client. An isolated test namespace or explicit test-client endpoint is safer than registering production service names. Never unload the current launchd job to discover behavior.
