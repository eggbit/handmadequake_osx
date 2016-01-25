#ifndef common_h
#define common_h

#include <stdint.h>
#include <stdio.h>

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

// stdlib replacements
i32 q_strcmp(const char *str1, const char *str2);
i32 q_atoi(const char *numstr);
size_t q_strlen(const char *str);
void q_strcpy(char *dest, const char *src);
void q_strncpy(char *dest, const char *src, i32 count);

// common functions
const char *com_check_parm(const char *search_arg, u32 num_args, const char *args[]);

#endif /* common_h */
