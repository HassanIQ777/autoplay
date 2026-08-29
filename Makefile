# Makefile template by (HassanIQ777)[https://github.com/HassanIQ777]
# Now with a STATIC mode, for when you want your friend's machine
# to have zero say in the matter.

CXX := g++
TARGET := autoplay
BINDIR := .

PREFIX  := $(HOME)/.local
INSTDIR := $(PREFIX)/bin

# Library directory
LIB_UTILS_DIR     := libutils
LIB_UTILS_SRC_DIR := $(LIB_UTILS_DIR)/src
LIB_UTILS_LIB     := $(LIB_UTILS_DIR)/build/libutils.a

# Root program sources
ROOT_SRCS := $(wildcard *.cpp)
ROOT_OBJS := $(patsubst %.cpp,%.o,$(ROOT_SRCS))

# If libutils exists, add includes
ifeq ($(wildcard $(LIB_UTILS_DIR)),)
  INCLUDES  :=
  LIB_TARGET :=
  LIB_LINK  :=
else
  INCLUDES := -I$(LIB_UTILS_DIR)
  LIB_TARGET := $(LIB_UTILS_LIB)

  # libutils.a is only real linker fodder if it was actually built from
  # .cpp translation units. If libutils is header-only, `ar` packing
  # the .hpp files into it is ceremonial - a .a of headers isn't valid
  # object code, and handing it to the linker just gets you
  # "archive has no index; run ranlib to add one" followed by nothing
  # ranlib can actually fix. So: build it (for the ritual), but only
  # feed it to the linker when it holds real symbols.
  LIB_UTILS_REAL_SRCS := $(wildcard $(LIB_UTILS_SRC_DIR)/*.cpp)
  ifeq ($(LIB_UTILS_REAL_SRCS),)
    LIB_LINK :=
  else
    LIB_LINK := $(LIB_UTILS_LIB)
  endif
endif

OBJS := $(ROOT_OBJS)

# ---- Compiler flags ----
WARNINGS := -Wall -Wextra -Wpedantic -Wshadow -Wconversion

RELEASE_FLAGS := -std=c++20 -O2 -march=native -flto $(WARNINGS)
DEBUG_FLAGS   := -std=c++20 -g -Og -DDEBUG $(WARNINGS) \
                 -fsanitize=address,undefined -fno-omit-frame-pointer

# Default is release
CXXFLAGS := $(RELEASE_FLAGS)

# ---- Static linking knobs ----
# debug:   fully dynamic - ASan/UBSan don't get along with -static anyway
# release: static runtime only (libgcc/libstdc++ baked in, system libs
#          like libc/libSDL2/libncurses stay dynamic - the sane default)
# static:  the nuclear option, everything including glibc goes in the
#          binary. Single file, runs anywhere, no install step for your
#          friend. Caveat: if you ever call getaddrinfo()/gethostbyname()
#          for DNS lookups, glibc's NSS still wants to dlopen() plugins
#          at runtime even in a "static" binary - raw-IP socket code is
#          unaffected, but hostname resolution can misbehave. GUI libs
#          (SDL2, X11/Wayland, ALSA) should NOT be dragged into this -
#          add them back as dynamic (-lSDL2, drop from STATIC_FLAGS)
#          if TARGET needs them.
STATIC_FLAGS :=

.PHONY: all debug release static clean run install uninstall verify

all: release

release: STATIC_FLAGS := -static-libgcc -static-libstdc++
release: CXXFLAGS := $(RELEASE_FLAGS)
release: $(BINDIR)/$(TARGET)

debug: STATIC_FLAGS :=
debug: CXXFLAGS := $(DEBUG_FLAGS)
debug: $(BINDIR)/$(TARGET)

static: STATIC_FLAGS := -static
static: CXXFLAGS := $(RELEASE_FLAGS)
static: $(BINDIR)/$(TARGET)
	@echo "$(TARGET) is now hermetically sealed. glibc, libstdc++, libutils -"
	@echo "all of it, entombed inside one file. Ship it and walk away."

# Link step
$(BINDIR)/$(TARGET): $(OBJS) $(LIB_TARGET)
	@mkdir -p $(BINDIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $(OBJS) $(LIB_LINK) $(STATIC_FLAGS)
	@echo "Built -> $@"

# Compile rules
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# If libutils.a exists and has its own Makefile, defer build to it
$(LIB_UTILS_LIB):
	@if [ -f $(LIB_UTILS_DIR)/Makefile ]; then \
	  echo "Building libutils..."; \
	  $(MAKE) -C $(LIB_UTILS_DIR); \
	fi

run: all
	@./$(BINDIR)/$(TARGET) $(ARGS)

# Prove the binary is what you think it is, instead of taking Make's word for it
verify: all
	@echo "--- file ---"
	@file $(BINDIR)/$(TARGET)
	@echo "--- ldd ---"
	@ldd $(BINDIR)/$(TARGET) 2>&1 || true

install: release
	@echo "Installing $(TARGET) to $(INSTDIR)..."
	@install -Dm755 $(BINDIR)/$(TARGET) $(INSTDIR)/$(TARGET)
	@echo "$(TARGET) is now a system citizen. Run it from anywhere."

uninstall:
	@echo "Evicting $(TARGET) from $(INSTDIR)..."
	@rm -f $(INSTDIR)/$(TARGET)
	@echo "$(TARGET) has been yeeted into the void."

clean:
	-@rm -f $(OBJS)
	-@rm -f $(BINDIR)/$(TARGET)
	@echo "Cleaned up the ashes. Nothing but echoes remain..."

.SUFFIXES:
#
