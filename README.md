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

```
- FOR pageCount = 1 to 100
  - Apply K-Means algo to waypoints to find pageCount clusters
  - Create page centred on center of each cluster
  - If all waypoints on at least one page
      - DONE
  - If missed waypoints count < some specified % of total waypoints
       - FOR missedPageCount = 1 to 100
           - Apply K-Means algo to missed waypoints to find missedPageCount clusters
           - Create page on center of each missed cluster
           - If all waypoints on at least one page
                - DONE
       - ENDLOOP over missedPageCount
- ENDLOOP over pageCount
- FAILED
```

## GUI

The GUI displays the waypoints ( red ) and the page locations ( black ).

To zoom in or out, use the mouse wheel.

To pan, right click and select for pupup menu

