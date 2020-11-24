
PROJECT = fcmprs

CARGO_FEATURES?= threads

OPT_FLAGS?= -C target-cpu=native
RUSTFLAGS?=

PROF_DIR=/tmp/fcmprs/prof
PROF_ITERATIONS?=100
PROF_LOCATION?=/tmp/fcmprs-profiters

PROF_LARGE_BOUND= $$(( 1024 * 1024 * 10 ))
PROF_SMALL_BOUND= $$(( 1024 * 10 ))

.PHONY: release
release: target/release/$(PROJECT)

.PHONY: debug
debug: target/debug/$(PROJECT)

.PHONY: pgo
pgo: target/release/$(PROJECT)-pgo
	# Currently broken.


target/release/$(PROJECT): RUSTFLAGS+= $(OPT_FLAGS)
target/release/$(PROJECT):
	RUSTFLAGS="$(RUSTFLAGS)" cargo build --release --no-default-features $(addprefix --features ,$(CARGO_FEATURES))
	strip $@

target/debug/$(PROJECT):
	RUSTFLAGS="$(RUSTFLAGS)" cargo build --no-default-features $(addprefix --features ,$(CARGO_FEATURES))

pgo-generate: RUSTFLAGS+= -Cprofile-generate=$(PROF_DIR)
pgo-generate: target/release/$(PROJECT)

pgo-profile: pgo-generate
	for i in {1..$(PROF_ITERATIONS)}; do \
		rm -rf $(PROF_LOCATION); \
		mkdir -p $(PROF_LOCATION)/{large,small}; \
		printf "Iteration $$i of $(PROF_ITERATIONS)\r"; \
		../profile/gen $(PROF_LARGE_BOUND) "$(PROF_LOCATION)/large" >> /dev/null; \
		../profile/gen $(PROF_SMALL_BOUND) "$(PROF_LOCATION)/small" >> /dev/null; \
		./target/release/fcmprs $(PROF_LOCATION)/large/matching/* & > $(PROF_LOCATION)/stdout; \
		./target/release/fcmprs $(PROF_LOCATION)/large/unmatching/* & > $(PROF_LOCATION)/stdout; \
		./target/release/fcmprs $(PROF_LOCATION)/small/matching/* & > $(PROF_LOCATION)/stdout; \
		./target/release/fcmprs $(PROF_LOCATION)/small/unmatching/* & > $(PROF_LOCATION)/stdout; \
		\
		./target/release/fcmprs $(PROF_LOCATION)/small/matching/{1,2} & > $(PROF_LOCATION)/stdout; \
		./target/release/fcmprs $(PROF_LOCATION)/large/matching/{1,2} & > $(PROF_LOCATION)/stdout; \
		./target/release/fcmprs $(PROF_LOCATION)/small/unmatching/{1,2} & > $(PROF_LOCATION)/stdout; \
		./target/release/fcmprs $(PROF_LOCATION)/large/unmatching/{1,2} & > $(PROF_LOCATION)/stdout; \
		wait; \
		rm -rf $(PROF_LOCATION)/{large,small}; \
	done
	@echo ""
	rm -rf $(PROF_LOCATION)
	llvm-profdata merge -o $(PROF_DIR)/merged.profdata $(PROF_DIR)

pgo-use: RUSTFLAGS+= -Cprofile-use=$(PROF_DIR)/merged.profdata -Cllvm-args=-pgo-warn-missing-function
pgo-use: target/release/$(PROJECT)

pgo-reset:
	rm -rf $(PROF_DIR)
	mkdir -p $(PROF_DIR)


target/release/$(PROJECT)-pgo: | pgo-reset pgo-profile
	#rm -rf target
	CARGO_INCREMENTAL=0 $(MAKE) -B pgo-use
	mv -f target/release/$(PROJECT) $@

clean:
	rm -rf target
	rm -rf $(PROF_LOCATION) $(PROF_DIR)
