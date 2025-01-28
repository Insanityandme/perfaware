/* NOTE(casey): _CRT_SECURE_NO_WARNINGS is here because otherwise we cannot
   call fopen(). If we replace fopen() with fopen_s() to avoid the warning,
   then the code doesn't compile on Linux anymore, since fopen_s() does not
   exist there.
   
   What exactly the CRT maintainers were thinking when they made this choice,
   I have no idea. */
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <sys/stat.h>

typedef uint8_t u8;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int32_t b32;

typedef float f32;
typedef double f64;

#define ArrayCount(Array) (sizeof(Array)/sizeof((Array)[0]))

struct haversine_pair
{
    f64 X0, Y0;
    f64 X1, Y1;
};

#define PROFILER 1
#include "listing_0068_buffer.cpp"
#include "listing_0100_bandwidth_profiler.cpp"
#include "listing_0065_haversine_formula.cpp"

#include "listing_0094_profiled_lookup_json_parser.cpp"
// #include "lookup_json_parser.cpp"

static buffer ReadEntireFile(char *FileName)
{
    TimeFunction;

    buffer Result = {};
        
    FILE *File = fopen(FileName, "rb");
    if(File)
    {
#if _WIN32
        struct __stat64 Stat;
        _stat64(FileName, &Stat);
#else
        struct stat Stat;
        stat(FileName, &Stat);
#endif
        
        Result = AllocateBuffer(Stat.st_size);
        if(Result.Data)
        {
            // TimeBandwidth("fread", Result.Count);
            if(fread(Result.Data, Result.Count, 1, File) != 1)
            {
                fprintf(stderr, "ERROR: Unable to read \"%s\".\n", FileName);
                FreeBuffer(&Result);
            }
        }
        
        fclose(File);
    }
    else
    {
        fprintf(stderr, "ERROR: Unable to open \"%s\".\n", FileName);
    }
    
    return Result;
}

static f64 QuadScalarSumHaversineDistances(u64 PairCount, haversine_pair *Pairs)
{
    TimeBandwidth(__func__, PairCount*sizeof(haversine_pair));

    f64 SumA = 0;
    f64 SumB = 0;
    f64 SumC = 0;
    f64 SumD = 0;
    
    f64 SumCoef = 1 / (f64)PairCount;
    f64 EarthRadius = 6372.8;

    for(u64 PairIndex = 0; PairIndex < PairCount; PairIndex += 4)
    {
        haversine_pair PairA = Pairs[PairIndex + 0];
        haversine_pair PairB = Pairs[PairIndex + 1];
        haversine_pair PairC = Pairs[PairIndex + 2];
        haversine_pair PairD = Pairs[PairIndex + 3];

        f64 DistA = ReferenceHaversine(PairA.X0, PairA.Y0, PairA.X1, PairA.Y1, EarthRadius);
        f64 DistB = ReferenceHaversine(PairB.X0, PairB.Y0, PairB.X1, PairB.Y1, EarthRadius);
        f64 DistC = ReferenceHaversine(PairC.X0, PairC.Y0, PairC.X1, PairC.Y1, EarthRadius);
        f64 DistD = ReferenceHaversine(PairD.X0, PairD.Y0, PairD.X1, PairD.Y1, EarthRadius);

        SumA += SumCoef*DistA;
        SumB += SumCoef*DistB;
        SumC += SumCoef*DistC;
        SumD += SumCoef*DistD;
    }
    
    f64 Sum = SumA + SumB + SumC + SumD;
    
    return Sum;
}

int main(int ArgCount, char **Args)
{
    BeginProfile();

    int Result = 1;
    
    if((ArgCount == 2) || (ArgCount == 3))
    {
        buffer InputJSON = ReadEntireFile(Args[1]);
        
        u32 MinimumJSONPairEncoding = 6*4;
        u64 MaxPairCount = InputJSON.Count / MinimumJSONPairEncoding;
        if(MaxPairCount)
        {
            buffer ParsedValues = AllocateBuffer(MaxPairCount * sizeof(haversine_pair));
            if(ParsedValues.Count)
            {
                haversine_pair *Pairs = (haversine_pair *)ParsedValues.Data;

                u64 PairCount = ParseHaversinePairs(InputJSON, MaxPairCount, Pairs);

                f64 Sum = QuadScalarSumHaversineDistances(PairCount, Pairs);
                
                Result = 0;

                fprintf(stdout, "Input size: %llu\n", InputJSON.Count);
                fprintf(stdout, "Pair count: %llu\n", PairCount);
                fprintf(stdout, "Haversine sum: %.16f\n", Sum);
                
                if(ArgCount == 3)
                {
                    buffer AnswersF64 = ReadEntireFile(Args[2]);
                    if(AnswersF64.Count >= sizeof(f64))
                    {
                        f64 *AnswerValues = (f64 *)AnswersF64.Data;
                        
                        fprintf(stdout, "\nValidation:\n");
                        
                        u64 RefAnswerCount = (AnswersF64.Count - sizeof(f64)) / sizeof(f64);
                        if(PairCount != RefAnswerCount)
                        {
                            fprintf(stdout, "FAILED - pair count doesn't match %llu.\n", RefAnswerCount);
                        }
                        
                        f64 RefSum = AnswerValues[RefAnswerCount];
                        fprintf(stdout, "Reference sum: %.16f\n", RefSum);
                        fprintf(stdout, "Difference: %.16f\n", Sum - RefSum);
                        
                        fprintf(stdout, "\n");
                    }
                }
            }
            
            FreeBuffer(&ParsedValues);
        }
        else
        {
            fprintf(stderr, "ERROR: Malformed input JSON\n");
        }

        FreeBuffer(&InputJSON);
    }
    else
    {
        fprintf(stderr, "Usage: %s [haversine_input.json]\n", Args[0]);
        fprintf(stderr, "       %s [haversine_input.json] [answers.f64]\n", Args[0]);
    }

    if(Result == 0)
    {
        EndAndPrintProfile();
    }
    
    return Result;
}

ProfilerEndOfCompilationUnit;
