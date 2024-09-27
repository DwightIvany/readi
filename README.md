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

A intentionally limited gain stage plugin that only does +/- 6dB.

Using the solo in front is a great mix technique, but sometimes I like to make one of the background items louder. So I find that I move a fader that is not in context of the entire mix.

Problem: when I leave solo I am not certain where the return the slider. For years I have been manually entering 6dB increments, because they are obvious

Solution: have a simple plug-in that only does +/- 6dB

Typically I will disable or delete the plug-in when done (certainly before render).

I use a keyboard shorcut, so that this is a two click process.

# mono
There are just so many reasons I want a plugin that simply does mono and nothing else.
