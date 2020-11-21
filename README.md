# fcmp - compare file identity

A small shell util that compares files for identity with `mmap()` and `memcmp()`.

# Usage
`fcmp` produces no output, but success is indicated by its return code.

```shell
$ fcmp file1 file2 && echo "Equal!"
```

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

## Debug target
Build with debugging information and no optimisations using `make debug`, it will output a binary at `fcmp-debug`.

## Note
Before switching between `release` and `debug` targets, make sure to run `make clean`.

# License
GPL'd with <3
