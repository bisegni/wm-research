# SkyLight API analysis

Target: macOS 27.0.1 (26A434), arm64, inspected 2026-10-30. CONFIRMED means the stated observation, not every inferred behavior. No foreign-window mutation was attempted.

## Artifact identity and discovery

**CONFIRMED:** The framework install-name path is `/System/Library/PrivateFrameworks/SkyLight.framework/Versions/A/SkyLight`. Its on-disk Versions/A directory has Resources and a signature directory but no standalone library. `dlopen` of the framework path succeeds nevertheless. The active cache location is `/System/Volumes/Preboot/Cryptexes/OS/System/Library/dyld/`, with `dyld_shared_cache_arm64e` and many numbered subcaches. This is consistent with cache-resident framework delivery; cache image extraction was not performed. A missing standalone file must not be interpreted as a missing framework.

The executable is `Resources/WindowServer`; SHA256 `51a949ae77519ecca2efe2d9176cf85636a9fd91e2ba5b70817616c3b315ed7b`. Its load commands depend on SkyLight version 600.0.0 and libSystem. Thus SkyLight is not merely an application-side API facade: server code can also reside in the framework. See [local evidence](evidence/windowserver-local.txt).

An isolated C process performed only `dlopen` and `dlsym`, never invoking any recovered function. All nine names below resolve on this host. [Probe source](evidence/skylight-symbols.c), [output](evidence/skylight-symbols.txt). This proves availability, not argument validity, authority, or semantics.

## Interface map

Signatures below are source declarations from yabai, not ABI independently recovered from this OS. `cid` is a connection integer; `wid` a uint32 window ID; `CGError` is an error result (not bool). Success/error behavior on this host remains untested. The callback typedef and declarations are preserved in [source snapshot](evidence/yabai-extern.h).

| Interface / declaration | Purpose inferred from callers/naming | IPC mechanism | Evidence | Confidence |
|---|---|---|---|---|
| `int SLSMainConnectionID(void)` | Obtain process main connection | May return cached client state; exact initialization unknown | dlsym + yabai extern.h | CONFIRMED export; semantics source-supported |
| `CGError SLSNewConnection(int zero,int *cid)` | Establish connection and return identifier | Wire handshake unknown | same | CONFIRMED export; UNKNOWN handshake |
| `CGError SLSGetWindowBounds(int cid,uint32_t wid,CGRect *frame)` | Query bounds into out parameter | Cache versus synchronous message not traced | same | CONFIRMED export |
| `CGError SLSMoveWindow(int cid,uint32_t wid,CGPoint *point)` | Move window | Exact message unknown | same | CONFIRMED export |
| `CGError SLSOrderWindow(int cid,uint32_t wid,int mode,uint32_t rel_wid)` | Relative ordering | Exact message unknown | same | CONFIRMED export |
| `CGError SLSSetWindowLevel(int cid,uint32_t wid,int level)` | Window stacking level | Exact message unknown | same | CONFIRMED export |
| `CGError SLSRegisterConnectionNotifyProc(int cid,connection_callback *handler,uint32_t event,void *context)` | Register callback | Delivery transport/thread unknown | same | CONFIRMED export |
| `CGError SLSNewWindow(int cid,int type,float x,float y,CFTypeRef region,uint32_t *wid)` | Allocate native window identity | Exact request/region encoding unknown | same | CONFIRMED export |
| `CGError SLSReleaseWindow(int cid,uint32_t wid)` | Release window | Exact message unknown | same | CONFIRMED export |

## Permissions and lifecycle boundary

**UNKNOWN:** Whether any specific mutation succeeds on another process's window in this build. An exported entry point does not remove server ownership checks. Accessibility consent, private API availability, Dock-mediated authority and protected-process injection are different permissions. Do not encode “SIP disabled” as a universal requirement for every SLS call.

Provisional lifecycle: application main-connection initialization → window registration → surface/context association → transaction updates → server presentation → release. Only symbol availability and declared API shape were tested here; this sequence is not a traced AppKit call graph. Creation may select a richer entry point such as `SLSNewWindowWithOpaqueShapeAndContext`, also declared by yabai; it was not included in the export probe.

## Reverse-engineering coverage and next step

REA/Hopper opened the WindowServer executable and recovered six segments, 23 procedures and 17 strings (including external-symbol segment), evidence `ev_198aa0ecb2d9a8f54765fbcddcc8e35076e5a8611fc5ede99ef7699a67c6be10`, [retained snapshot](evidence/windowserver-rea.json). These counts describe the small launcher, not the cached SkyLight server implementation. A `_main` lookup failed; no main-function decompilation is claimed. Session closed cleanly. Extract the exact cached framework with a compatible cache extractor, retain UUID/hash and inspect per-function client stubs before assigning Mach IDs. Do not scan unrelated cache strings and attribute them to SkyLight.

Historical CGS/SLS names and modern yabai declarations establish research leads, not cross-version ABI stability. No second macOS build was examined; removed/hidden exports and signature changes remain UNKNOWN. Repeat the included probe on each supported build and record results separately.

Source: [yabai pinned extern.h](https://github.com/koekeishiya/yabai/blob/dd845723416f5fe92af49fad5ebab00369e07edd/src/misc/extern.h). The master snapshot was compared with a fresh fetch of this exact commit using `cmp`; they are byte-identical (exit 0). The pinned copy is retained as `evidence/yabai-extern-pinned.h`.
