/* ========================================================================
   LISTING 110
   ======================================================================== */
static void WriteToAllBytes(repetition_tester *Tester, read_parameters *Params)
{
    while(IsTesting(Tester))
    {
        buffer DestBuffer = Params->Dest;
        HandleAllocation(Params, &DestBuffer);

        BeginTime(Tester);
        for(u64 Index = 0; Index < DestBuffer.Count; ++Index)
        {
            DestBuffer.Data[Index] = (u8)Index;
        }
        EndTime(Tester);

        CountBytes(Tester, DestBuffer.Count);

        HandleDeallocation(Params, &DestBuffer);
    }
}
