# UTM macOS research lab — future runbook

**Status: NOT EXECUTED.** No VM is created, started, or changed as part of this planning task. Follow this runbook only when lab execution is requested. Phase dependencies and acceptance criteria are in [MASTER_TODO.md](MASTER_TODO.md).

## Verified versus pending

- Verified on 2026-10-30: `/opt/homebrew/bin/utmctl` exists; local help exposes `list`, `status`, `start`, `clone`, `stop`, and snapshot commands.
- Pending: UTM version, registered VM inventory, selected UUID/backend, guest OS/SDK, GPU features, guest-agent/SSH access, snapshot support, and recovery.
- The research host baseline does not establish the guest configuration.

## 1. Inventory and select the lab

Future host commands, not an automatic execution script:

```sh
utmctl version
utmctl list
utmctl help start
utmctl help clone
utmctl help stop
utmctl help snapshot
```

Record the selected VM UUID and backend. If no dedicated macOS guest exists, provision one with UTM's macOS Virtualization workflow when authorized. Do not reuse a personal VM without an explicit scope decision. Record the compatible IPSW and guest build; do not assume the CLI offers a `create` subcommand.

## 2. Prepare baseline and working copy

Configure guest tools, a lab account, developer tools, GUI login, and SSH access. Establish actual guest capabilities before relying on `utmctl exec`, file transfer, snapshots, or disposable mode. Use SSH/SCP if available and independently verified.

Shut the baseline down cleanly and verify it is stopped before cloning. Assign the following variables to actual recorded UUIDs; placeholders must be replaced before execution:

```sh
LAB_BASELINE_UUID='<baseline-uuid>'
utmctl status "$LAB_BASELINE_UUID"
utmctl clone "$LAB_BASELINE_UUID" --name 'wm-research-working'
utmctl list
LAB_WORKING_UUID='<new-working-clone-uuid>'
utmctl status "$LAB_WORKING_UUID"
utmctl start "$LAB_WORKING_UUID"
```

Start returning successfully is not a readiness signal. Verify guest boot, GUI login, SSH access, and the graphics baseline separately, using bounded waits in any future automation. Keep the baseline stopped and untouched during experiments.

## 3. Test and collect evidence

Run graphics clients in the guest's intended GUI login session, not blindly from an SSH shell. Keep orchestration on the host and all protocol/server experiments in the working guest. Export logs and manifests before shutdown or rollback. Share only a dedicated exchange directory.

For normal shutdown, request guest power-down and then check status:

```sh
utmctl stop "$LAB_WORKING_UUID" --request
utmctl status "$LAB_WORKING_UUID"
```

The installed CLI describes bare `stop` as forced power-off by default. Do not treat it as clean shutdown. If the guest does not stop within the recorded timeout, record the failure and use the reviewed recovery procedure; do not silently escalate to process killing.

## 4. Prove recovery before server takeover

Use a harmless marker in a disposable clone to demonstrate recovery to the stopped baseline. If using snapshots, verify backend/OS support and exact command help first, then test restore; command presence alone is insufficient. Record boot/login and marker verification after restoration.

A failed experiment leaves the baseline intact. Export available evidence, stop the failed working guest through the recovery procedure, and create a fresh working clone. Retain failed guests until evidence is captured and cleanup is explicitly scoped. No automatic deletion commands are part of this runbook.

## Sources and limits

- [UTM macOS guest support](https://docs.getutm.app/guest-support/macos/): supported installation path and version-dependent VM features.
- [UTM scripting](https://docs.getutm.app/scripting/scripting/): CLI discovery, VM identity, and guest-agent requirements.
- Local `utmctl --help`, `start --help`, `list --help`, `clone --help`, `stop --help`, and `snapshot --help` inspected during planning.

A working macOS VM is a test environment. It does not grant native protocol compatibility, independent guest presentation, or access to physical host display hardware.
