#include <kernel.h>

/// @brief Runs function and measures function execution time
/// @param function Pointer to function to test
/// @return Milliseconds elapsed during function execution.
size_t measure_func(void* (function())) {
    if(!function)
        return 0;

    size_t start = getTicks();
    
    function();

    size_t end = getTicks();

    return (end - start) / (getFrequency() / 1000);
}