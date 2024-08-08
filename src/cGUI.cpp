#include "cMapify.h"
#include "cGUI.h"

cGUI::cGUI()
    : cStarterGUI(
          "Mapify",
          {50, 50, 1000, 1000}),
      myScale(1.0 / 200.0),
      myXoff(350000),
      myYoff(380000),
      myDisplayTab(eDisplayTab::viz)
{
    constructMenus();
    registerEventHandlers();
    show();
    run();
}

void cGUI::constructMenus()
{

    wex::menubar mbar(fm);

    wex::menu mfile(fm);
    mfile.append("Read Waypoint file",
                 [&](const std::string &title)
                 {
                     // prompt user to select waypoint file
                     wex::filebox fb(fm);
                     auto fname = fb.open();
                     if (fname.empty())
                         return;
                     fm.text("Mapify " + fname);
                     M.readWaypoints(fname);
                     scale();
                     M.calculate();
                     fm.update();
                 });
    mfile.append("Run unit tests",
                 [&](const std::string &title)
                 {
                     if (!M.unitTest())
                     {
                         wex::msgbox("Unit test failed");
                         exit(1);
                     }
                     wex::msgbox("Unit tests passed");
                 });
    mbar.append("File", mfile);

    // static wex::menu malgo(fm);
    // malgo.append("Cluster",
    //              [&](const std::string &title)
    //              {
    //                  M.algoCluster();
    //                  M.calculate();
    //                  malgo.check(0);
    //                  malgo.check(1, false);
    //                  fm.update();
    //              });
    // malgo.append("Greedy",
    //              [&](const std::string &title)
    //              {
    //                  M.algoGreedy();
    //                  M.calculate();
    //                  malgo.check(0, false);
    //                  malgo.check(1);
    //                  fm.update();
    //              });
    // mbar.append("Algorithm", malgo);

    static wex::menu mdisplay(fm);
    mdisplay.append("Visualization",
                    [&](const std::string &title)
                    {
                        DisplayViz();
                        mdisplay.check(0);
                        mdisplay.check(1, false);
                        mdisplay.check(2, false);
                        fm.update();
                    });
    mdisplay.append("Page Locations",
                    [&](const std::string &title)
                    {
                        DisplayPages();
                        mdisplay.check(0, false);
                        mdisplay.check(1);
                        mdisplay.check(2, false);
                        fm.update();
                    });
    mdisplay.append("Uncovered",
                    [&](const std::string &title)
                    {
                        DisplayUncovered();
                        mdisplay.check(0, false);
                        mdisplay.check(1, false);
                        mdisplay.check(2);
                        fm.update();
                    });
    mdisplay.appendSeparator();
    mdisplay.append("Pan left",
                    [&](const std::string &title)
                    {
                        panLeft();
                        fm.update();
                    });
    mdisplay.append("Pan right",
                    [&](const std::string &title)
                    {
                        panRight();
                        fm.update();
                    });
    mdisplay.append("Pan up",
                    [&](const std::string &title)
                    {
                        panUp();
                        fm.update();
                    });
    mdisplay.append("Pan down",
                    [&](const std::string &title)
                    {
                        panDown();
                        fm.update();
                    });
    mdisplay.check(0);
    mbar.append("Display", mdisplay);
}
void cGUI::registerEventHandlers()
{
    fm.events().draw(
        [&](PAINTSTRUCT &ps)
        {
            wex::shapes S(ps);
            waypointsDisplay(S);
            pageDisplay(S);
            uncoveredDisplay(S);
        });

    // handle mouse wheel
    fm.events().mouseWheel(
        [&](int dist)
        {
            if (dist > 0)
                incScale();
            else
                decScale();
            fm.update();
        });
}


void cGUI::scale()
{
    double xmin, xmax, ymin, ymax;
    xmax = ymax = 0;
    xmin = ymin = INT_MAX;
    for (auto &p : cMapify::theWayPoints)
    {
        if (p.x < xmin)
            xmin = p.x;
        if (p.y < ymin)
            ymin = p.y;
        if (p.x > xmax)
            xmax = p.x;
        if (p.y > ymax)
            ymax = p.y;
    }

    myXoff = xmin;
    myYoff = ymin;

    double xscale = 800 / (xmax - xmin);
    double yscale = 800 / (ymax - ymin);
    myScale = xscale;
    if (yscale < xscale)
        myScale = yscale;
}

void cGUI::waypointsDisplay(wex::shapes &S)
{
    if (!cMapify::theWayPoints.size())
        return;
    if (myDisplayTab != eDisplayTab::viz)
        return;

    S.color(0x0000FF);
    for (auto &p : cMapify::theWayPoints)
        S.pixel(
            myScale * (p.x - myXoff),
            myScale * (p.y - myYoff));
}
void cGUI::uncoveredDisplay(wex::shapes &S)
{
    // if (myDisplayTab != eDisplayTab::uncovered)
    //     return;
    // S.color(0x0000FF);
    // for (int ic = 0; ic < myCovered.size(); ic++)
    //     if (!myCovered[ic])
    //         S.pixel(
    //             myScale * (cMapify::theWayPoints[ic].x - myXoff),
    //             myScale * (cMapify::theWayPoints[ic].y - myYoff));
}
void cGUI::pageDisplay(wex::shapes &S)
{
    S.color(0);

    if (myDisplayTab == eDisplayTab::viz)
    {
        int w = myScale * (cPage::thePaper.x);
        int h = myScale * (cPage::thePaper.y);
        for (int c = 0; c < cMapify::thePages.size(); c++)
            S.rectangle(
                {(int)(myScale * (cMapify::thePages[c].center.x - myXoff) - w / 2),
                 (int)(myScale * (cMapify::thePages[c].center.y - myYoff) - h / 2),
                 w, h});
    }
    if (myDisplayTab == eDisplayTab::page)
    {
        S.text(M.text(), {10, 10, 1000, 1000});
    }
}
