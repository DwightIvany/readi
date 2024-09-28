--[[
I am quite organized in how I structure my Project folders.
If my scripts knew the path of the reaper project without the filename,
they could easily run without my user input.

Oddly it does not seem to be built in https://forums.cockos.com/showthread.php?t=189922
It seems odd to me but this function will do this, so I have saved it in my repo.

I still need to test this on a Mac 2024-09-28

I have decided to expand this, to return information that I will likely parse

]]--

-- Function to get the folder path, file name, and file name without extension of the current project
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
    local fileName = projectPath:match("[^" .. sep .. "]+$")
  
    -- Extract the file name without extension
    local fileNameNoExt = fileName:match("(.+)%..+$")
  
    return folderPath, fileName, fileNameNoExt
  end
  
  -- Example usage
  local folderPath, fileName, fileNameNoExt = GetProjectPaths()
  reaper.ShowConsoleMsg("Folder Path: " .. folderPath .. "\n")
  reaper.ShowConsoleMsg("File Name: " .. fileName .. "\n")
  reaper.ShowConsoleMsg("File Name without Extension: " .. fileNameNoExt .. "\n")
  