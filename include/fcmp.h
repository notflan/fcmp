#ifndef _FCMP_H
#define _FCMP_H

#ifdef DEBUG
#define _FORCE_INLINE static inline __attribute__((gnu_inline))
#else
#define _FORCE_INLINE extern inline __attribute__((gnu_inline))
#endif

#define _ALIAS __attribute__((may_alias))

#ifdef DEBUG
#define __name(d) #d
#define dprintf(fmt, ...) printf("[dbg @" __FILE__ "->%s:%d] " fmt "\n", __func__, __LINE__, ## __VA_ARGS__)
#else
#define dprintf(fmt, ...)
#endif

/// Enabled threaded scheduling
// Set to 1 to FORCE threaded scheduling, 0 to use when opportune.
//
//#define _RUN_THREADED 0


extern const char* _prog_name;

#endif /* _FCMP_H */
