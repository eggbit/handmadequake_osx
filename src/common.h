#ifndef common_h
#define common_h

#include <stdint.h>
#include <stdio.h>

// stdlib replacements
int32_t q_strcmp(const char *str1, const char *str2);
int32_t q_atoi(const char *numstr);
size_t q_strlen(const char *str);
void q_strcpy(char *dest, const char *src);
void q_strncpy(char *dest, const char *src, int32_t count);

// common functions
const char *com_check_parm(const char *search_arg, uint32_t num_args, const char *args[]);

#endif /* common_h */
