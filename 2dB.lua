--[[
ToDo
- Add decent header
- Create decent command ID
- Improve comments
- Update readme
- Additional testing
- Update submix to note that it closes
- Remove or edit my text
- Have this only run on the selected track
- Wrap the undo

]]--
-- Step 1: Get the selected track
local track = reaper.GetSelectedTrack(0, 0)
if track == nil then
    reaper.ShowConsoleMsg("No track selected.\n")
    return
end

-- Get the first selected media item on the track
local item = reaper.GetTrackMediaItem(track, 0) -- Assume we're working with the first item on the track
if item == nil then
    reaper.ShowConsoleMsg("No media item found on the selected track.\n")
    return
end

-- Move cursor to the peak of the selected media item (SWS: Move cursor to item peak sample)
reaper.Main_OnCommand(reaper.NamedCommandLookup("_SWS_FINDITEMPEAK"), 0)

-- Store the peak position
local peak_pos = reaper.GetCursorPosition()

-- Disable auto-crossfades (41196: Options: Disable auto-crossfade on split)
reaper.Main_OnCommand(41196, 0)

-- Step 2: Move cursor to the next zero crossing in the selected item
reaper.Main_OnCommand(40790, 0) -- Move edit cursor to next zero crossing in items

-- Store the next zero crossing position
local next_zero_pos = reaper.GetCursorPosition()

-- Split item at the next zero crossing
reaper.Main_OnCommand(40196, 0) -- Item: Split items at play cursor

-- Step 3: Move cursor to the previous zero crossing in the selected item
reaper.Main_OnCommand(40791, 0) -- Move edit cursor to previous zero crossing in items

-- Store the previous zero crossing position
local prev_zero_pos = reaper.GetCursorPosition()

-- Split item at the previous zero crossing
reaper.Main_OnCommand(40196, 0) -- Item: Split items at play cursor

-- Re-enable auto-crossfades (41195: Options: Enable auto-crossfade on split)
reaper.Main_OnCommand(41195, 0)

-- Step 4: Move cursor back to the peak position and select the item at that position
reaper.SetEditCurPos(peak_pos, false, false) -- Move the cursor back to the peak position

-- Select the item under the edit cursor at the peak position
reaper.Main_OnCommand(reaper.NamedCommandLookup("_XENAKIOS_SELITEMSUNDEDCURSELTX"), 0)

-- Step 5: Lower the volume of the peak item by 2 dB
local peak_item = reaper.GetSelectedMediaItem(0, 0)
if peak_item ~= nil then
    local take = reaper.GetActiveTake(peak_item)
    if take ~= nil then
        local current_volume = reaper.GetMediaItemTakeInfo_Value(take, "D_VOL")
        local new_volume = current_volume * 10 ^ (-2 / 20) -- Lower by 2 dB
        reaper.SetMediaItemTakeInfo_Value(take, "D_VOL", new_volume)
    end
end

-- Step 6: Select all three items (previous, peak, and next)
reaper.Main_OnCommand(40289, 0) -- Unselect all items

-- Loop through items on the selected track
local num_items = reaper.CountTrackMediaItems(track)
for i = 0, num_items - 1 do
    local current_item = reaper.GetTrackMediaItem(track, i)
    local item_start = reaper.GetMediaItemInfo_Value(current_item, "D_POSITION")
    local item_end = item_start + reaper.GetMediaItemInfo_Value(current_item, "D_LENGTH")
    
    -- Check if the item's start or end overlaps with our previous, peak, or next zero crossing positions
    if (prev_zero_pos >= item_start and prev_zero_pos <= item_end) or
       (peak_pos >= item_start and peak_pos <= item_end) or
       (next_zero_pos >= item_start and next_zero_pos <= item_end) then
        reaper.SetMediaItemSelected(current_item, true)
    end
end

-- Step 7: Glue the selected items
reaper.Main_OnCommand(41588, 0) -- Glue selected items

-- Optional: Update Reaper's arrangement view
reaper.UpdateArrange()

-- Show positions for debugging (optional)
reaper.ShowConsoleMsg("Peak Position: " .. peak_pos .. "\n")
reaper.ShowConsoleMsg("Next Zero Crossing Position: " .. next_zero_pos .. "\n")
reaper.ShowConsoleMsg("Previous Zero Crossing Position: " .. prev_zero_pos .. "\n")
