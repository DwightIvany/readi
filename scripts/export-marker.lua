-- Define the output file path
local outputFilePath = "G:\\Data\\Dropbox\\ToDo\\music-readme\\chordino\\markers.csv"

-- Open the output file for writing
local file = io.open(outputFilePath, "w")
if not file then
  reaper.ShowMessageBox("Error opening file for writing", "Error", 0)
  return
end

-- Write the header row
file:write("Index,Type,Position,Length,Name\n")

-- Get the number of markers and regions in the project
local retval, numMarkers, numRegions = reaper.CountProjectMarkers(0)

-- Loop through all markers and regions
for i = 0, numMarkers + numRegions - 1 do
  -- Get details for the current marker or region
  local retval, isRegion, position, regionEnd, name, idx = reaper.EnumProjectMarkers(i)

  -- Determine if it's a marker or a region
  local markerType = isRegion and "Region" or "Marker"

  -- Calculate the length of the region (for markers, this will be 0)
  local length = isRegion and (regionEnd - position) or 0

  -- Write the marker/region data to the CSV file
  file:write(string.format("%d,%s,%.4f,%.4f,%s\n", idx, markerType, position, length, name))
end

-- Close the file
file:close()

reaper.ShowMessageBox("Markers and regions exported to C:\\markers.csv", "Export complete", 0)
