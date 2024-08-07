#include "cMapify.h"
#include "cGUI.h"

cGUI::cGUI()
    : cStarterGUI(
          "Mapify",
          {50, 50, 1000, 1000})
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
                        M.DisplayViz();
                        mdisplay.check(0);
                        mdisplay.check(1, false);
                        mdisplay.check(2, false);
                        fm.update();
                    });
    mdisplay.append("Page Locations",
                    [&](const std::string &title)
                    {
                        M.DisplayPages();
                        mdisplay.check(0, false);
                        mdisplay.check(1);
                        mdisplay.check(2, false);
                        fm.update();
                    });
    mdisplay.append("Uncovered",
                    [&](const std::string &title)
                    {
                        M.DisplayUncovered();
                        mdisplay.check(0, false);
                        mdisplay.check(1, false);
                        mdisplay.check(2);
                        fm.update();
                    });
    mdisplay.appendSeparator();
    mdisplay.append("Pan left",
                    [&](const std::string &title)
                    {
                        M.panLeft();
                        fm.update();
                    });
    mdisplay.append("Pan right",
                    [&](const std::string &title)
                    {
                        M.panRight();
                        fm.update();
                    });
    mdisplay.append("Pan up",
                    [&](const std::string &title)
                    {
                        M.panUp();
                        fm.update();
                    });
    mdisplay.append("Pan down",
                    [&](const std::string &title)
                    {
                        M.panDown();
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
            M.waypointsDisplay(S);
            M.pageDisplay(S);
            M.uncoveredDisplay(S);
        });

    // handle mouse wheel
    fm.events().mouseWheel(
        [&](int dist)
        {
            if (dist > 0)
                M.incScale();
            else
                M.decScale();
            fm.update();
        });
}
