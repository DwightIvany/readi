-- Define the output file path
local outputFilePath = "C:\\markers.csv"

-- Open the output file for writing
local file = io.open(outputFilePath, "w")
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

-- Confirm the export with a message
reaper.ShowMessageBox("Markers and regions exported to C:\\markers.csv", "Export complete", 0)
