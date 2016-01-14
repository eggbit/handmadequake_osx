#ifndef common_h
#define common_h

#include <stdint.h>

int32_t q_strcmp(const char *str1, const char *str2);
int32_t q_atoi(const char *numstr);
const char *com_check_parm(const char *search_arg, uint32_t num_args, const char *args[]);

#endif /* common_h */
