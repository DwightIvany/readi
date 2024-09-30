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

    Msg(projectFileNameNoExt)


    --    local csvChordinoInput = "G:\\Data\\Dropbox\\ToDo\\music-readme\\chordino\\" .. projectFileNameNoExt .. " -chordino.csv"
    -- ToDo make this Mac ready
    --  local sep = reaper.GetOS():match("Win") and "\\" or "/"
    -- local csvChordinoInput = "G:" .. sep .. "Data" .. sep .. "Dropbox" .. sep .. "ToDo" .. sep .. "music-readme" .. sep .. "chordino" .. sep .. projectFileNameNoExt .. "-chordino.csv"
    -- reaper.ShowConsoleMsg("csvChordinoInput: " .. csvChordinoInput .. "\n")
  
  -- Export existing markers
  -- Define the output file path
  local exportMarkerPath = "G:\\Data\\Dropbox\\ToDo\\music-readme\\chordino\\" .. projectFileNameNoExt .. " -markers.csv"
 -- reaper.ShowMessageBox("Will export" .. exportMarkerPath, 0)
  
  -- Open the output file for writing
  local file = io.open(exportMarkerPath, "w")
  
  if not file then
    reaper.ShowMessageBox("Error opening file for writing", "Error", 0)
    return
  end
  
  -- Write header row for Reaper's import format
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
end

main()

reaper.Undo_EndBlock("Export Marker to Music Readme", -1) -- End of the undo block