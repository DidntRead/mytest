#include "tlStd.h"
#include "TlApi/TlApiHeap.h"
#include "TlApi/TlApiTime.h"
#include "tci.h"

uint32_t focal_mem_hint = 0;
static uint32_t first = 0;
static uint32_t highest = 0;
#define _MB (1024 * 1024)
#define _KB (1024)

void* malloc(unsigned int size) {
    int b, k, m, diff;
    void* p;

    p = tlApiMalloc((size_t)size, focal_mem_hint);
#ifdef DEBUG
    if (p == NULL) {
        focal_debug("malloc failed for %d", size);
    }

    if (first == 0) {
        first = (int32_t)p;
    }
    if (highest < ((uint32_t)p)) {
        highest = ((uint32_t)p);
        diff = highest - first; 
        m = diff / _MB;
        k = (diff % _MB) / _KB;
        b = diff % _KB;
        focal_debug("Call to malloc %d %d %d M %d K %D b", size, diff, m, k, b);
    }
#endif
    return p;
}

void free(void* mem) {
	tlApiFree(mem);
}

void* realloc(void* ptr, size_t size) {
    void* p;
    p = tlApiRealloc(ptr, size);
    if (p == NULL) {
        focal_debug("malloc failed in realloc");
    }
    return p;
}
#ifdef DEBUG
void focal_tee_debug(const char* format, ...) {
    va_list arg;
    va_start (arg, format);
    tlApiLogvPrintf(format, arg);
    va_end(arg);
}
#endif

/*
void* __aeabi_memcpy4(void *dest, const void *src, size_t count) {
    return memcpy(dest, src, count);
}
*/

void focal_get_timestamp(uint32_t* high, uint32_t* low)
{
    uint64_t time_stamp = 0;
    tlApiGetSecureTimestamp(&time_stamp);
    *high = (time_stamp >> 32);
    *low = (uint32_t)time_stamp;
}
