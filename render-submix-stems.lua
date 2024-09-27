--[[
 * ReaScript Name: Render Submix Stems
 * Author: Dwight Ivany
 * Version: 1.0
 * Date: 2024-09-24
 * Description: Renders stems bass, harmony, rhythm and vox to user specified folder
 * REAPER: 6.0+
 *
 * Note this automatically closes render dialogs on completion.
 * Some users will want to re-enable this

--]]

-- Function to turn off all mutes and solos
function clear_mutes_and_solos()
  local track_count = reaper.CountTracks(0)
  for i = 0, track_count - 1 do
    local track = reaper.GetTrack(0, i)
    -- Unmute track
    reaper.SetMediaTrackInfo_Value(track, "B_MUTE", 0)
    -- Unsolo track
    reaper.SetMediaTrackInfo_Value(track, "I_SOLO", 0)
  end
end

-- Function to render selected track to a file
function render_track(track, name, destinationPath)
  -- Clear all solo states first
  reaper.Main_OnCommand(40340, 0) -- Unsolo all tracks
  
  -- Solo the selected track
  reaper.SetMediaTrackInfo_Value(track, "I_SOLO", 1)
  
  -- Set render path (only directory) and file name separately
  reaper.GetSetProjectInfo_String(0, "RENDER_FILE", destinationPath, true) -- Set only the destination path (no file name)
  
  -- Use the file name separately in the render process
  reaper.GetSetProjectInfo_String(0, "RENDER_PATTERN", name, true) -- Set only the file name (e.g., "bass", "harmony")

  -- Set the render source to Master Mix (not Master Mix + Stems)
  reaper.GetSetProjectInfo(0, "RENDER_SETTINGS", 0, true) -- 0 means render only the master mix

  -- Set to overwrite existing files (no -001, -002)
  reaper.GetSetProjectInfo(0, "RENDER_OVERWRITE", 1, true)

  -- Set render to 24-bit stereo WAV at 48k
  reaper.GetSetProjectInfo(0, "RENDER_FORMAT", 3, true)  -- WAV format
  reaper.GetSetProjectInfo(0, "RENDER_SRATE", 48000, true)  -- Sample rate 48k
  reaper.GetSetProjectInfo(0, "RENDER_CHANNELS", 2, true)  -- Stereo
  reaper.GetSetProjectInfo(0, "RENDER_RESAMPLE", 1, true)  -- Resample if necessary
  reaper.GetSetProjectInfo(0, "RENDER_BITS", 24, true)  -- 24-bit

  -- Render project without showing the render dialog (background render)
  reaper.Main_OnCommand(42230, 0) -- Render project silently (without showing the render dialog)

  -- Unsolo track after rendering
  reaper.SetMediaTrackInfo_Value(track, "I_SOLO", 0)
end

-- Main function to process the tracks
function process_tracks(destinationPath)
  local track_names = {"harmony", "bass", "rhythm", "vox"}
  local track_count = reaper.CountTracks(0)

  for i = 1, #track_names do
    local track_name_to_find = track_names[i]:lower()

    for j = 0, track_count - 1 do
      local track = reaper.GetTrack(0, j)
      local _, track_name = reaper.GetTrackName(track)

      if track_name:lower() == track_name_to_find then
        render_track(track, track_name_to_find, destinationPath)
      end
    end
  end
end

-- Run the script
reaper.Undo_BeginBlock()

clear_mutes_and_solos()

-- Prompt user for destination path
retval, destinationPath = reaper.GetUserInputs("Destination Path", 1, "Enter destination folder path:", "")
if retval then
  process_tracks(destinationPath)
end

reaper.Undo_EndBlock("Render individual tracks", -1)
