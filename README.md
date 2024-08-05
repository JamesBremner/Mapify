# Mapify

![image](https://github.com/user-attachments/assets/49e8b937-73e0-452c-9b5e-874a2870f35e)<br>
Photo: @peaklass1


## Problem Statement

I do the occasional long distance hike and often require printing 10+ A3 maps.

I'm trying to find an algorithmic solution to the manual process of printing off multiple maps of a route.

The manual process is often non-optimal, in which I mean pages should be kept to a minimum. Eg. Extra pages are added, where as if I had moved previous pages over slightly here and there it would have encompassed the route in fewer pages.

To complicate things printing can be done in both portrait and landscape modes.

Id really appreciate a push in the right direction. Is there a simple solution or does it require some more elaborate thinking?

To clarify: Given a list of 2d points, find the minimum amount of boxes they will fit in. With the boxes being either wh (portrait) or hw (landscape). Also the maps cannot be rescaled.

## Algorithm

A two-pass algorithm.  First pass maximizes point coverage with no page overlaps.  Second pass clusters uncovered points, with page overlaps allowd.

```
  ( Pass 1 )
- Create a page that covers the first waypoint and the maximum number of other waypoints
- FOR ever
   - find page margin ( top, left, right, bottom ) nearest to last waypoint covered
   - add page adjacent without overlap to margin that covers the maximum number of new waypoints
   - IF last waypoint covered
        - BREAK
- ENDLOOP for ever

  ( Pass 2 )
- FOR pageCount = 1 to 100
  - Apply K-Means algo to uncovered waypoints to find pageCount clusters
  - Create page centred on center of each cluster
  - If all waypoints on at least one page
      - DONE
- ENDLOOP over pageCount
- FAILED
```

## GUI

To read a waypoint file select from menu File

To run the unit tests, select from menu file.

To display a vizualization of the waypoints ( red dots ) and the page locations ( black rectangles ) select from menu Display

To display page locations ( x,y ) select from menu Display

To zoom vizualization in or out, use the mouse wheel.

To pan vizualization, select from menu Display


## Results

![image](https://github.com/user-attachments/assets/c823184a-2760-415f-8def-185366005447)

