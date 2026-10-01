# make_ref.py - golden reference generator for the di75 VST3 DSP port.
#
# Models jsfx/di75.jsfx (@sample block, lines 291-326) + jsfx/dipass.jsfx
# exactly, in float64, and writes raw float32 stereo input/reference files
# plus cases.txt for the C++ golden test (tests/golden_test.cpp).
#
# Usage:  python make_ref.py            (writes into tests/data next to this script)
#         python make_ref.py --selftest
#
# DSP equations (must match src/Di75Dsp.cpp):
#   HPF (only when hp > 0):  b1 = exp(-4*pi*f/sr); a0 = (b1+1)/2
#       hp = (x - old)*a0 + hp*b1; old = x; x = hp
#   det   = max(|L|, |R|)
#   overdb = max(0, log2db * ln(det / threshv))     threshv = exp(thresh*db2log)
#   env:  rundb = overdb + (overdb > rundb ? atcoef : relcoef) * (rundb - overdb)
#   grv   = exp(-rundb * (ratio-1)/ratio * db2log)
#   L,R  *= grv * makeupv                            makeupv = exp(gain*db2log)
#   mono: L = R = (L+R)*0.5
import math
import os
import struct
import sys

LOG2DB = 20.0 / math.log(10.0)
DB2LOG = math.log(10.0) / 20.0
SR = 48000.0
N = 48000  # 1 second


def process(inp, sr, hp, thresh, ratio, gain, atk_us, rel_ms, mono):
    # coefficients (jsfx slidercode, di75.jsfx:125-146)
    hpb1 = math.exp(-4.0 * math.pi * hp / sr) if hp > 0 else 0.0
    hpa0 = (hpb1 + 1.0) * 0.5
    threshv = math.exp(thresh * DB2LOG)
    makeupv = math.exp(gain * DB2LOG)
    atcoef = math.exp(-1.0 / ((atk_us / 1000000.0) * sr))
    relcoef = math.exp(-1.0 / ((rel_ms / 1000.0) * sr))

    hp0 = hp1 = old0 = old1 = rundb = 0.0
    out = []
    for l, r in inp:
        if hp > 0:
            hp0 = (l - old0) * hpa0 + hp0 * hpb1
            hp1 = (r - old1) * hpa0 + hp1 * hpb1
            old0, old1 = l, r
            l, r = hp0, hp1
        det = max(abs(l), abs(r))
        overdb = max(0.0, LOG2DB * math.log(det / threshv)) if det > 0 else 0.0
        coef = atcoef if overdb > rundb else relcoef
        rundb = overdb + coef * (rundb - overdb)
        grv = math.exp(-rundb * (ratio - 1.0) / ratio * DB2LOG)
        l *= grv * makeupv
        r *= grv * makeupv
        if mono:
            l = r = (l + r) * 0.5
        out.append((l, r))
    return out


def sine(amp, freq, n=N, sr=SR):
    return [amp * math.sin(2.0 * math.pi * freq * i / sr) for i in range(n)]


def mixed(n=N, sr=SR):
    # bursts of sines at different freqs + deterministic noise, L/R different
    out = []
    for i in range(n):
        t = i / sr
        env = 1.0 if (t % 0.5) < 0.25 else 0.05
        l = env * (0.7 * math.sin(2 * math.pi * 220 * t) + 0.2 * math.sin(2 * math.pi * 3000 * t))
        r = env * (0.6 * math.sin(2 * math.pi * 330 * t) + 0.25 * math.sin(2 * math.pi * 1700 * t))
        l += 0.05 * math.sin(2 * math.pi * (i * 7919 % 1000) * t)
        r += 0.04 * math.sin(2 * math.pi * (i * 104729 % 800) * t)
        out.append((l, r))
    return out


def f32(x):
    return struct.unpack('f', struct.pack('f', x))[0]


def write_raw(path, frames):
    with open(path, 'wb') as f:
        f.write(struct.pack('<%df' % (2 * len(frames)),
                            *[f32(v) for fr in frames for v in fr]))


def main():
    here = os.path.dirname(os.path.abspath(__file__))
    data = os.path.join(here, 'data')
    os.makedirs(data, exist_ok=True)

    # (name, hp, thresh, ratio, gain, atk_us, rel_ms, mono, signal)
    cases = [
        ('nulltest', 0, 0, 4, 0.0, 20, 250, 0, mixed()),
        ('comp_sine', 0, -20, 8, 3.5, 100, 200, 0,
         [(s, s * 0.8) for s in sine(0.8, 1000)]),
        ('full_monty', 120, -12, 4, -6.2, 20, 250, 0, mixed()),
        ('mono_case', 80, -18, 12, 2.0, 50, 300, 1, mixed()),
        ('quiet_pass', 100, -30, 20, 6.0, 500, 1000, 0,
         [(0.001 * math.sin(2 * math.pi * 440 * i / SR),
           0.001 * math.sin(2 * math.pi * 550 * i / SR)) for i in range(int(N))]),
    ]

    lines = ['# name hp_hz thresh_db ratio gain_db attack_us release_ms mono input_file ref_file']
    for name, hp, thr, ratio, gain, atk, rel, mono, inp in cases:
        # round-trip input through float32 so C++ sees identical samples
        inp32 = [(f32(l), f32(r)) for l, r in inp]
        ref = process(inp32, SR, hp, thr, ratio, gain, atk, rel, mono)
        ipath = os.path.join(data, name + '.in.raw')
        rpath = os.path.join(data, name + '.ref.raw')
        write_raw(ipath, inp32)
        write_raw(rpath, ref)
        lines.append('%s %d %d %d %.4f %d %d %d %s %s' %
                     (name, hp, thr, ratio, gain, atk, rel, mono,
                      os.path.basename(ipath), os.path.basename(rpath)))
        print('%-12s peak_ref=%.6f' % (name, max(max(abs(l), abs(r)) for l, r in ref)))

    with open(os.path.join(data, 'cases.txt'), 'w') as f:
        f.write('\n'.join(lines) + '\n')

    if '--selftest' in sys.argv:
        # 1) null test: default-flat params must be bit-transparent
        null = process(mixed(), SR, 0, 0, 4, 0.0, 20, 250, 0)
        mixed32 = [(f32(l), f32(r)) for l, r in mixed()]
        null32 = [(f32(l), f32(r)) for l, r in null]
        d = max(abs(a - b) for (a, _c), (b, _d) in zip(null32, mixed32))
        assert d == 0.0, 'null test failed: %g' % d
        # 2) mono must collapse L/R to identical channels
        m = process(mixed32, SR, 80, -18, 12, 2.0, 50, 300, 1)
        d = max(abs(l - r) for l, r in m)
        assert d < 1e-12, 'mono collapse failed: %g' % d
        # 3) compression must actually reduce loud peaks below unity path
        c = process(mixed32, SR, 0, -12, 8, 0.0, 20, 250, 0)
        pk_in = max(max(abs(l), abs(r)) for l, r in mixed32)
        pk_c = max(max(abs(l), abs(r)) for l, r in c)
        assert pk_c < pk_in, 'compression did not reduce peaks'
        print('selftest OK (null bit-transparent, mono collapse, peak reduction)')


if __name__ == '__main__':
    main()
