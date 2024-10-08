--[[
 * ReaScript Name: Render Submix to Project Folder
 * Author: Dwight Ivany
 * Version: 1.0
 * Date: 2024-09-28
 * Description: Renders stems bass, harmony, rhythm and vox to user project .\submix\audio folder
 * REAPER: 6.0+
 *
 * Note this automatically closes render dialogs on completion.
 * Some users will want to re-enable this
--]]

-- Function to get the folder path of the current project
function GetProjectFolder()
  -- Get the full project path including the .rpp file
  local proj, projectPath = reaper.EnumProjects(-1, "")
  -- Detect if running on Windows or Unix-based (Mac/Linux) systems
  local isWindows = reaper.GetOS():match("Win")
  -- Set the correct path separator based on the operating system
  local sep = isWindows and "\\" or "/"
  -- Find the last occurrence of the separator to get only the folder path (excluding the .rpp file)
  local folderPath = projectPath:match("(.*" .. sep .. ")")
  return folderPath
end

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
-- Function to render selected track to a file
function render_track(track, name, destinationPath)
  -- Construct the full file path
  local filePath = destinationPath .. name .. ".wav"
  
  -- Debug: Output the file path
  reaper.ShowConsoleMsg("Trying to delete: " .. filePath .. "\n")
  
  -- Check if the file exists by trying to open it
  local file = io.open(filePath, "r")
  if file then
    file:close()
    
    -- Try to delete the file
    local result, errorMsg = os.remove(filePath)
    if result then
      reaper.ShowConsoleMsg("Deleted existing file: " .. filePath .. "\n")
    else
      reaper.ShowConsoleMsg("Failed to delete file: " .. errorMsg .. "\n")
    end
  else
    reaper.ShowConsoleMsg("File does not exist: " .. filePath .. "\n")
  end

  -- Proceed with rendering after deletion attempt
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
-- Get project folder path
local isWindows = reaper.GetOS():match("Win") -- Detect if running on Windows or Mac/Linux
local projectFolder = GetProjectFolder()
local destinationPath = projectFolder .. (isWindows and "Submix\\Audio\\" or "Submix/Audio/")
clear_mutes_and_solos()

-- Process the tracks and render
process_tracks(destinationPath)

reaper.Undo_EndBlock("Render individual tracks", -1)
