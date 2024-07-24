#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <algorithm>

#include <wex.h>
#include "cxy.h"
#include "KMeans.h"

#include "cMapify.h"
#include "cGUI.h"



cMapify::cMapify()
    : myScale(1.0 / 200.0),
      myXoff(350000),
      myYoff(380000)
{
}

void cMapify::generateRandom()
{
    myPaperDim = std::make_pair(10.0, 7.0);
    for (int k = 0; k < 20; k++)
    {
        myWayPoints.emplace_back(
            rand() % 100,
            rand() % 100);
    }
}

void cMapify::readWaypoints(const std::string &fname)
{
    myWayPoints.clear();
    myPaperDim = std::make_pair(6925, 10000);
    std::ifstream ifs(fname);
    if (!ifs.is_open())
        throw std::runtime_error("Cannot open waypoints file");
    std::string line;
    while (getline(ifs, line))
    {
        // std::cout << line << "\n";
        int p = line.find(",");
        if (p == -1)
            throw std::runtime_error("Bad waypoint format");
        myWayPoints.emplace_back(
            atof(line.substr(0, p).c_str()),
            atof(line.substr(p + 1).c_str()));
    }
}

void cMapify::cluster()
{
    if( ! myWayPoints.size() )
    return;

    K.clearData();
    myPageCenters.clear();

    // add waypoints to KMeans class instance K
    for (auto &p : myWayPoints)
    {
        K.Add({p.x, p.y});
    }
    // increment number of pages until fit found
    for (int PageCount = 1; PageCount < 100; PageCount++)
    {
        // Init KMeans
        K.Init(PageCount);

        // Run KMeans
        K.Iter(10);

        // check that every cluster fits into one page
        if (!isMaxPaperDimOK())
        {
            std::cout << "Cannot fit into " << PageCount << " pages\n";

            // continue to increase number of pages
            continue;
        }

        // Display fit found on terminal
        std::cout << text();

        break;
    }
}

bool cMapify::isMaxPaperDimOK()
{
    double minx, miny, maxx, maxy, clusterWidth, clusterHeight;

    myPageCenters.clear();

    int countOversizedClusters = 0;
    int countAcceptOversized = 0.3 * K.clusters().size();

    // loop over clusters
    for (int c = 0; c < K.clusters().size(); c++)
    {
        // ignore empty clusters
        if (K.clusters()[c].points().size() <= 1)
            continue;

        // calculate cluster width and height
        minx = miny = INT_MAX;
        maxx = maxy = -INT_MAX;
        for (auto p : K.clusters()[c].points())
        {
            double x = p->d[0];
            double y = p->d[1];
            if (x < minx)
                minx = x;
            if (x > maxx)
                maxx = x;
            if (y < miny)
                miny = y;
            if (y > maxy)
                maxy = y;
        }
        clusterWidth = maxx - minx;
        clusterHeight = maxy - miny;

        // std::cout << "Page " << c << " min size: "
        //           << clusterWidth << " " << clusterHeight
        //           << " range: " << minx << " " << maxx << " " << miny << " " << maxy
        //           << "\n";

        // If entire cluster will NOT fit inside a single page
        // abandon this solution
        if (clusterWidth > myPaperDim.first ||
            clusterHeight > myPaperDim.second)
        {
            countOversizedClusters++;
        }

        myPageCenters.emplace_back(
            (minx + maxx) / 2, (miny + maxy) / 2);
    }

    if (!countOversizedClusters)
        // every cluster fits inside a single page
        return true;

    if (countOversizedClusters <= countAcceptOversized)
    {
        // accept a few oversized clusters
        auto missed = missedWaypoints();
        std::cout << "oversized " << countOversizedClusters
                  << " missed points " << missed.size()
                  << "\n";

        // add pages to cover the missed waypoints
        clusterMissed(missed);

        return true;
    }

    return false;
}
void cMapify::clusterMissed(const std::vector<cxy> &missed)
{
    // missing waypoints to KMeans class instance K
    K.clearData();
    for (auto &p : missed)
    {
        K.Add({p.x, p.y});
    }

    std::vector<cxy> pagesForMissed;

    // increment number of pages until fit found
    for (int PageCount = 1; PageCount < 100; PageCount++)
    {
        // Init KMeans
        K.Init(PageCount);

        // Run KMeans
        K.Iter(10);

        // check that every cluster fits into one page
        if (!isMaxPaperDimOKPass2(pagesForMissed))
        {
            std::cout << "Cannot fit missing into " << PageCount << " pages\n";

            // continue to increase number of pages
            continue;
        }

        // add extra pages to cover missed waypoints
        myPageCenters.insert(
            myPageCenters.end(),
            pagesForMissed.begin(), pagesForMissed.end());

        return;
    }
}
bool cMapify::isMaxPaperDimOKPass2(std::vector<cxy> &pagesForMissed)
{
    double minx, miny, maxx, maxy, clusterWidth, clusterHeight;

    int countOversizedClusters = 0;
    int countAcceptOversized = 0.5 * K.clusters().size();

    pagesForMissed.clear();

    // loop over clusters
    for (int c = 0; c < K.clusters().size(); c++)
    {
        // ignore empty clusters
        if (K.clusters()[c].points().size() <= 1)
            continue;

        // calculate cluster width and height
        minx = miny = INT_MAX;
        maxx = maxy = -INT_MAX;
        for (auto p : K.clusters()[c].points())
        {
            double x = p->d[0];
            double y = p->d[1];
            if (x < minx)
                minx = x;
            if (x > maxx)
                maxx = x;
            if (y < miny)
                miny = y;
            if (y > maxy)
                maxy = y;
        }
        clusterWidth = maxx - minx;
        clusterHeight = maxy - miny;

        std::cout << "Page " << c << " min size: "
                  << clusterWidth << " " << clusterHeight
                  << " range: " << minx << " " << maxx << " " << miny << " " << maxy
                  << "\n";

        // If entire cluster will NOT fit inside a single page
        // abandon this solution
        if (clusterWidth > myPaperDim.first ||
            clusterHeight > myPaperDim.second)
        {
            return false;
        }

        // place page on cluster
        // myPageCenters.emplace_back(
        //     K.clusters()[c].center().d[0],
        //     K.clusters()[c].center().d[1]);
        pagesForMissed.emplace_back(
            (minx + maxx) / 2, (miny + maxy) / 2);
    }

    return true;
}

