
#include <common.h>

#define PATHINFO_DIRNAME 0
#define PATHINFO_BASENAME 1
#define PATHINFO_EXTENSION 2
#define PATHINFO_FILENAME 3

char* pathinfo(const char* Path, int Mode);