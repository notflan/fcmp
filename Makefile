SRC = $(wildcard src/*.c)
INCLUDE=include

PROJECT=fcmp

OPT_FLAGS?= -march=native -fgraphite -fopenmp -floop-parallelize-all -ftree-parallelize-loops=4

FEAT_CFLAGS?= -D_RUN_THREADED=0
FEAT_LDFLAGS?= -lpthread

RELEASE_CFLAGS?=  -O3 -flto $(OPT_FLAGS)
RELEASE_LDFLAGS?= -O3 -flto

DEBUG_CFLAGS?=  -DDEBUG -O0 -g
DEBUG_LDFLAGS?= -O0

CFLAGS+= $(FEAT_CFLAGS) -Wall -pedantic --std=gnu11 $(addprefix -I,$(INCLUDE))
LDFLAGS+= $(FEAT_LDFLAGS)

# PGO specific vars

PROF_ITERATIONS=50
PROF_LARGE_BOUND= $$(( 1024 * 1024 * 10 ))
PROF_SMALL_BOUND= $$(( 1024 * 10 ))
PROF_LOCATION?=/tmp/fcmp-pgo

PROF_FLAGS = -fprofile-generate

OBJ = $(addprefix obj/,$(SRC:.c=.o))
PGO_OBJ = $(addprefix prof/,$(SRC:.c=.o))

.PHONY: release
release: | dirs $(PROJECT)-release

.PHONY: debug
debug: | dirs $(PROJECT)-debug

.PHONY: pgo
pgo: | dirs $(PROJECT)-pgo

dirs:
	@mkdir -p {obj,prof}/src

obj/%.o: %.c
	$(CC) -c $< $(CFLAGS) -o $@ $(LDFLAGS)

prof/%.o: %.c
	$(CC) -c $< $(CFLAGS) $(PROF_FLAGS) -o $@ $(LDFLAGS) $(PROF_FLAGS)

$(PROJECT)-release: CFLAGS := $(RELEASE_CFLAGS) $(CFLAGS)
$(PROJECT)-release: LDFLAGS := $(RELEASE_LDFLAGS) $(LDFLAGS)
$(PROJECT)-release: $(OBJ)
	$(CC) $^ $(CFLAGS) -o $@ $(LDFLAGS)
	strip $@

$(PROJECT)-debug: CFLAGS := $(DEBUG_CFLAGS) $(CFLAGS)
$(PROJECT)-debug: LDFLAGS := $(DEBUG_LDFLAGS) $(LDFLAGS)
$(PROJECT)-debug: $(OBJ)
	$(CC) $^ $(CFLAGS) -o $@ $(LDFLAGS)

pgo-generate: CFLAGS := $(RELEASE_CFLAGS) $(CFLAGS)
pgo-generate: LDFLAGS := $(RELEASE_LDFLAGS) $(LDFLAGS)
pgo-generate: $(PGO_OBJ)
	$(CC) $^ $(CFLAGS) $(PROF_FLAGS) -o $@ $(LDFLAGS) $(PROF_FLAGS)

pgo-reset:
	find ./prof -name \*.gcda -exec rm {} +

pgo-profile: | pgo-reset pgo-generate
	#./profile/gen $(PROF_LARGE_BOUND) "$(PROF_LOCATION)/large"
	#./profile/gen $(PROF_SMALL_BOUND) "$(PROF_LOCATION)/small"
	for i in {1..$(PROF_ITERATIONS)}; do \
		rm -rf $(PROF_LOCATION); \
		mkdir -p $(PROF_LOCATION)/{large,small}; \
		printf "Iteration $$i of $(PROF_ITERATIONS)\r"; \
		./profile/gen $(PROF_LARGE_BOUND) "$(PROF_LOCATION)/large" >> /dev/null; \
		./profile/gen $(PROF_SMALL_BOUND) "$(PROF_LOCATION)/small" >> /dev/null; \
		./pgo-generate $(PROF_LOCATION)/large/matching/* & > $(PROF_LOCATION)/stdout; \
		./pgo-generate $(PROF_LOCATION)/large/unmatching/* & > $(PROF_LOCATION)/stdout; \
		./pgo-generate $(PROF_LOCATION)/small/matching/* & > $(PROF_LOCATION)/stdout; \
		./pgo-generate $(PROF_LOCATION)/small/unmatching/* & > $(PROF_LOCATION)/stdout; \
		\
		./pgo-generate $(PROF_LOCATION)/small/matching/{1,2} & > $(PROF_LOCATION)/stdout; \
		./pgo-generate $(PROF_LOCATION)/large/matching/{1,2} & > $(PROF_LOCATION)/stdout; \
		./pgo-generate $(PROF_LOCATION)/small/unmatching/{1,2} & > $(PROF_LOCATION)/stdout; \
		./pgo-generate $(PROF_LOCATION)/large/unmatching/{1,2} & > $(PROF_LOCATION)/stdout; \
		wait; \
		rm -rf $(PROF_LOCATION)/{large,small}; \
	done
	@echo ""
	rm -rf $(PROF_LOCATION)
	rm pgo-generate

pgo-use: CFLAGS := $(RELEASE_CFLAGS) $(CFLAGS)
pgo-use: LDFLAGS := $(RELEASE_LDFLAGS) $(LDFLAGS)
pgo-use: PROF_FLAGS = -fprofile-use -fprofile-correction
pgo-use: $(PGO_OBJ)
	$(CC) $^ $(CFLAGS) $(PROF_FLAGS) -o $@ $(LDFLAGS) $(PROF_FLAGS)

$(PROJECT)-pgo: CFLAGS := $(RELEASE_CFLAGS) $(CFLAGS)
$(PROJECT)-pgo: LDFLAGS := $(RELEASE_LDFLAGS) $(LDFLAGS)
$(PROJECT)-pgo: pgo-profile
	find ./prof -name \*.o -exec rm {} +
	$(MAKE) pgo-use
	mv pgo-use $@
	strip $@

clean:
	rm -rf {obj,prof}
	rm -f $(PROJECT)-{release,debug,pgo}