std::vector<cxy> cMapify::missedWaypoints()
{

    std::vector<bool> included(myWayPoints.size(), false);
    for (auto &pageCenter : myPageCenters)
    {
        std::vector<cxy> page;
        page.emplace_back(
            pageCenter.x - myPaperDim.first / 2,
            pageCenter.y - myPaperDim.second / 2);
        page.emplace_back(
            pageCenter.x + myPaperDim.first / 2,
            pageCenter.y - myPaperDim.second / 2);
        page.emplace_back(
            pageCenter.x + myPaperDim.first / 2,
            pageCenter.y + myPaperDim.second / 2);
        page.emplace_back(
            pageCenter.x - myPaperDim.first / 2,
            pageCenter.y + myPaperDim.second / 2);

        for (int wi = 0; wi < myWayPoints.size(); wi++)
        {
            if (!included[wi])
                if (myWayPoints[wi].isInside(page))
                    included[wi] = true;
        }
    }

    std::vector<cxy> missed;
    for (int ii = 0; ii < included.size(); ii++)
    {
        if (!included[ii])
            missed.emplace_back(myWayPoints[ii]);
    }

    return missed;
}

std::string cMapify::text()
{
    if( ! myPageCenters.size() )
        return "";

    std::stringstream ss;

    // ss << "Waypoints: ";
    // for (auto &p : myWayPoints)
    //     ss << p.x << " " << p.y << ", ";
    // ss << "\n";

    ss << myPageCenters.size() << " pages of " << myPaperDim.first << " by " << myPaperDim.second << "\r\n";

    // for (int c = 0; c < myPageCount; c++)
    int c = 0;
    {
        ss << "Page " << c + 1
           << " center " << myPageCenters[c].x << ", " << myPageCenters[c].y
           << " " << K.clusters()[c].points().size() << " waypoints:\r\n ";
        for (auto p : K.clusters()[c].points())
        {
            std::cout << p->d[0] << " " << p->d[1] << ", ";
        }
        ss << "\r\n";
    }
    return ss.str();
}

void cMapify::waypointsDisplay(wex::shapes &S)
{
    S.color(0x0000FF);
    for (auto &p : myWayPoints)
        S.pixel(
            myScale * (p.x - myXoff),
            myScale * (p.y - myYoff));
}
void cMapify::pageDisplay(wex::shapes &S)
{
    S.color(0);

    int w = myScale * (myPaperDim.first);
    int h = myScale * (myPaperDim.second);
    for (int c = 0; c < myPageCenters.size(); c++)
        S.rectangle(
            {(int)(myScale * (myPageCenters[c].x - myXoff) - w / 2),
             (int)(myScale * (myPageCenters[c].y - myYoff) - h / 2),
             w, h});
}

cGUI::cGUI()
    : cStarterGUI(
          "Mapify",
          {50, 50, 1000, 1000}),
      myText(wex::maker::make<wex::multiline>(fm))
{

    myText.move(400, 50, 200, 200);
    
    constructMenus();

    fm.events().draw(
        [&](PAINTSTRUCT &ps)
        {
            wex::shapes S(ps);
            M.waypointsDisplay(S);
            M.pageDisplay(S);
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
                     wex::filebox fb(fm);
                     auto fname = fb.open();
                     if (fname.empty())
                         return;
                     M.readWaypoints(fname);
                     M.cluster();
                     myText.text(M.text());
                     fm.update();
                 });
    mbar.append("File", mfile);

    wex::menu mdisplay(fm);
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
    mbar.append("Display", mdisplay);
}

main()
{

    cGUI theGUI;
    return 0;
}
