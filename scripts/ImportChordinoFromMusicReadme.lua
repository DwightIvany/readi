--[[
 * ReaScript Name: Import Chordino From Music Readme
 * Description: Likely only useful to Dwight modified from X-Raym
 * Instructions: Select a track. Run.
 * Author: Dwight Ivany
 * Author URI: https://github.com/dwightIvany/readi
 * Repository: GitHub > DwightIvany > Readi
 * Repository URI: https://github.com/dwightIvany/readi
 * Links
    Forum Thread nil
 * Licence: GPL v3
 * REAPER: 5.0
 * Version: 1.2
 * Date: 2024-09-29

 * Changelog:
 * v1.0 (2019-01-26 X-Raym version
 * v1.1 eliminate the need for user input in Dwight's workflow

My intention is to hard code this copy of ImportChordino.lua to simplify my workflow

If I use my repo on
G:\Data\Dropbox\ToDo\music-readme\chordino

Then my workflow could be
- Render file for SonicVisualizer
- Run Chordino and export to G:\Data\Dropbox\ToDo\music-readme\chordino\ProjectName -chordino.csv
- Run script that will export existing markers to G:\Data\Dropbox\ToDo\music-readme\chordino\ProjectName -markers.csv
- Import ProjectName -chordino.csv to replace markers
- Export ProjectName -chord markers.csv

Not only will this save me time, and improve consistency. It means I will automatically get git history on my markers.
That not only shows the evolution of chords, but of song structure

Always assume chordino file is csv

This script does the following steps:
1. Export existing markers
2. Delete existing markers
3. Import Chordino

As of 2024-09-29 the script has simplified my workflow, and no longer requires my user input
Future Steps could include:
- Convert existing regions to markers keeping color. I should first test this manually a few times
]]--

-- Begin undo block
reaper.Undo_BeginBlock()

function Msg(value) -- ToDo dup
  if console then
    reaper.ShowConsoleMsg(tostring(value) .. "\n")
  end
end

-- START OF USER CONFIG AREA
-- X-Raym version ask for csv / text, Dwight forces csv input with the next two lines
input_choose  = "csv"    
sep = ","     
-- this is different than Reapers default for export command
col_pos = 1 -- Position column index in the CSV aka #
col_name = 2 -- Name column index in the CSV
col_color = 3
col_pos_end = 4 -- Length column index in the CS
col_ticks = 5
col_len = 6 -- Length column index in the CSV
col_pattern = 7
-- Essential info for csv to make sense
bpm, beat_per_measure = reaper.GetProjectTimeSignature2(0)
-- END OF USER CONFIG AREA

function ColorHexToInt(hex)
  hex = hex:gsub("#", "")
  local R = tonumber("0x"..hex:sub(1,2))
  local G = tonumber("0x"..hex:sub(3,4))
  local B = tonumber("0x"..hex:sub(5,6))
  return reaper.ColorToNative(R, G, B)
end

-- X-Raym Optimization
local reaper = reaper

-- CSV to Table http://lua-users.org/wiki/LuaCsv
function ParseCSVLine (line,sep)
  local res = {}
  local pos = 1 
  sep = sep or ','
  while true do
    local c = string.sub(line,pos,pos)
    if (c == "") then break end
    if (c == '"') then
      -- quoted value (ignore separator within)
      local txt = ""
      repeat
        local startp,endp = string.find(line,'^%b""',pos)
        txt = txt..string.sub(line,startp+1,endp-1)
        pos = endp + 1
        c = string.sub(line,pos,pos)
        if (c == '"') then txt = txt..'"' end
        -- check first char AFTER quoted string, if it is another
        -- quoted string without separator, then append it
        -- this is the way to "escape" the quote char in a quote. example:
        --   value1,"blub""blip""boing",value3  will result in blub"blip"boing  for the middle
      until (c ~= '"')
      table.insert(res,txt)
      assert(c == sep or c == "")
      pos = pos + 3
    else
      -- no quotes used, just look for the first separator
      local startp,endp = string.find(line,sep,pos)
      if (startp) then
        table.insert(res,string.sub(line,pos,startp-1))
        pos = endp + 1
      else
        -- no separator found -> use rest of string and terminate
        table.insert(res,string.sub(line,pos))
        break
      end
    end
  end
  return res
end

--- UTILITIES

-- Display a message in the console for debugging
function Msg(value)
  if console then
    reaper.ShowConsoleMsg(tostring(value) .. "\n")
  end
end

function ReverseTable(t)
    local reversedTable = {}
    local itemCount = #t
    for k, v in ipairs(t) do
        reversedTable[itemCount + 1 - k] = v
    end
    return reversedTable
end

--- END OF UTILITIES

function read_lines(filepath)
  lines = {}
  local f = io.input(filepath)
  repeat
    s = f:read ("*l") -- read one line
    if s then  -- if not end of file (EOF)
      table.insert(lines, ParseCSVLine (s,sep))
    end
  until not s  -- until end of file
  f:close()
end

function snap_all_regions_to_grid()
      reaper.Main_OnCommand(40754, 0) -- Enable snap
      region_count , num_markersOut, num_regionsOut = reaper.CountProjectMarkers(0)
      for i=0, region_count -1 do
       --EnumProjectMarkers(i, is_region, region_start, region_end, #name, region_id)
       retval, isrgnOut, posOut, rgnendOut, region_name, markrgnindexnumberOut, colorOut = reaper.EnumProjectMarkers3(0, i)      
      --if isrgnOut then
         region_snapped_start =  reaper.SnapToGrid(0, posOut)
         region_snapped_end =  reaper.SnapToGrid(0, rgnendOut) 
         --SetProjectMarker(region_id, 1, region_snapped_start, region_snapped_end, #name)
         reaper.SetProjectMarker3( 0, markrgnindexnumberOut, isrgnOut, region_snapped_start, region_snapped_end, region_name , colorOut )
      end
end  

-- Dwight's function to get GetProjectPaths in a way I find useful
function GetProjectPaths()
  -- Get the full project path including the .rpp file
  local proj, projectPath = reaper.EnumProjects(-1, "")
  -- Detect if running on Windows or Unix-based (Mac/Linux) systems
  local isWindows = reaper.GetOS():match("Win")  
  -- Set the correct path separator based on the operating system
  local sep = isWindows and "\\" or "/"  
  -- Find the last occurrence of the separator to get only the folder path (excluding the .rpp file)
  local folderPath = projectPath:match("(.*" .. sep .. ")")
  -- Extract the file name (with extension)
  local projectFileName = projectPath:match("[^" .. sep .. "]+$")
  -- Extract the file name without extension
  local projectFileNameNoExt = projectFileName:match("(.+)%..+$")
  return folderPath, projectFileName, projectFileNameNoExt
end

-- Main function
function main()

--Dwight's hack to get the file in a folder I expect, instead og requiring my input
-- "G:\Data\Dropbox\ToDo\music-readme\chordino\" .. projectFileNameNoExt
  local folderPath, projectFileName, projectFileNameNoExt = GetProjectPaths()
  local csvChordinoInput = "G:\\Data\\Dropbox\\ToDo\\music-readme\\chordino\\" .. projectFileNameNoExt .. " -chordino.csv"
  -- ToDo make this Mac ready
  --  local sep = reaper.GetOS():match("Win") and "\\" or "/"
  -- local csvChordinoInput = "G:" .. sep .. "Data" .. sep .. "Dropbox" .. sep .. "ToDo" .. sep .. "music-readme" .. sep .. "chordino" .. sep .. projectFileNameNoExt .. "-chordino.csv"
  -- reaper.ShowConsoleMsg("csvChordinoInput: " .. csvChordinoInput .. "\n")

-- Export existing markers
-- Define the output file path
local exportMarkerPath = "G:\\Data\\Dropbox\\ToDo\\music-readme\\chordino\\" .. projectFileNameNoExt .. " -markers.csv"

-- Open the output file for writing
local file = io.open(exportMarkerPath, "w")

if not file then
  reaper.ShowMessageBox("Error opening file for writing", "Error", 0)
  return
end

-- Write the correct header row for Reaper's import format
file:write("#,Name,Start,End,Length,Color\n")

-- Get the number of markers and regions in the project
local retval, numMarkers, numRegions = reaper.CountProjectMarkers(0)

-- Loop through all markers and regions
for i = 0, numMarkers + numRegions - 1 do
  -- Get details for the current marker or region
  local retval, isRegion, position, regionEnd, name, idx = reaper.EnumProjectMarkers(i)

  -- If it's a region, calculate the length and include the end position
  local startPos = position
  local endPos = isRegion and regionEnd or ""
  local length = isRegion and (regionEnd - position) or ""
  
  -- Write the marker/region data to the CSV file, omitting the color (not used in this case)
  file:write(string.format("%d,%s,%.4f,%s,%s,\n", idx, name, startPos, endPos, length))
end

-- Close the file
file:close()
-- End of marker export

-- Confirm the export with a message
reaper.ShowMessageBox("Markers and regions exported", "Export complete", 0)

-- SWS: Delete all regions _SWSMARKERLIST10
reaper.Main_OnCommand(reaper.NamedCommandLookup("_SWSMARKERLIST10"), 0)
-- ToDo Convert regions to markers

  folder = csvChordinoInput:match[[^@?(.*[\/])[^\/]-$]]

  for i, line in ipairs( lines ) do
    --if line[col_pattern] == "pattern" then 
     if i > 1 then
      -- Name Variables
      local pos = line[col_pos]
      --local pos_end = tonumber(line[col_pos] +1) --tonumber(line[col_pos_end])
      local len =  line[col_len] 
      local name = line[col_name]
      local color = 0
        color = ColorHexToInt("#3776EB")|0x1000000
        --Msg(" Marker " .. pos .."Name ".. name)
        if name == "N" then i = i +1
           else
               reaper.AddProjectMarker2(0, false, pos, 0, name, -1, color)
        end
      end
    end    
end     

-- INIT
-- ToDo in debug, I needed my hack here maybe a local problem
local folderPath, projectFileName, projectFileNameNoExt = GetProjectPaths()
local csvChordinoInput = "G:\\Data\\Dropbox\\ToDo\\music-readme\\chordino\\" .. projectFileNameNoExt .. " -chordino.csv" --todo get this down to once
-- reaper.ShowConsoleMsg("csvChordinoInput: " .. csvChordinoInput .. "\n") --not run

read_lines(csvChordinoInput)

-- X-Raym's main code
reaper.PreventUIRefresh(1)
reaper.Undo_BeginBlock() -- Beginning of the undo block. Leave it at the top of your main function.
reaper.ClearConsole()

read_lines(csvChordinoInput)
-- reaper.Main_OnCommand( reaper.NamedCommandLookup( "_SWSMARKERLIST10" ), -1) -- SWS: Delete all regions
-- interesting that X-Raym had this commented out, I might want to move this up and include it
main()

snap_all_regions_to_grid()
commandID1 = reaper.NamedCommandLookup("_SWSMARKERLIST13")
reaper.Main_OnCommand(commandID1, 0) -- SWS: Convert markers to regions

reaper.Undo_EndBlock("Import Chordino", -1) -- End of the undo block. Leave it at the bottom of your main function.

reaper.UpdateArrange()
reaper.PreventUIRefresh(-1)
::finish::
