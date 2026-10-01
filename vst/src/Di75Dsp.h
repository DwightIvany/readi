#pragma once

// Pure C++17 DSP core for the di75 compressor. No JUCE includes so it can be
// compiled standalone by the golden test. Math must match vst/scripts/make_ref.py
// (which models jsfx/di75.jsfx); see Di75Dsp.cpp for lineage.

class Di75Dsp
{
public:
    void setSampleRate(double sr);
    void reset(); // zero all state

    // hp 0..400 (0 = bypass filter), threshDb -60..0, ratio 4..20,
    // gainDb -20..20, attackUs 20..2000, releaseMs 20..1000, mono bool.
    // Recomputes coefficients only when any value changed since last call.
    void setParams(double hp, double threshDb, double ratio,
                   double gainDb, double attackUs, double releaseMs, bool mono);

    // interleaved stereo frames in-place
    void process(float* leftRight, int numFrames);

    // last gain-reduction factor (1.0 = none), for the meter
    double currentGrv() const { return grv; }

private:
    void updateCoeffs();

    double sampleRate = 48000.0;

    double hp = 0.0, threshDb = 0.0, ratio = 4.0, gainDb = 0.0;
    double attackUs = 20.0, releaseMs = 250.0;
    bool mono = false;
    bool coeffsDirty = true;

    // coefficients
    double hpb1 = 0.0, hpa0 = 0.0;
    double threshv = 1.0, makeupv = 1.0, atcoef = 0.0, relcoef = 0.0;

    // state
    double hp0 = 0.0, hp1 = 0.0, old0 = 0.0, old1 = 0.0, rundb = 0.0;
    double grv = 1.0;
};
