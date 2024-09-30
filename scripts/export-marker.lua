--[[
 * ReaScript Name: Export Marker to Music Readme
 * Description: Likely only useful to Dwight
 * Instructions: Run
 * Author: Dwight Ivany
 * Version: 0.9
 * Date: 2024-09-30

 * Changelog:
 * v0.9 2024-09-30
 ]]--

-- Begin undo block
reaper.Undo_BeginBlock()

function Msg(value) -- ToDo dup
  if console then
    reaper.ShowConsoleMsg(tostring(value) .. "\n")
  end
end

reaper.Undo_EndBlock("Export Marker to Music Readme", -1) -- End of the undo block