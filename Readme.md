# sop-c-backup

A directory sync and backup tool for Linux, written in C. Watches source
directories through `inotify` and keeps a tree of destination copies in sync,
file-by-file, through a small cluster of forked worker processes talking over
pipes and FIFOs.

Written as a university project for Systemy Operacyjne 1.

## Build

Linux, GCC or Clang, GNU Make.

```bash
make           # debug build with ASan + UBSan + LeakSan
CI=1 make      # release-ish build with -Werror and no sanitizers
```

Produces a single `sop-backup` binary.

## Usage

```bash
./sop-backup
```

Interactive REPL on stdin:

| Command                     | Effect                                                    |
|-----------------------------|-----------------------------------------------------------|
| `add <src> <dst> [dst...]`  | Watch `src`, mirror its contents to each `dst`            |
| `remove <src> <dst>`        | Stop mirroring `src` into `dst`                           |
| `recover <dst> <src>`       | One-shot restore of a backed-up directory                 |
| `list`                      | Print the current source → destinations tree              |
| `exit`                      | Clean shutdown (also handles `SIGINT`/`SIGTERM`)          |

Paths with spaces can be quoted. Everything is canonicalized through
`realpath(3)`; missing destination directories are created.

## Architecture

Three kinds of process wired together with Unix IPC primitives:

```
  stdin ────┐
  inotify ──┼──> [ main ] ──pipe──> [ controller ] ──fork / kill──> [ sync worker ]
  signalfd ─┘       │                                                     ▲
                    │                                                     │
                    └──── FIFO per source (sync events) ──────────────────┘
```

- Over the **pipe**, `main` sends `controller_event`s only — `SYNC_START`,
  `SYNC_STOP`, `RESTORE`.
- Over each **FIFO**, `main` streams `sync_event`s (create / delete / modify)
  straight to every worker watching that source. The controller never sees
  them — it just spawns the readers.

- **main** — parses commands, owns the source/destinations structure, holds
  the recursive `inotify` watch map, and multiplexes `stdin`, inotify, and a
  `signalfd` through `select()`. Each inotify event is translated into a
  `sync_event` and written into the FIFO for the matching source.
- **controller** — forked once at startup. Reads `controller_event`s
  (`SYNC_START`, `SYNC_STOP`, `RESTORE`) from an anonymous pipe from main and
  spawns or terminates sync workers accordingly.
- **sync worker** — one per `(source, destination)` pair. Does an initial
  `copy_directory` of the source, then `select()`s on the per-source FIFO and
  applies incoming create/delete/modify events for files and directories.

Named FIFOs live under `/tmp/dirsync/<main-pid>/<source-key>` — one FIFO per
source, multiplexing out to every worker watching that source.

## Technical content

- Linux-only syscalls: `inotify_init1`, `inotify_add_watch`, `signalfd`,
  `select`, `fork`, `waitpid`, `kill`, `pipe`, `mkfifo`, `fcntl`, `realpath`.
- Recursive inotify with a `wd → path` map; `IN_MOVED_FROM`/`IN_MOVED_TO`
  cookie pairs are matched so moved subtrees keep their watches.
- Partial-read-safe I/O (`read_bulk`/`write_bulk`) in both file copy and event
  framing.
- Fixed-size `controller_event` / `sync_event` structs sent as raw bytes —
  framing by `sizeof`, no serialization layer.

## Credits

Portions of the inotify setup and the `signalfd` scaffold are adapted from
public SOP1 lab examples by SaQQ
([gitlab.com/SaQQ/sop1](https://gitlab.com/SaQQ/sop1)) — credited inline in the
relevant files. The generic vector is from my own
[big-calculator-c](https://github.com/JohnThePenguin/big-calculator-c).
