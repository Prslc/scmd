# scmd

Event-driven battery charge controller daemon for Android/Linux (Magisk / APatch).

Zero CPU usage — sleeps on `epoll_wait`, wakes only on kernel battery uevents
or config file changes.

## Structure

```
├── src/                   # C source
│   ├── core/              #   main loop, context, init
│   ├── io/                #   netlink, inotify, sysfs, uevent parse
│   ├── conf/              #   config file parse
│   ├── log/               #   file logger
│   └── Makefile
├── scmd/                  # Magisk module
│   ├── META-INF/          #   recovery flash support
│   ├── module.prop
│   ├── customize.sh       #   picks the right arch on install
│   ├── service.sh         #   late_start entrypoint
│   ├── config.ini
│   └── bin/               #   built by CI
└── .github/workflows/     # CI → scmd-magisk.zip
```

## How it works

```
kernel battery uevent (netlink)
    │
    ▼
epoll_wait ─── parse CAPACITY=85 ─── cap >= stop? ── write input_suspend=1 (cut)
                                    cap <= start? ── write input_suspend=0 (resume)
    ▲
config file change (inotify, parent dir watch)
```

## Configuration

```ini
stop_capacity=80
resume_capacity=70
trickle=true
debug=false
control_path=/sys/class/power_supply/battery/input_suspend
capacity_path=/sys/class/power_supply/battery/capacity
```

| Key | Default | Description |
|---|---|---|
| `stop_capacity` | 80 | Suspend charging at or above this % |
| `resume_capacity` | 70 | Resume charging at or below this % |
| `trickle` | true | When `stop=100`, wait for charge IC `STATUS=Full` |
| `debug` | false | Enable verbose per-event logging |
| `control_path` | .../input_suspend | Write 1 to cut, 0 to resume |
| `capacity_path` | .../capacity | Read current battery level |

## Build

```bash
cd src && make

# Android ARM64
cd src && make CC=/path/to/ndk/.../aarch64-linux-android21-clang \
                  STRIP=/path/to/ndk/.../llvm-strip
```

## Install

Flash `scmd-magisk.zip` in the Magisk app, or copy the module directory manually:

```bash
cp -r scmd /data/adb/modules/
```

The module starts automatically via `service.sh` on boot. Edit
`/data/adb/modules/scmd/config.ini` to adjust thresholds — the daemon
hot-reloads on save.

## Logging

Appends to `scmd.log` in the module directory. Line-buffered.
