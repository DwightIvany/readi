# readi
READI (as in ReaperDI) is a repo for Reaper Scripts and plugins that Dwight Ivany has written.

# duplicate-src
I often find it useful to commit audio tracks; however, it is nice to have a copy that is unheard and unseen in the project that could be used, if I want to go back. Similar to an archve.

My workflow has been to duplicate, and then render one of the duplicates.
This script 
- disables the parent send
- disables fx(s)
- hides the track in TCP and MCP
- renames with -src at the end
This is not only CPU efficient, I find it this creatively stimulating and a useful workflow.

# render-submix-stems
I (almost) always group my projects into four sub-groups. I have
- harmony
- bass
- rhythm
- vox
I like to render these as stems. I have found over the years this is incredibly useful.

This script makes sure that mutes and solos are disabled (perhaps you don't want this for mutes).
It then requests a destination folder to save the wav files.

The file names are hard coded in the script, because that is 100% what I want most other people will want to modify this.
