# =========================
# Compiler settings
# =========================

MINGW_CC = x86_64-w64-mingw32-gcc
LINUX_CC = gcc

SDL_PATH ?= /usr/x86_64-w64-mingw32

# =========================
# Targets
# =========================

WIN_TARGET = ds5_test.exe
LINUX_TARGET = ds_verbose_test

# =========================
# Build flags
# =========================

WIN_CFLAGS = -I$(SDL_PATH)/include
WIN_LDFLAGS = -L$(SDL_PATH)/lib \
              -lmingw32 -lSDL2main -lSDL2 -mconsole

LINUX_LDFLAGS = -lasound -lm

# =========================
# Default target
# =========================

all: $(WIN_TARGET) $(LINUX_TARGET)

# =========================
# Windows build (MinGW)
# =========================

$(WIN_TARGET): ds5_test.c
	$(MINGW_CC) $< -o $@ $(WIN_CFLAGS) $(WIN_LDFLAGS)
	cp $(SDL_PATH)/bin/SDL2.dll .

# =========================
# Linux build
# =========================

$(LINUX_TARGET): ds_diag.c
	$(LINUX_CC) -o $@ $< $(LINUX_LDFLAGS)

# =========================
# Clean
# =========================

clean:
	rm -f $(WIN_TARGET) $(LINUX_TARGET) SDL2.dll

# =========================
# Phony
# =========================

.PHONY: all clean
