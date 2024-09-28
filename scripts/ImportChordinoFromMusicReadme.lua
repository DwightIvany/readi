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
 * Version: 1.0

ImportChordinoFromMusicReadme.lua

--]]

--[[
 * Changelog:
 * v1.0 (2019-01-26)
  + Initial Release

My intention is to hard code this to simplify my workflow

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

Step 1. Replace user input
Test example: G:\Data\Dropbox\ToDo\music-readme\chordino\Debug -chordino.cs
"G:\Data\Dropbox\ToDo\music-readme\chordino\" .. projectFileNameNoExt
return folderPath, projectFileName, projectFileNameNoExt

I currently have csvChordinoInput be the file I want

ToDo
stop asking for the file
wrap in undo
always assume csv
--]]

-- USER CONFIG AREA -----------------------------------------------------------
-- Duplicate and Rename the script if you want to modify this.
-- Else, a script update will erase your mods.

-- Display a message in the console for debugging
function Msg(value)
  if console then
    reaper.ShowConsoleMsg(tostring(value) .. "\n")
  end
end

Info = 
[[Sonic Visualiser load your audio file
Transform > Chordino
File > Export Annotation Layer
https://www.sonicvisualiser.org/  Win/Lin/Mac

Chordino Vamp Plugin
http://www.isophonics.net/nnls-chroma
  
]]
reaper.MB(Info, "Creating reapeak file", 0)

retval_split,input_choose  = reaper.GetUserInputs("Choose File Type", 1, "txt or csv", "csv")
            
            
if not retval_split then goto finish end
            
--if input_choose then

console = true -- true/false: display debug messages in the console

     if input_choose == "txt" then sep = "\t" end-- default sep
     if input_choose == "csv" then sep = "," end 
--(1)pattern,(2)type,(3)A verse B chorus,(4)weight,(5)mask,(6)duration,(7)bar
--(1)shot,(2)0 shot 1 hold,(3)bar,(4)ticks,(5)duration ticks,weight,volume

col_pos = 1 -- Position column index in the CSV
col_pos_end = 4 -- Length column index in the CS
col_len = 6 -- Length column index in the CSV
col_name = 2 -- Name column index in the CSV
col_color = 3
col_pattern = 7
col_ticks = 5

bpm, beat_per_measure = reaper.GetProjectTimeSignature2(0)

------------------------------------------------------- END OF USER CONFIG AREA

function ColorHexToInt(hex)
  hex = hex:gsub("#", "")
  local R = tonumber("0x"..hex:sub(1,2))
  local G = tonumber("0x"..hex:sub(3,4))
  local B = tonumber("0x"..hex:sub(5,6))
  return reaper.ColorToNative(R, G, B)
end

-- Optimization
local reaper = reaper

-- CSV to Table
-- http://lua-users.org/wiki/LuaCsv
function ParseCSVLine (line,sep)
  --[[
  local line = string.gsub(line, "{.*}", "") -- remove { } and text between them
  local line = string.gsub(line, ";.*", "") -- remove ; and text after it
  local line = string.gsub(line, "pattern,PreFill.*", "") -- remove pattern,PreFill line
  
  --]]
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

-- UTILITIES -------------------------------------------------------------

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

--------------------------------------------------------- END OF UTILITIES

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
    --end  
end  

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

--[[Dwight's hack]]--
-- "G:\Data\Dropbox\ToDo\music-readme\chordino\" .. projectFileNameNoExt
  local folderPath, projectFileName, projectFileNameNoExt = GetProjectPaths()
  local csvChordinoInput = "G:\\Data\\Dropbox\\ToDo\\music-readme\\chordino\\" .. projectFileNameNoExt .. "-chordino.csv"
  --  local sep = reaper.GetOS():match("Win") and "\\" or "/"
  -- local csvChordinoInput = "G:" .. sep .. "Data" .. sep .. "Dropbox" .. sep .. "ToDo" .. sep .. "music-readme" .. sep .. "chordino" .. sep .. projectFileNameNoExt .. "-chordino.csv"
  reaper.ShowConsoleMsg("csvChordinoInput: " .. csvChordinoInput .. "\n")
--End my Hack

  folder = filetxt:match[[^@?(.*[\/])[^\/]-$]]

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
         
         
         --snap_all_regions_to_grid()
                  
    end
      
end     

-- INIT

retval, filetxt = reaper.GetUserFileNameForRead("", "Import Chordino chords to regions", input_choose)

if retval then

  reaper.PreventUIRefresh(1)

  reaper.Undo_BeginBlock() -- Begining of the undo block. Leave it at the top of your main function.
  
  reaper.ClearConsole()

  read_lines(filetxt)
  
  -- reaper.Main_OnCommand( reaper.NamedCommandLookup( "_SWSMARKERLIST10" ), -1) -- SWS: Delete all regions

  main()
  
  snap_all_regions_to_grid()
  
  commandID1 = reaper.NamedCommandLookup("_SWSMARKERLIST13")
  reaper.Main_OnCommand(commandID1, 0) -- SWS: Convert markers to regions

  reaper.Undo_EndBlock("ReaTrak chordino chords csv to regions", -1) -- End of the undo block. Leave it at the bottom of your main function.

  reaper.UpdateArrange()

  reaper.PreventUIRefresh(-1)

end

::finish::