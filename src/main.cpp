#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <algorithm>
#include <wex.h>
#include "cStarterGUI.h"

#include "cxy.h"
#include "KMeans.h"

class cMapify
{
    std::vector<cxy> myWayPoints;
    std::pair<double, double> myPaperDim;
    KMeans K;
    int myPageCount;
    std::vector<cxy> myPageCenters;

    double myScale, myXoff, myYoff;

public:
    cMapify();
    void generateRandom();
    void readWaypoints(const std::string &fname);
    void cluster();
    bool isMaxPaperDimOK();

    void waypointsDisplay(wex::shapes &S);
    void pageDisplay(wex::shapes &S);
    std::string text();
};

cMapify::cMapify()
    : myScale(1.0 / 50.0),
      myXoff(380000),
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
    // add waypoints to KMeans class instance K
    for (auto &p : myWayPoints)
    {
        K.Add({p.x, p.y});
    }
    // increment number of pages until fit found
    for (myPageCount = 1; myPageCount < 100; myPageCount++)
    {
        // Init KMeans
        K.Init(myPageCount);

        // Run KMeans
        K.Iter(10);

        // check that every cluster fits into one page
        if (!isMaxPaperDimOK())
        {
            std::cout << "Cannot fit into " << myPageCount << " pages\n";

            // continue to increase number of pages
            continue;
        }

        // Display fit found

        std::cout << "\nFits into " << myPageCount
                  << " of " << myPaperDim.first << " by " << myPaperDim.second
                  << " pages\n";

        for (int c = 0; c < myPageCount; c++)
        {
            std::cout << "Page " << c + 1
                      << " center " << K.clusters()[c].center().d[0] << ", " << K.clusters()[c].center().d[1]
                      << " " << K.clusters()[c].points().size() << " waypoints:\n ";
            for (auto p : K.clusters()[c].points())
            {
                std::cout << p->d[0] << " " << p->d[1] << ", ";
            }
            std::cout << "\n";
        }
        break;
    }
}

bool cMapify::isMaxPaperDimOK()
{
    double minx, miny, maxx, maxy, clusterWidth, clusterHeight;

    myPageCenters.clear();

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

        std::cout << "Page " << c << " min size "
                  << clusterWidth << " " << clusterHeight << "\n";

        // If entire cluster will NOT fit inside a single page
        // abandon this solution
        if (clusterWidth > myPaperDim.first ||
            clusterHeight > myPaperDim.second)
            return false;

        // place page on cluster
        myPageCenters.emplace_back(
            K.clusters()[c].center().d[0],
            K.clusters()[c].center().d[1]);
    }
    // every cluster fits inside a single page
    return true;
}

std::string cMapify::text()
{
    std::stringstream ss;

    // ss << "Waypoints: ";
    // for (auto &p : myWayPoints)
    //     ss << p.x << " " << p.y << ", ";
    // ss << "\n";

    ss << myPageCount << " pages of " << myPaperDim.first << " by " << myPaperDim.second << "\r\n";

    for (int c = 0; c < myPageCount; c++)
    {
        ss << "Page " << c + 1
           << " center " << myPageCenters[c].x << ", " << myPageCenters[c].y
           << " " << K.clusters()[c].points().size() << " waypoints:\n ";
        // for (auto p : K.clusters()[c].points())
        // {
        //     std::cout << p->d[0] << " " << p->d[1] << ", ";
        // }
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
    for (int c = 0; c < myPageCount; c++)
        S.rectangle(
            {(int)(myScale * (myPageCenters[c].x - myXoff) - w / 2),
             (int)(myScale * (myPageCenters[c].y - myYoff) - h / 2),
             w, h});
}

class cGUI : public cStarterGUI
{
public:
    cGUI()
        : cStarterGUI(
              "Mapify",
              {50, 50, 1000, 1000}),
          myText(wex::maker::make<wex::multiline>(fm))
    {
        //M.readWaypoints("../dat/test2-0to2000.txt");
        M.readWaypoints("../dat/test2.txt");

        M.cluster();

        myText.move(50, 50, 200, 200);
        myText.text(M.text());

        fm.events().draw(
            [&](PAINTSTRUCT &ps)
            {
                wex::shapes S(ps);
                M.waypointsDisplay(S);
                M.pageDisplay(S);
            });

        //         fm.events().onMouseWheel(int dist )
        //         {}
        // {
        //     clicked = 1;
        // });

        show();
        run();
    }

private:
    cMapify M;

    wex::multiline myText;
};

main()
{

    cGUI theGUI;
    return 0;
}
