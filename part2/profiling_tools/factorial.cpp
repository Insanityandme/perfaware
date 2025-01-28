#include <stdio.h>
#include <stdint.h>

typedef uint8_t u8;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int32_t b32;

typedef float f32;
typedef double f64;

#include "listing_0085_recursive_profiler.cpp"

int factorial(int number)
{
    TimeFunction;
    if(number >=1)
        return number*factorial(number - 1);
    else
        return 1;
}

int main(int ArgCount, char **Args) 
{
    BeginProfile();
    int fact = factorial(5);
    EndAndPrintProfile();
    fprintf(stdout, "Factorial: %d", fact);
}
