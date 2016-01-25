#include "common.h"

// Custom version of strcmp from string.h.  Same functionality and return values.
i32
q_strcmp(const char *str1, const char *str2) {
    if(!str1 || !str2) return -2;

    while(*str1 == *str2) {
        if(*str1 == '\0') return 0;

        ++str1;
        ++str2;
    }

    return (*str1 < *str2) ? 1 : -1;
}

// Updated version of atoi from stdlib.h.  Can handle hexadecimal and integer strings.
i32
q_atoi(const char *numstr) {
    // Check for null string
    if(!numstr) return 0;

    i32 sign = 1;
    i32 num = 0;
    i32 base = 10;

    while(*numstr) {
        // Check for signed/unsigned
        if(*numstr == '-')
            sign = -1;
        // Hexadecimal check.
        else if(*numstr == '0' && (*(numstr + 1) == 'x' || *(numstr + 1) == 'X')) {
            // Can't have negative hexadecimals (eg. -0x64BCD) or muli-hex (eg 0x350xf)
            if(sign < 0 || base == 16) return 0;

            // Hex is base-16, decidmal is base-10.
            base = 16;

            // Skip over the '0x'.
            ++numstr;
        }
        else if(*numstr >= '0' && *numstr <= '9')
            num = num * base + (*numstr - 48);
        else if(*numstr >= 'a' && *numstr <= 'f')
            num = num * base + (*numstr - 97) + 10;
        else if(*numstr >= 'A' && *numstr <= 'F')
            num = num * base + (*numstr - 65) + 10;
        else
            return num * sign;

        ++numstr;
    }

    return num * sign;
}

// Get the lengthof a given string.
size_t
q_strlen(const char *str) {
    if(!str) return 0;

    int i = 0;

    while(*str) {
        i++;
        str++;
    }

    return i;
}

// Custom versions of strcpy and strncpy from string.h.
void
q_strcpy(char *dest, const char *src) {
    if(!src) return;

    while(*src) {
        *dest++ = *src++;
    }
}

void
q_strncpy(char *dest, const char *src, i32 count) {
    if(!src || count < 0) return;

    while(*src && count) {
        *dest++ = *src++;
        --count;
    }

    while(*dest && count) {
        *dest++ = 0;
        --count;
    }
}

// Check if the command line argument 'search_arg' exists.  If it does, return the associated value.
// If it doesn't exist, return null.
//
//      eg. quake -setalpha 50 -startwindowed
//
//      const char *alpha_val = com_check_parm("-setalpha", argc, argv);
//      const char *windowed_val = com_check_parm("-startwindowed", argc, argv);
//
//      /* alpha_val will contain '50' */
//      /* windowed_val will contain '1' for true */
const char *
com_check_parm(const char *search_arg, u32 num_args, const char *args[]) {
    char *last_arg = "";

    // Loop backwards num_args amount of times.
    for(u32 i = num_args; i--;) {

        // See if any arguments match the one we're searching for.
        if(q_strcmp(args[i], search_arg) == 0) {

            // If last_arg isn't empty or the start of a new argument, return it.
            // Otherwise we have a boolean flag so just return a "1".
            return (*last_arg && *last_arg != '-') ? last_arg : "1";
        }

        // Save the parameter we just looked at for the next iteration of the loop;
        last_arg = (char *)args[i];
    }
    // Parameter not found
    return NULL;
}
