SRC = $(wildcard src/*.c)
INCLUDE=include

PROJECT=fcmp

OPT_FLAGS+= -fgraphite -fno-strict-aliasing

RELEASE_CFLAGS?=  -O3 -march=native -flto $(OPT_FLAGS)
RELEASE_LDFLAGS?= -O3 -flto

DEBUG_CFLAGS?=  -O0 -g
DEBUG_LDFLAGS?= -O0

CFLAGS+= -Wall -pedantic --std=gnu11 $(addprefix -I,$(INCLUDE))
LDFLAGS+= 

OBJ = $(addprefix obj/,$(SRC:.c=.o))

.PHONY: release
release: | dirs $(PROJECT)-release

.PHONY: debug
debug: | dirs $(PROJECT)-debug

dirs:
	@mkdir -p obj/src

%.o: %.c
	$(CC) -c $< $(CFLAGS) -o $@ $(LDFLAGS)

$(PROJECT)-release: CFLAGS := $(RELEASE_CFLAGS) $(CFLAGS)
$(PROJECT)-release: LDFLAGS := $(RELEASE_LDFLAGS) $(LDFLAGS)
$(PROJECT)-release: $(OBJ)
	$(CC) $^ $(CFLAGS) -o $@ $(LDFLAGS)
	strip $@

$(PROJECT)-debug: CFLAGS := $(DEBUG_CFLAGS) $(CFLAGS)
$(PROJECT)-debug: LDFLAGS := $(DEBUG_LDFLAGS) $(LDFLAGS)
$(PROJECT)-debug: $(OBJ)
	$(CC) $^ $(CFLAGS) -o $@ $(LDFLAGS)

clean:
	rm -rf obj
	rm -f $(PROJECT)-{release,debug}