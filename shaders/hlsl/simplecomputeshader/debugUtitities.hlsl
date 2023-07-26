
/*
    float2 vFontSize = 2.*float2(8.0, 10.0); // Multiples of 4x5 work best
    if (PrintValue((float2(tid.x, dim.y-tid.y) - float2(100, 5)) / vFontSize, pc.JetForceOrigin.x, 6.0, 2.0) > 0.1
      ||PrintValue((float2(tid.x, dim.y-tid.y) - float2(200, 5)) / vFontSize, pc.JetForceOrigin.y, 6.0, 2.0) > 0.1)
    {
      F_out[tid] = float2(1, 0);
      return;
    }
*/
float PRIVATE_DigitBin(const int x)
{
    return x == 0 ? 480599.0 :
        x == 1 ? 139810.0 :
        x == 2 ? 476951.0 :
        x == 3 ? 476999.0 :
        x == 4 ? 350020.0 :
        x == 5 ? 464711.0 :
        x == 6 ? 464727.0 :
        x == 7 ? 476228.0 :
        x == 8 ? 481111.0 :
        x == 9 ? 481095.0 : 0.0;
}

float PrintValue(float2 screenCoords, float fValue, float fMaxDigits, float fDecimalPlaces)
{
    if ((screenCoords.y < 0.0) || (screenCoords.y >= 1.0))
    {
        return 0.0;
    }
    bool bNeg = (fValue < 0.0);
    fValue = abs(fValue);
    float fLog10Value = log2(abs(fValue)) / log2(10.0);
    float fBiggestIndex = max(floor(fLog10Value), 0.0);
    float fDigitIndex = fMaxDigits - floor(screenCoords.x);
    float fCharBin = 0.0;
    if (fDigitIndex > (-fDecimalPlaces - 1.01))
    {
        if (fDigitIndex > fBiggestIndex)
        {
            if ((bNeg) && (fDigitIndex < (fBiggestIndex + 1.5)))
            {
                fCharBin = 1792.0;
            }
        }
        else
        {
            if (fDigitIndex == -1.0)
            {
                if (fDecimalPlaces > 0.0)
                {
                    fCharBin = 2.0;
                }
            }
            else
            {
                float fReducedRangeValue = fValue;
                if (fDigitIndex < 0.0)
                {
                    fReducedRangeValue = frac(fValue);
                    fDigitIndex += 1.0;
                }
                float fDigitValue = (abs(fReducedRangeValue / (pow(10.0, fDigitIndex))));
                fCharBin = PRIVATE_DigitBin(int(floor(fmod(fDigitValue, 10.0))));
            }
        }
    }
    return floor(fmod((fCharBin / pow(2.0, floor(frac(screenCoords.x) * 4.0) + (floor(screenCoords.y * 5.0) * 4.0))), 2.0));
}