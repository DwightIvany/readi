// golden_test.cpp - console test for the di75 DSP port (no JUCE).
//
// Usage: golden_test <cases.txt> <data-dir>
//
// Each non-comment line of cases.txt is:
//   name hp_hz thresh_db ratio gain_db attack_us release_ms mono input_file ref_file
// Input/reference files are interleaved little-endian float32 stereo raw.
// Prints "name maxdiff=<value>" per case; exits 1 if any case exceeds 1e-4
// or a file is unreadable, otherwise prints "golden test OK".

#include "Di75Dsp.h"

#include <algorithm>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace
{
bool readRawStereo(const std::string& path, std::vector<float>& samples)
{
    std::FILE* f = std::fopen(path.c_str(), "rb");
    if (f == nullptr)
        return false;

    std::fseek(f, 0, SEEK_END);
    const long bytes = std::ftell(f);
    std::fseek(f, 0, SEEK_SET);

    if (bytes < 0 || (bytes % (long)(2 * sizeof(float))) != 0)
    {
        std::fclose(f);
        return false;
    }

    samples.resize((size_t)bytes / sizeof(float));
    const size_t got = samples.empty() ? 0 : std::fread(samples.data(), 1, (size_t)bytes, f);
    std::fclose(f);
    return got == (size_t)bytes;
}
} // namespace

int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        std::fprintf(stderr, "usage: %s <cases.txt> <data-dir>\n", argv[0]);
        return 1;
    }

    std::ifstream cases(argv[1]);
    if (!cases)
    {
        std::fprintf(stderr, "cannot open cases file: %s\n", argv[1]);
        return 1;
    }

    const std::string dir = argv[2];
    bool ok = true;
    std::string line;

    while (std::getline(cases, line))
    {
        if (line.empty() || line[0] == '#')
            continue;

        std::istringstream ss(line);
        std::string name, inFile, refFile;
        double hp, thresh, ratio, gain, atk, rel;
        int mono;

        if (!(ss >> name >> hp >> thresh >> ratio >> gain >> atk >> rel >> mono >> inFile >> refFile))
        {
            std::fprintf(stderr, "bad case line: %s\n", line.c_str());
            ok = false;
            continue;
        }

        std::vector<float> in, ref;
        if (!readRawStereo(dir + "/" + inFile, in) || !readRawStereo(dir + "/" + refFile, ref))
        {
            std::printf("%s maxdiff=unreadable\n", name.c_str());
            ok = false;
            continue;
        }

        if (in.size() != ref.size() || in.size() < 2)
        {
            std::printf("%s maxdiff=size-mismatch\n", name.c_str());
            ok = false;
            continue;
        }

        const int frames = (int)(in.size() / 2);

        Di75Dsp dsp;
        dsp.setSampleRate(48000.0);
        dsp.setParams(hp, thresh, ratio, gain, atk, rel, mono != 0);
        dsp.process(in.data(), frames);

        double maxdiff = 0.0;
        for (size_t i = 0; i < in.size(); ++i)
            maxdiff = std::max(maxdiff, (double)std::fabs(in[i] - ref[i]));

        std::printf("%s maxdiff=%.6g\n", name.c_str(), maxdiff);
        if (maxdiff > 1e-4)
            ok = false;
    }

    if (!ok)
    {
        std::printf("golden test FAILED\n");
        return 1;
    }

    std::printf("golden test OK\n");
    return 0;
}
