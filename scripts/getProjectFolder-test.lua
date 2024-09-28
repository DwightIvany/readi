--[[
I am quite organized in how I structure my Project folders.
If my scripts knew the path of the reaper project without the filename,
they could easily run without my user input.

Oddly it does not seem to be built in https://forums.cockos.com/showthread.php?t=189922
It seems odd to me but this function will do this, so I have saved it in my repo.

I still need to test this on a Mac 2024-09-28    
]]--

-- Function to get the folder path of the current project
function GetProjectFolder()
    -- Get the full project path including the .rpp file
    local proj, projectPath = reaper.EnumProjects(-1, "")
    -- Find the last slash of GetProjectFolder both Windows and Linux/Mac
    local lastSlash = string.match(projectPath, "[/\\]")
    reaper.ShowConsoleMsg("\nlastSlash: " .. lastSlash)
    -- Strip the file name to get only the directory path
    local folderPath = string.match(projectPath, "(.*" .. lastSlash .. ")")   
    return folderPath
end

-- Testing the function
local projectFolder = GetProjectFolder()
reaper.ShowConsoleMsg("\nProject folder: " .. projectFolder)