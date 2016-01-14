#include "common.h"

// Custom version of strcmp from string.h.  Same functionality and return values.
int32_t
q_strcmp(const char *str1, const char *str2) {
    while(*str1 == *str2) {
        if(*str1 == '\0') return 0;
        
        ++str1;
        ++str2;
    }
    
    return (*str1 < *str2) ? 1 : -1;
}

// Updated version of atoi from stdlib.h.  Can handle hexadecimal and integer strings.
int32_t
q_atoi(const char *numstr) {
    // Check for null string
    if(!numstr) return 0;
    
    int32_t sign = 1;
    int32_t num = 0;
    int32_t base = 10;
    
    while(*numstr) {
        // Check for signed/unsigned
        if(*numstr == '-') {
            sign = -1;
            
            // Go directly to next iteration of the loop.
            ++numstr;
            continue;
        }
        
        // Hexadecimal check.
        if(*numstr == '0' && (*(numstr + 1) == 'x' || *(numstr + 1) == 'X')) {
            // Can't have negative hexadecimals (eg. -0x64BCD) or muli-hex (eg 0x350xf)
            if(sign < 0 || base == 16) return 0;
            
            // Hex is base-16, decidmal is base-10.
            base = 16;
            
            // Skip over the '0x' and go to the next iteration.
            numstr += 2;
            continue;
        }
        
        if(*numstr >= '0' && *numstr <= '9') num = num * base + (*numstr - 48);
        if(*numstr >= 'a' && *numstr <= 'f') num = num * base + (*numstr - 97) + 10;
        if(*numstr >= 'A' && *numstr <= 'F') num = num * base + (*numstr - 65) + 10;
        
        ++numstr;
    }
    
    return num * sign;
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
com_check_parm(const char *search_arg, uint32_t num_args, const char *args[]) {
    // Loop through all available parameters.
    for(uint32_t i = 0; i < num_args; i++) {
        
        // See if any match the parameter we're searching for.
        if(q_strcmp(args[i], search_arg) == 0) {
            
            // If we found the parameter, get it's value
            const char *arg_val = args[++i];
            
            // if the parameter value is empty or the start of a new parameter, return 1, else return the value
            return (arg_val[0] == '\0' || arg_val[0] == '-') ? "1" : arg_val;
        }
    }
    // Parameter not found
    return NULL;
}