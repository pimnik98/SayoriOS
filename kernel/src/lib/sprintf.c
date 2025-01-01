#include <stdarg.h>
#include "common.h"
#include "lib/math.h"
#include "mem/vmm.h"
#include "io/ports.h"
#include "drv/fpu.h"
#include "lib/stdlib.h"

size_t measure_vsprintf(const char *format, va_list args) {
	ON_NULLPTR(format, {
		qemu_log("Format is nullptr!");
		return 0;
	});

    char* fmt = (char*)format;
    size_t size = 0;

    while (*fmt) {
        if (*fmt == '%') {
            size_t width = 0;
            bool left_align = false;

            bool is_precision = false;

            fmt++;

            if(*fmt == '.') {
            	is_precision = true;
            	
            	fmt++;
            }

            if(*fmt == '-') {
                left_align = true;
                fmt++;
            }

            while(isdigit(*fmt)) {
                width = width * 10 + (*fmt - '0');
                fmt++;
            }

            if(*fmt == '*') {
                width = va_arg(args, int);
                fmt++;
            }

            switch (*fmt) {
                case 's': {
                    char* arg = va_arg(args, char*);
                    arg = arg ? arg : "(nullptr)";

                    if(is_precision) {
                        size += width;
                        break;
                    }

                    int length = strlen(arg);

                    int space = (int)width - length;

                    if(space > 0)
                        size += space;

                    size += length;

                    break;
                }
                case 'c': {
                    (void)va_arg(args, int);

                    size++;

                    break;
                }
                case 'f': {
                    double a = va_arg(args, double);

					if(!fpu_isInitialized()) {
						size += 7;
						break;
					}

					if(a < 0) {
						a = -a;
						size++;
					}

                    if(a == NAN || a == INFINITY) {
                        size += 3;
                        break;
                    }

                    size_t prec = is_precision ? width : 5;

					size += digit_count((int)a) + 1 + prec;  // xxxx.xxxx
                    
                    break;
                }
                case 'i':
                case 'd': {
                    int num = va_arg(args, int);

                    if(num < 0) {
                        num = -num;
                        size++;
                    }

                    int space = width - digit_count(num);

                    if(left_align)
                        size += digit_count(num);
                    
                    if(space > 0) {
                        size += space;
                    }

                    if(!left_align)
                        size += digit_count(num);
                    
                    break;
                }
                case 'u': {
                    unsigned int num = va_arg(args, unsigned int);
                    int space = width - digit_count((int)num);

                    if(space > 0) {
                        size += space;
                    }

                    size += digit_count(num);
                }
                case 'p':
                case 'x': {
                    int num = va_arg(args, int);
                    int space = width - hex_count(num) - 2;
                    
                    if(space > 0) {
                        size += space;
                    }

                    size += hex_count(num) + 2;
                    
                    break;
                }
                case 'v': {
                    int num = va_arg(args, int);
                    int space = width - hex_count(num);

                    if(left_align)
                        size += hex_count(num);
                    
                    if(space > 0) {
                        size += space;
                    }

                    if(!left_align)
                        size += hex_count(num);
                    
                    break;
                }
                default: {
                    size++;
                }
            }
        } else {
            size++;
        }

        fmt++;
    }

    return size + 1;
}

