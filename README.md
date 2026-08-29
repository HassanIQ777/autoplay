# Project-Template

Makes it more streamlined to start a new simple C++ project.

No boilerplate archaeology, no re-writing the same Makefile for the tenth time — clone this, rename a few things, and start writing `main()` instead of scaffolding.

## What's in the box

```
Project-Template/
├── Makefiles/
│   ├── Regular/
│   │   └── Makefile   # standard build
│   └── Static/
│       └── Makefile   # statically linked build
├── libutils/          # git submodule → HassanIQ777/libutils
├── main.cpp           # your project starts here (currently: int main() {})
├── update.sh          # pulls latest + syncs submodules, no mercy
├── .gitmodules
├── .gitignore
└── LICENSE            # MIT
```

[libutils](https://github.com/HassanIQ777/libutils) is wired in as a **git submodule**, not a copy-pasted folder — meaning it stays a real, independently-versioned repo instead of quietly rotting inside this one.

## Getting started

Clone it whole, submodule and all, in one shot:

```bash
git clone --recurse-submodules https://github.com/HassanIQ777/Project-Template.git
```

Forgot the flag? No harm done, just finish the job:

```bash
git submodule update --init --recursive
```

(Skip this and `libutils/` shows up as an empty, judgmental folder.)

## Building

Copy whichever Makefile you want to your project's root directory.

Two flavors live under `Makefiles/`:

```bash
# Regular (dynamically linked)
cd Makefiles/Regular
make

# Static (statically linked binary — no runtime dependency hunting)
cd Makefiles/Static
make
```

> Double-check the exact targets (`make help`, or just open the Makefile) since these are meant to be adapted per-project — treat them as a starting skeleton, not a locked-in contract.

## Keeping it fresh — `update.sh`

```bash
chmod +x update.sh && ./update.sh
# or quickly
bash update.sh
```

This does exactly what it says on the tin, and the tin has a warning label:

```bash
# WARNING:
# This hard resets any project in which you run this
# Use this to update any program and its submodules to their latest versions
# (this is NOT the same as submodules' own latest. bump pins separately when needed)
```

Under the hood: `git fetch` → `git reset --hard origin/main` → `git submodule update --init --recursive`. It snaps your local copy (and `libutils`) to exactly what's pinned on `origin/main` — no half-updated limbo, no "works on my machine." Just know `--hard` means **local changes get vaporized**, not politely stashed. Commit or lose it.

If you specifically want `libutils` chasing its own latest upstream commit (rather than whatever's pinned here), that's a manual, deliberate step:

```bash
cd libutils
git pull origin main
cd ..
git add libutils
git commit -m "bump libutils"
```

## License

MIT — see [`LICENSE`](./LICENSE). Do whatever you want with it, just don't blame me if your Makefile achieves sentience.