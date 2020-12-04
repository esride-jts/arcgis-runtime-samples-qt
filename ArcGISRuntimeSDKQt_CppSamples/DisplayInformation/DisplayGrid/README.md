# Display a grid

Display coordinate system grids including Latitude/Longitude, MGRS, UTM and USNG on a map view. Also, toggle label visibility and change the color of grid lines and grid labels.

![](screenshot.png)

## Use case

Grids are often used on printed maps, but can also be helpful on digital maps, to identify locations on a map.

## How to use the sample

Tap on the `Change Grid` button in the toolbar to open a settings view. You can select type of grid from `Grid Type` (LatLong, MGRS, UTM and USNG) and modify its properties like label visibility, grid line color, and grid label color.

## How it works

1. Create an instance of one of the `Grid` types.
1. Grid lines and labels can be styled per grid level with `setLineSymbol(gridLevel, lineSymbol)` and `setTextSymbol(gridLevel, textSymbol)` methods on the grid.
1. The label position can be set with `setLabelPosition(labelPosition)` method on the grid.
1. For the `LatitudeLongitudeGrid` type, you can specify a label format of `DecimalDegrees` or `DegreesMinutesSeconds`.
1. To set the grid, use the `setGrid(grid)` method on the map view.

## Relevant API

* ArcGISGrid
* LatitudeLongitudeGrid
* LatitudeLongitudeGridLabelFormat::DecimalDegrees
* LatitudeLongitudeGridLabelFormat::DegreesMinutesSeconds
* MapView
* MGRSGrid
* SimpleLineSymbol
* TextSymbol
* USNGGrid
* UTMGrid

## Tags

coordinates, degrees, graticule, grid, latitude, longitude, MGRS, minutes, seconds, USNG, UTM
