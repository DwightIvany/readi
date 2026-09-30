# readi
READI (as in ReaperDI) is a repo for Reaper Scripts and plugins that Dwight Ivany has written.

# SCRIPTS

## duplicate-src
I often find it useful to commit audio tracks; however, it is nice to have a copy that is unheard and unseen in the project that could be used, if I want to go back. Similar to an archve.

My workflow has been to duplicate, and then render one of the duplicates.
This script 
- disables the parent send
- disables fx(s)
- hides the track in TCP and MCP
- renames with -src at the end
This is not only CPU efficient, I find it this creatively stimulating and a useful workflow.

## render-submix-stems
I (almost) always group my projects into four sub-groups. I have
- harmony
- bass
- rhythm
- vox
I like to render these as stems. I have found over the years this is incredibly useful.

This script makes sure that mutes and solos are disabled (perhaps you don't want this for mutes).
It then requests a destination folder to save the wav files.

The file names are hard coded in the script, because that is 100% what I want most other people will want to modify this.

## 2dB

In the year 2000, before I had a limiter I liked. I used features in Steinberg Wavelab 2.0 - 4.0 to find the peak in a file and pull it down a couple dB at the zero crosssing. This tiny destructive edit, gave me a couple of dB of head room to avoid defects in the digital compressors and mixers at the time. With shortcuts it only took less than a minute.

Even with much better tools like decent limiters, I missed that, once I stopped using Wavelab. I find that to get a truly tranparent limiter setting, is not as easy and clean as this neat little hack that is almost always in audible.

I eventually figured out how to do this in Reaper, but it took to many clicks for me to do that often manually.

So I wrote a script that for the selected track:
- Finds the peak
- Cuts at the surrounding zero points
- Reduces the clip by 2dB
- Glues the track again

This results in a track that is almost identical. For things with a sharp transients, this change will be inaudible, yet may help avoid downstream artifacts in the mixing process. 

You could even run it multiple times on the same source file taming a few peaks.

For low frequency things like kick, organ and bass this will still work; however, it will affect more time. I first used this on accoustic guitar and I am convinced the change would be inaudible to the best of ears. For a low note on an organ, a decent set of ears should be able to A B the difference.

I have done lots of null testing to prove this theory to myself. I am surprised that this is not a feature in many DAWs as I think this is conceptually, so much better than running a limiter on an entire source track or mix.

# Plugins

## 6dB
An intentionally limited gain stage plugin that only does +/- 6dB.

Using the solo in front is a great mix technique, but sometimes I like to make one of the background items louder. So I find that I move a fader that is not in context of the entire mix.

Problem: when I leave solo I am not certain where the return the slider. For years I have been manually entering 6dB increments, because they are obvious

Solution: have a simple plug-in that only does +/- 6dB

Typically I will disable or delete the plug-in when done (certainly before render).

I use a keyboard shorcut, so that this is a two click process.

## dipass
An extremely simple high pass filter, with a default cutoff of 100 Hz. This is a first-order filter with a slope of 6dB per octave.

My idea is to have something as simple as the hardware switches I have on mics and preamps. The slider only goes from 20 to 400 Hz (that limit is just UX, and you can type a higher number, you desire).
## ditch
A pitch shifter (100% wet) that adjusts pitch in cents (-50 to +50), with an overlap size control and an optional filter that compensates for the pitch shifting artifacts.

## mondi
There are just so many reasons I want a plugin that simply does mono and nothing else. This one just averages the left and right channels.

I have the word mono hidden (hence "Mondi") to hide a pile of Waves bloat.

## simple-time-adjust
A simple delay plugin (original source Cockos) that delays the signal +/- 1000 ms, with separate wet and dry mix sliders. A negative delay advances the timing using plugin delay compensation (PDC).

## simplest-time-adjust
The same simple delay as above, but stripped down to a single delay amount slider and 100% wet output.

## di76
A FAST attack compressor based on the Stillwell 1175 JSFX.

The 1175 is modeled on the classic 1176 style FET compressor: ratios 4, 8, 12, 20 and All, with threshold, gain, attack (uS) and release (mS) controls, plus a gain reduction meter. I renamed it di76 and simplified it: I dropped the deprecated "Blown Capacitor" ratio modes, the "All" ratio, and the mix slider, because I want this to be simple. Ratio is now a slider (no dropdown) that snaps to 4, 8, 12 or 20, and there is a simple high-pass filter from my dipass plugin in front, before the fast compressor.

Credit where credit is due: this is the work of Thomas Scott Stillwell (Stillwell Audio), originally released under the BSD license. The copyright notice, license conditions and disclaimer are retained verbatim at the top of jsfx/di76.jsfx, as the license requires.

Original source: https://github.com/stillwellaudio/jsfx/blob/master/1175

See di76.md for a walkthrough of the compressor math and notes on the variables that silently default to zero.

## di75
The same compressor as di76, reduced to a sound-equivalent minimal implementation of the 1175. Same sliders, same sound (verified bit-identical output against di76 in simulation), but the dead code and no-op variables are gone: the vestigial peak tracker (`maxover`/`runmax`), the RMS scaffolding that collapsed to a plain peak detector (`rmscoef`/`runave`/square/sqrt), the never-enabled `softknee` and `autogain` hooks, the placeholder attack/release temps, and the redundant explicit zeroing in `@init`. I also inlined `grv`, `rpos`, and the attack/release coefficient math. Roughly half the code, identical audio.

The math is documented in di76.md — everything there applies here, minus the removed no-ops.

## di76-png
di76 with selectable TUKAN faces and clickable value entry, all in one plugin (selector in a new slider 22, so sessions from before it existed default to PNG):
- **PNG Classic** (default): framed knob images (HexKnob.png), blue faceplate, needle VU wired permanently to gain reduction with glow/shadow dressing.
- **GUI 1 White**: the di76 white skin - code-drawn white knobs with input/gain-reduction/output bar meters and peak-hold lines.
- **Generic sliders**: plain REAPER sliders, like the plugin had before it grew a GUI.

Pick the UI from the "GUI:" text at the top-right of the canvas modes, or from the "Gui mode" dropdown in generic mode. Both canvas modes share the same compact knob layout (two rows of three, gain under ratio, no title text) and the same click-a-label-to-type input boxes: numbers only, current value pre-filled and selected, Enter commits, Escape cancels, Gain is the only decimal knob - the others are integers everywhere (typed, displayed and dragged values all snap).

A mono/stereo switch (slider 23, default Stereo) sits on the VU dial in PNG mode (BL-1C's PlasticSwitch.png image) and as a plain labelled button in White mode - or as the "Mono" dropdown in generic mode. Mono engages the exact mondi.jsfx logic (`spl0 = spl1 = (spl0+spl1)*0.5`) after the makeup gain, and is completely out of the signal path when Stereo, so Stereo stays bit-transparent. This one needs five PNG files next to the plugin in jsfx/: HexKnob.png, Transtparent_Black_VU.png, VU_Shadow.png, Glow_VU.png (all from the library zip) plus PlasticSwitch.png (from the BL-1C folder of the TUKAN repo) - plus tk_gui_tools.jsfx-inc. Credits: Tukan_Studios library and artwork, ZenoMOD for the VU code, JClones/Tukan BL-1C layout (MIT), Stillwell DSP (BSD). Same DSP and the same DSP slider slots as di76/di76-tuk, with deliberate differences: ratio is continuous 4-20 (no snapping to 4/8/12/20), and the defaults are threshold -12 dB and high pass 80 Hz, so the filter is active on new instances (set it to 0 for the bit-transparent bypass).

Like di76-tuk, the native slider panel is hidden and the edge meters are off. This one needs four PNG files next to the plugin in jsfx/ (all from the library zip): HexKnob.png, Transtparent_Black_VU.png, VU_Shadow.png, Glow_VU.png - plus tk_gui_tools.jsfx-inc. Credits: Tukan_Studios library and artwork, ZenoMOD for the VU code, JClones/Tukan BL-1C layout (MIT), Stillwell DSP (BSD).

## di76-tuk
di76 with a fancy face: the same compressor and the same sliders, but a "GUI 1 White" interface drawn with the tk_gui_tools library by Tukan_Studios instead of the plain gain-reduction bar. It has knobs for all six controls plus input/gain-reduction/output meters. Because the slider block is identical to di76, saved sessions swap between the two.

The GUI code is the tk_gui_tools.jsfx-inc include file (vendored next to the plugin in jsfx/ — keep the two files together, REAPER resolves the import relative to the plugin). The native REAPER slider panel is hidden (`slider_show(slider, 0)`) and REAPER's edge I/O meter strips are turned off (`options: no_meter`); the knobs and in/GR/out meters in the canvas are the whole UI. Slider values are still saved in sessions and remain automatable. The library is "free as in free beer" for JSFX plugins, with parts of the VU meter code used with friendly permission from ZenoMOD. The white layout is adapted from Tukan's "Blue One Compressor BL-1C" example (MIT). GUI 1 draws everything from code, so no image files are needed. DSP credit is the same as di76: Thomas Scott Stillwell, BSD.
