# fcmp - compare file identity

A shell utils that compares files for identity with `mmap()` and `memcmp()`.

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

Therefor you can use this in shell scripts easily:

``` shell
# Example

if fcmp "$1" "$2"; then
    echo "Files are equal!"
else
    echo "Files are not equal!"
fi
```

# License
GPL'd with <3
