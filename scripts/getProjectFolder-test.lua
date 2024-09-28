-- Function to get the folder path of the current project
function GetProjectFolder()
    -- Get the full project path including the .rpp file
    local proj, projectPath = reaper.EnumProjects(-1, "")
    
    -- Find the last occurrence of the directory separator (backslash or forward slash)
    -- This handles both Windows and Linux/Mac path formats
    local sep = string.match(projectPath, "[/\\]")
    
    -- Strip the file name to get only the directory path
    local folderPath = string.match(projectPath, "(.*" .. sep .. ")")
    
    return folderPath
end

-- Testing the function
local projectFolder = GetProjectFolder()
reaper.ShowConsoleMsg("Project folder: " .. projectFolder)