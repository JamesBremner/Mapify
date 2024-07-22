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

public:
    void generateRandom();
    void readWaypoints(const std::string &fname);
    void cluster();
    bool isMaxPaperDimOK();
    std::string text() const;
};

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
    std::ifstream ifs( fname );
    if( ! ifs.is_open() )
        throw std::runtime_error("Cannot open waypoints file");
    std::string line;
    while( getline( ifs,line)) {
        //std::cout << line << "\n";
        int p = line.find(",");
        if( p == -1 )
            throw std::runtime_error("Bad waypoint format");
        myWayPoints.emplace_back( 
            atof(line.substr(0,p).c_str()),
            atof(line.substr(p+1).c_str() ));
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
    for (int pageCount = 1; pageCount < 100; pageCount++)
    {
        // Init KMeans
        K.Init(pageCount);

        // Run KMeans
        K.Iter(10);

        // check that every cluster fits into one page
        if (!isMaxPaperDimOK())
        {
            std::cout << "Cannot fit into " << pageCount << " pages\n";

            // continue to increase number of pages
            continue;
        }

        // Display fit found

        std::cout << "\nFits into " << pageCount
                  << " of " << myPaperDim.first << " by " << myPaperDim.second
                  << " pages\n";

        for (int c = 0; c < pageCount; c++)
        {
            std::cout << "Page " << c + 1
                      << " center " << K.clusters()[c].center().d[0] << ", " << K.clusters()[c].center().d[1];
                    //   << " waypoints: ";
            // for (auto p : K.clusters()[c].points())
            // {
            //     std::cout << p->d[0] << " " << p->d[1] << ", ";
            // }
            std::cout << "\n";
        }
        break;
    }
}

bool cMapify::isMaxPaperDimOK()
{
    double minx, miny, maxx, maxy;

    for (int c = 0; c < K.clusters().size(); c++)
    {
        if (K.clusters()[c].points().size() <= 1)
            continue;
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
        // std::cout << "Min page size "
        //           << maxx - minx << " " << maxy - miny << "\n";
        if (maxx - minx > myPaperDim.first &&
            maxy - miny > myPaperDim.second)
            return false;
    }
    // no cluster exceeded the page size
    return true;
}

std::string cMapify::text() const
{
    std::stringstream ss;
    ss << "Waypoints: ";
    for (auto &p : myWayPoints)
        ss << p.x << " " << p.y << ", ";
    ss << "\n";
    return ss.str();
}

class cGUI : public cStarterGUI
{
public:
    cGUI()
        : cStarterGUI(
              "Starter",
              {50, 50, 1000, 500}),
          lb(wex::maker::make<wex::label>(fm))
    {
        lb.move(50, 50, 100, 30);
        lb.text("Hello World");

        show();
        run();
    }

private:
    wex::label &lb;
};

main()
{
    cMapify M;
    //M.generateRandom();
    M.readWaypoints("../dat/test2.txt");
    std::cout << M.text();
    M.cluster();

    // cGUI theGUI;
    return 0;
}
