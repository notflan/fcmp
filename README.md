# fcmp - compare file identity

A small shell util that compares files for identity with `mmap()` and `memcmp()`.

# Usage
`fcmp` produces no output, but success is indicated by its return code.

```shell
$ fcmp file1 file2 && echo "Equal!"
```

You can pass any number of files, but at least 2 must be provided.

| Code | Meaning                                                      |
|------|--------------------------------------------------------------|
| 0    | The files are equal                                          |
| 1    | The files are unequal                                        |
| 2    | The files have unequal lengths, and therefor must be unequal |
| -1   | There was an error                                           |

You can use this in shell scripts easily:

``` shell
# Example

if fcmp "$1" "$2"; then
    echo "Files are equal!"
else
    echo "Files are not equal!"
fi
```

# Building
To build normally, run `make`.

## Release target 
Build with default optimisations using `make release`, it will output a stripped binary at `fcmp-release`.

### Notes
* The Makefile uses variables `RELEASE_CFLAGS` and `RELEASE_LDFLAGS` to apply optimisations (and `DEBUG_CFLAGS` + `DEBUG_LDFLAGS` for extra compiler flags with the debug target). If needed you can set these yourself to prevent the defaults.
* The default `RELEASE_CFLAGS` specify `-march=native` which may be undesireable for you. Set the variable or modify the Makefile if you need to remove this.

## PGO
Building with Profile Guided Optimisation is supported with the `pgo` Makefile target. It uses the same rules as the `release` target and outputs a binary to `fcmp-pgo`.

There may be small performance improvements from using this target instead of `release`, but the difference is mostly negligable.

## Debug target
Build with debugging information and no optimisations using `make debug`, it will output a binary at `fcmp-debug`.

## Notes
- Before switching between targets, make sure to run `make clean`.
- GCC + Graphite compiler specific optimisation flags are added by default with the `OPT_FLAGS` variable. Override this variable if using another compiler that doesn't support these optimisations.

### Multithreading
- By default, parallel processing is enabled when building through `libpthread`, to build a single-threaded version override the variables `FEAT_CFLAGS` and `FEAT_LDFLAGS` to empty.
- By default the program will decide at runtime whether or not to use parallelised processing. You can set `FEAT_CFLAGS="-D_RUN_THREADED=1"` to _force_ the use of a parallelised run every time in the binary, although this is not recommended.
- Performance gains from parallelised runs mostly appear with a large number of files being compared at once, as the task delegation overhead is surpassed.

# License
GPL'd with <3