int vsprintf(char* buffer, const char *format, va_list args) {
    char* fmt = (char*)format;
    size_t size = measure_vsprintf(format, args);

    while (*fmt) {
        if (*fmt == '%') {
            // NOTE: If is_precision == true, width is a precision.

            size_t width = 0;
            bool left_align = false;
            bool is_precision = false;
            bool zero_padding = false;

            fmt++;

            if(*fmt == '.') {
            	is_precision = true;
            	fmt++;
            }

            if(*fmt == '-') {
                left_align = true;
                fmt++;
            }

            if(*fmt == '0') {
                zero_padding = true;
                fmt++;
            }

            while(isdigit(*fmt)) {
                width = width * 10 + (*fmt - '0');
                fmt++;
            }

            if(*fmt == '*') {
                width = va_arg(args, int);
                fmt++;
            }

            switch (*fmt) {
                case 's': {
                    char* arg = va_arg(args, char*);
                    arg = arg ? arg : "(nullptr)";

                    size_t length = strlen(arg);
                    int space = (int)width - (int)length;

                    if(!is_precision && left_align) {
                        memcpy(buffer, arg, length);

                        buffer += length;
                    }

					if(!is_precision) {
	                    if(space > 0) {
                            while(space--)
                                *buffer++ = ' ';
            	        }
                    }

                    if(!is_precision && !left_align) {
                        memcpy(buffer, arg, length);

                        buffer += length;
                    }

                    if(is_precision) {
                    	for(int i = 0; *arg != '\0' && i < width; i++) {
                    		*buffer++ = *arg++;
                    	}
                    }
                    
                    break;
                }

                case 'c': {
                    *buffer++ = (char)va_arg(args, int);  // Standard says that we need int instead of char
                    break;
                }

                case 'f': {
                	// FIXME: IMPLEMENT PRECISION HERE!

                    double a = va_arg(args, double);
					if(!fpu_isInitialized()) {
						memcpy(buffer, "0.00000", 7);

                        buffer += 7;
						break;
					}

					if(a < 0.0) {
						*buffer++ = '-';
						a = -a;
					}

//                    if(is_nan((float)a)) {
//						*buffer++ = 'n';
//						*buffer++ = 'a';
//						*buffer++ = 'n';
//
//                        break;
//                    }
//
//                    if(is_inf((float)a)) {
//                        *buffer++ = 'i';
//						*buffer++ = 'n';
//						*buffer++ = 'f';
//
//                        break;
//                    }

                    double intpart;
					double floatpart = modf(a, &intpart);

                    itoa((int)intpart, buffer);

					buffer += digit_count((int)intpart);
                    
                    *buffer++ = '.';

                    size_t prec = is_precision ? width : 5;

                    for(int i = 0; i < prec; i++) {
                        *buffer++ = '0' + (char)((int)(floatpart * pow(10.0, i + 1)) % 10);
                    }

                    break;
                }

                case 'i':
                case 'd': {
                    // FIXME: IMPLEMENT PRECISION HERE!
                    int num = va_arg(args, int);

                    if(num < 0) {
                        num = -num;
                        *buffer++ = '-';
                    }

                    int space = width - digit_count(num);

                    if(left_align) {
                        itoa(num, buffer);

                        buffer += digit_count(num);
                    }

                    if(space > 0) {
                        if(!zero_padding)
                            while(space--)
                                *buffer++ = ' ';
                        else
                            while(space--)
                                *buffer++ = '0';
                    }

                    if(!left_align) {
                        itoa(num, buffer);

                        buffer += digit_count(num);
                    }

                    break;
                }
				case 'u': {
					unsigned int num = va_arg(args, unsigned int);
					int space = width - digit_count((size_t)num);

					if(left_align) {
						itou(num, buffer);
						buffer += digit_count((size_t)num);
					}

					if(space > 0) {
						if(!zero_padding)
							while(space--)
								*buffer++ = ' ';
						else
							while(space--)
								*buffer++ = '0';
					}

					if(!left_align) {
						itou(num, buffer);
						buffer += digit_count((size_t)num);
					}

					break;
				}
                case 'p':
                case 'x': {
                	// TODO: IMPLEMENT PRECISION HERE!
                	
                    unsigned int num = va_arg(args, unsigned int);
                    int space = (int)width - hex_count(num) - 2;

                    if(left_align) {
                        *buffer++ = '0';
                        *buffer++ = 'x';
                        
                        itoh(num, buffer);
                        
                        buffer += hex_count(num);
                    }
                    
                    if(space > 0) {
                        if(!zero_padding)
                            while(space--)
                                *buffer++ = ' ';
                        else
                            while(space--)
                                *buffer++ = '0';
                    }

                    if(!left_align) {
                        *buffer++ = '0';
                        *buffer++ = 'x';
                        
                        itoh(num, buffer);
                        
                        buffer += hex_count(num);
                    }
                    
                    break;
                }
                case 'v': {
                	// FIXME: IMPLEMENT PRECISION HERE!

                    int num = va_arg(args, int);
                    int space = width - hex_count(num);

                    if(left_align) {
                        itoh(num, buffer);
                        
                        buffer += hex_count(num);
                    }
                    
                    if(space > 0) {
                        if(!zero_padding)
                            while(space--)
                                *buffer++ = ' ';
                        else
                            while(space--)
                                *buffer++ = '0';
                    }

                    if(!left_align) {
                        itoh(num, buffer);
                        
                        buffer += hex_count(num);
                    }
                    
                    break;
                }
            }
        } else {
            *buffer++ = *fmt;
        }

        fmt++;
    }

    return (int)size - 1;
}

size_t measure_sprintf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    
    size_t size = measure_vsprintf(format, args);

    va_end(args);

    return size;
}

int sprintf(char* buffer, const char* format, ...) {
    va_list args;
    va_start(args, format);
    
    size_t size = (int)(measure_vsprintf(format, args)) - 1;

    vsprintf(buffer, format, args);

    va_end(args);

    return size;
}

int vasprintf(char** buffer, const char* format, va_list args) {
    size_t size = measure_vsprintf(format, args);
    
    *buffer = kcalloc(size, 1);
    
    vsprintf(*buffer, format, args);

    return (int)size - 1;
}

int asprintf(char** buffer, const char* format, ...) {
    va_list args;
    va_start(args, format);
    
    vasprintf(buffer, format, args);

    size_t size = measure_vsprintf(format, args) - 1;

    va_end(args);

    return (int)size;
}

int vsnprintf(char* buffer, size_t n, const char* format, va_list args) {
	size_t size = measure_vsprintf(format, args);

	if(buffer == 0 || n == 0) {
	        return size;
	}

	char* temp = kcalloc(size + 1, 1);

	vsprintf(temp, format, args);

	memcpy(buffer, temp, (size > n ? n : size) - 1);

	kfree(temp);

	return size;
}

int snprintf(char* buffer, size_t n, const char* format, ...) {
    va_list args;
    va_start(args, format);

    size_t res = vsnprintf(buffer, n, format, args);

    va_end(args);

    return res;
}
