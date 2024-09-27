--[[
 * ReaScript Name: Duplicate Source
 * Author: Dwight Ivany
 * Version: 1.0
 * Date: 2024-09-24
 * Description: Duplicates the selected track and disables the parent send, fx, hides and renames
 * REAPER: 6.0+
--]]

-- Begin undo block
reaper.Undo_BeginBlock()

-- Check how many tracks are selected
local num_selected_tracks = reaper.CountSelectedTracks(0)
-- If more than one track is selected, display an error and exit
if num_selected_tracks ~= 1 then
    reaper.ShowMessageBox("Please select only one track.", "Error", 0)
    return
end

-- Get the selected track
local track = reaper.GetSelectedTrack(0, 0)

-- Run the duplicate track command (ID: 40062)
reaper.Main_OnCommand(40062, 0) -- Duplicate track

-- Get the selected track
local track = reaper.GetSelectedTrack(0, 0)
    if track then
        reaper.SetMediaTrackInfo_Value(track, "B_MAINSEND", 0) -- disable master send
        reaper.SetMediaTrackInfo_Value(track, "D_VOL", 0) -- Set volume fader to 0
        reaper.SetMediaTrackInfo_Value(track, "B_SHOWINMIXER", 0) -- hide in MCP aka MIX
        reaper.SetMediaTrackInfo_Value(track, "B_SHOWINTCP", 0) -- hide in TCP

        -- Rename the track by appending "-src" to its current name
        local retval, current_name = reaper.GetSetMediaTrackInfo_String(track, "P_NAME", "", false)
        reaper.GetSetMediaTrackInfo_String(track, "P_NAME", current_name .. "-src", true)

        -- Disable (bypass) all FX on the track
    local num_fx = reaper.TrackFX_GetCount(track)  -- Get the number of FX on the track
    for i = 0, num_fx - 1 do
        reaper.TrackFX_SetEnabled(track, i, false)  -- Disable each FX by index
    end
end

-- Update the display
reaper.TrackList_AdjustWindows(false)
reaper.UpdateArrange()

-- End undo block and commit
reaper.Undo_EndBlock("Duplicate Track and Disable Parent Send", -1)
