/* ========================================================================

   (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

/* ========================================================================
   LISTING 76
   ======================================================================== */
#include "listing_0074_platform_metrics.cpp"
#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

struct profile_anchor
{
    const char *Label;
    u64 HitCount;
    u64 TSCElapsed;
};

struct profiler 
{
    profile_anchor Anchors[4096];

    u64 StartTSC;
    u64 EndTSC;
};
// NOTE(filip): This is used so that we do not have to initialize anything. 
static profiler GlobalProfiler;

struct profile_block
{
    profile_block(char const *Label_, u32 AnchorIndex_)
    {
        AnchorIndex = AnchorIndex_;
        Label = Label_;
        StartTSC = ReadCPUTimer();
    }

    ~profile_block(void)
    {
        u64 Elapsed = ReadCPUTimer() - StartTSC; 

        profile_anchor *Anchor = GlobalProfiler.Anchors + AnchorIndex;
        Anchor->TSCElapsed += Elapsed;
        ++Anchor->HitCount;

        Anchor->Label = Label;
    }

    char const *Label;
    u64 StartTSC;
    u32 AnchorIndex;
};

#define NameConcat2(A, B) A##B
#define NameConcat(A, B) NameConcat2(A, B)
#define TimeBlock(Name) profile_block NameConcat(Block, __LINE__)(Name, __COUNTER__ + 1);
#define TimeFunction TimeBlock(__func__)

static void PrintTimeElapsed(u64 TotalTSCElapsed, profile_anchor *Anchor)
{
    u64 Elapsed = Anchor->TSCElapsed;
    f64 Percent = 100.0 * ((f64)Elapsed / (f64)TotalTSCElapsed);
    printf("  %s[%llu]: %llu (%.2f%%)\n", Anchor->Label, Anchor->HitCount, Elapsed, Percent);
}

static void BeginProfile(void)
{
    GlobalProfiler.StartTSC = ReadCPUTimer();
}

static void EndAndPrintProfile()
{
    GlobalProfiler.EndTSC = ReadCPUTimer();
    u64 CPUFreq = EstimateCPUTimerFreq();

    u64 TotalCPUElapsed = GlobalProfiler.EndTSC - GlobalProfiler.StartTSC;

    if(CPUFreq)
    {
        printf("\nTotal time: %0.4fms (CPU freq %llu)\n", 1000.0 * (f64)TotalCPUElapsed / (f64)CPUFreq, CPUFreq);
    }

    for(u32 AnchorIndex = 0; AnchorIndex < ArrayCount(GlobalProfiler.Anchors); ++AnchorIndex)
    {
        profile_anchor *Anchor = GlobalProfiler.Anchors + AnchorIndex;
        if(Anchor->TSCElapsed)
        {
            PrintTimeElapsed(TotalCPUElapsed, Anchor);
        }
    }
}
