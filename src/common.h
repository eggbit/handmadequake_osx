#ifndef common_h
#define common_h

#include "quakedef.h"

// stdlib replacements
i32 q_strcmp(const char *str1, const char *str2);
i32 q_atoi(const char *numstr);
size_t q_strlen(const char *str);
void q_strcpy(char *dest, const char *src);
void q_strncpy(char *dest, const char *src, i32 count);

// common functions
const char *com_check_parm(const char *search_arg, u32 num_args, const char *args[]);

#endif /* common_h */
