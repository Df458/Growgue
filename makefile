include config.mk

# OS-specific config
ifeq ($(OSTARGET), Linux)
CC=gcc
PKGCONFIG=pkg-config
#OSTARGETFLAGS=$(LINUXFLAGS)
#OSTARGETLIBS=$(LINUXLIBS)
else ifeq ($(OSTARGET), Windows)
#OSTARGETFLAGS=$(WINFLAGS)
#OSTARGETLIBS=$(WINLIBS)
CC=i686-w64-mingw32-gcc
PKGCONFIG=i686-w64-mingw32-pkg-config
else
	$(error Unknown OS selected for output. Possible options: Linux, Windows)
endif

# Targets
GAMETARGET=$(APPNAME)
OBJPATH=obj
SRCPATH=src

# Flags
CFLAGS=-Wall -Werror -Wno-unused-but-set-variable -D BUILD_TYPE=$(BUILDTYPE) -D PROJECT_NAME="$(APPNAME)" `$(PKGCONFIG) --cflags lua libxml-2.0`
CLIBS=-l$(CURSES) `$(PKGCONFIG) --libs lua, libxml-2.0`

ifeq ($(CURSES), pdcurses)
	CFLAGS += -D PDCURSES -DNCURSES_MOUSE_VERSION `$(PKGCONFIG) --cflags sdl` -I/usr/include/xcurses
	CLIBS += `$(PKGCONFIG) --libs sdl`
else ifeq ($(CURSES), ncurses)
	CLIBS += -lpanel
endif

# Type-specific config
ifeq ($(BUILDTYPE), Debug)
	CFLAGS += -g -D DEVEL
else ifeq ($(BUILDTYPE), Release)
	CFLAGS += -O3
else
	$(error Unknown build type selected for output. Possible options: Debug, Release)
endif

# Dependency lists
GAMESRCS:=$(wildcard $(SRCPATH)/*.c)
GAMEOBJS:=$(patsubst $(SRCPATH)/%.c,$(OBJPATH)/%.o,$(GAMESRCS))
GAMEDEPS:=$(patsubst $(SRCPATH)/%.c,$(OBJPATH)/%.depend,$(GAMESRCS))

# Directory inits
$(shell mkdir -p $(OBJPATH))

# Filetype rules
$(OBJPATH)/%.depend: $(SRCPATH)/%.c
	@echo -e "Building dependecies for $<..."
	@set -e; rm -f $@; \
	$(CC) -M $(CFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,obj/\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$
$(OBJPATH)/%.o: $(SRCPATH)/%.c
	@echo -e "Building $<..."
	$(CC) -c $< -o $@ $(CFLAGS)

# Rules
all: $(GAMETARGET)

$(GAMETARGET): $(GAMEDEPS) $(GAMEOBJS)
	gcc -o $(GAMETARGET) $(GAMEFLAGS) $(GAMEOBJS) $(CLIBS)

clean:
	rm -rf $(OBJPATH)

# Dynamic includes
-include $(GAMEDEPS)
