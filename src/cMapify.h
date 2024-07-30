#pragma once
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <algorithm>
#include <limits>

#include <wex.h>
#include <cxy.h>
#include "KMeans.h"
class cPaper
{
public:
    cxy dim;
    
    void set(double width, double height)
    {
        dim.x = width;
        dim.y = height;
        corners();
    }
    /// @brief Locate page corners in map co-ords
    /// @param center page center location in map co-ords
    /// @return page corners in map co-ords

    std::vector<cxy> polygon(const cxy &center) const
    {
        std::vector<cxy> poly;
        for (const auto &c : cornerOffsets)
            poly.emplace_back(center.x + c.x, center.y + c.y);
        return poly;
    }

private:
    std::vector<cxy> cornerOffsets;

    void corners();
};

class cPage {
public:
    cxy center;
    bool rotated;

    cPage( const cxy& c)
    : center( c )
    {}
    cPage(double width, double height)
    : center( width,height)
    {}
};

class cMapify
{

public:
    cMapify();
    void generateRandom();
    void readWaypoints(const std::string &fname);
    void calculate();
    void waypointsDisplay(wex::shapes &S);
    void pageDisplay(wex::shapes &S);
    void uncoveredDisplay(wex::shapes &S);
    std::string text();

    void incScale()
    {
        myScale *= 1.2;
    }
    void decScale()
    {
        myScale *= 0.8;
    }
    void panUp()
    {
        myYoff += 0.1 * myYoff;
    }
    void panDown()
    {
        myYoff -= 0.1 * myYoff;
    }
    void panLeft()
    {
        myXoff += 0.1 * myXoff;
    }
    void panRight()
    {
        myXoff -= 0.1 * myXoff;
    }
    void DisplayViz()
    {
        myDisplayTab = eDisplayTab::viz;
    }
    void DisplayPages()
    {
        myDisplayTab = eDisplayTab::page;
    }
    void DisplayUncovered()
    {
        myDisplayTab = eDisplayTab::uncovered;
    }
    bool isDisplayPages() const
    {
        return myDisplayTab == eDisplayTab::page;
    }
    void algoCluster()
    {
        myAlgorithm = eAlgorithm::cluster;
    }
    void algoGreedy()
    {
        myAlgorithm = eAlgorithm::greedy;
    }

    bool unitTest();

private:
    std::vector<cxy> myWayPoints;
    std::vector<bool> myCovered;
    cPaper myPaper;
    KMeans K;
    std::vector<cPage> myPages;
    enum class eDisplayTab
    {
        viz,
        page,
        uncovered
    };
    eDisplayTab myDisplayTab;
    enum class eAlgorithm
    {
        cluster,
        greedy
    };
    eAlgorithm myAlgorithm;

    double myScale, myXoff, myYoff;

    void cluster();
    void greedy();
    void adjacentThenCluster();
    bool isMaxPaperDimOK();
    std::vector<cxy> missedWaypoints();
    void clusterMissed(const std::vector<cxy> &missed);
    bool isMaxPaperDimOKPass2(std::vector<cxy> &pagesForMissed);
    int NewPointsInPage(
        const cxy &page,
        const std::vector<bool> &covered,
        std::vector<int> &added,
        int &last);
    cxy bestPageLocation(
        const cxy &wpFirstInPage,
        const std::vector<cxy> &voff,
        std::vector<bool> &covered,
        int &bestlast,
        std::vector<int> &bestadded);

    enum class eMargin
    {
        top,
        right,
        bottom,
        left
    };
    cxy bestAdjacent(
        std::vector<bool> &covered,
        int &bestlast,
        std::vector<int> &bestadded);
    eMargin exitMargin(
        const cxy &lastPoint) const;
    int uncoveredCount();
    void clusterUncovered();
    
    std::vector<cxy> pageOffsets();

};