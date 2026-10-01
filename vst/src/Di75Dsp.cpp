// DSP derived from Stillwell 1175 (BSD license, copyright retained below).
// Original source: https://github.com/stillwellaudio/jsfx/blob/master/1175
// Ported to C++ for the di75 VST3; math mirrors vst/scripts/make_ref.py
// (float64 model of jsfx/di75.jsfx @sample + jsfx/dipass.jsfx).

// Copyright 2006, Thomas Scott Stillwell
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted
// provided that the following conditions are met:
//
// Redistributions of source code must retain the above copyright notice, this list of conditions
// and the following disclaimer.
//
// Redistributions in binary form must reproduce the above copyright notice, this list of conditions
// and the following disclaimer in the documentation and/or other materials provided with the distribution.
//
// The name of Thomas Scott Stillwell may not be used to endorse or
// promote products derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
// FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
// BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
// STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
// THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "Di75Dsp.h"

#include <algorithm>
#include <cmath>

namespace
{
constexpr double pi = 3.14159265358979323846;
constexpr double log2db = 8.6858896380650365530225783783321;  // 20 / ln(10)
constexpr double db2log = 0.11512925464970228420089957273422; // ln(10) / 20
} // namespace

void Di75Dsp::setSampleRate(double sr)
{
    if (sr != sampleRate)
    {
        sampleRate = sr;
        coeffsDirty = true;
    }
}

void Di75Dsp::reset()
{
    hp0 = hp1 = old0 = old1 = rundb = 0.0;
    grv = 1.0;
}

void Di75Dsp::setParams(double hpHz, double thresh, double ratioValue,
                        double gainDbValue, double attackMicroseconds,
                        double releaseMilliseconds, bool monoValue)
{
    if (hpHz == hp && thresh == threshDb && ratioValue == ratio
        && gainDbValue == gainDb && attackMicroseconds == attackUs
        && releaseMilliseconds == releaseMs && monoValue == mono)
        return;

    hp = hpHz;
    threshDb = thresh;
    ratio = ratioValue;
    gainDb = gainDbValue;
    attackUs = attackMicroseconds;
    releaseMs = releaseMilliseconds;
    mono = monoValue;
    coeffsDirty = true;
}

void Di75Dsp::updateCoeffs()
{
    coeffsDirty = false;

    if (hp > 0.0)
    {
        hpb1 = std::exp(-4.0 * pi * hp / sampleRate);
        hpa0 = (hpb1 + 1.0) * 0.5;
    }
    else
    {
        hpb1 = 0.0;
        hpa0 = 0.0;
    }

    threshv = std::exp(threshDb * db2log);
    makeupv = std::exp(gainDb * db2log);
    atcoef = std::exp(-1.0 / ((attackUs / 1000000.0) * sampleRate));
    relcoef = std::exp(-1.0 / ((releaseMs / 1000.0) * sampleRate));
}

void Di75Dsp::process(float* leftRight, int numFrames)
{
    if (coeffsDirty)
        updateCoeffs();

    const bool hpActive = hp > 0.0;

    for (int i = 0; i < numFrames; ++i)
    {
        double l = leftRight[2 * i];
        double r = leftRight[2 * i + 1];

        if (hpActive)
        {
            hp0 = (l - old0) * hpa0 + hp0 * hpb1;
            hp1 = (r - old1) * hpa0 + hp1 * hpb1;
            old0 = l;
            old1 = r;
            l = hp0;
            r = hp1;
        }

        const double det = std::max(std::abs(l), std::abs(r));
        const double overdb = det > 0.0
                                  ? std::max(0.0, log2db * std::log(det / threshv))
                                  : 0.0;

        const double coef = (overdb > rundb) ? atcoef : relcoef;
        rundb = overdb + coef * (rundb - overdb);

        grv = std::exp(-rundb * (ratio - 1.0) / ratio * db2log);

        const double gainReduction = grv * makeupv;
        l *= gainReduction;
        r *= gainReduction;

        if (mono)
        {
            const double m = (l + r) * 0.5;
            l = m;
            r = m;
        }

        leftRight[2 * i] = static_cast<float>(l);
        leftRight[2 * i + 1] = static_cast<float>(r);
    }
}
