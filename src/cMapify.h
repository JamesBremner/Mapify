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

class cPage;

enum class eMargin
{
    top,
    right,
    bottom,
    left
};

class cPage
{
public:
    static cxy thePaper;
    cxy center;   // page center location
    bool rotated; // true if page rotated 90o from input paper dimensions
    int lastCovered;

    cPage()
        : rotated(false),
          center(-INT_MAX,-INT_MAX)
    {
    }
    cPage(const cxy &c)
        : cPage()
    {
        center = c;
    }

    /// @brief Construct centered at specified point
    /// @param width
    /// @param height

    cPage(double width, double height)
        : cPage()
    {
        center = cxy(width, height);
    }

    cxy topleft() const
    {
        return polygon()[0];
    }
    cxy topright() const
    {
        return polygon()[1];
    }
    cxy bottomright() const
    {
        return polygon()[2];
    }
    cxy bottomleft() const
    {
        return polygon()[3];
    }

    /// @brief Is point covered by page
    /// @param p point
    /// @return true if point covered

    bool isInside(const cxy &p) const
    {
        return p.isInside(polygon());
    }

    std::vector<cxy> polygon() const;
};

class cMapify
{

public:
    static std::vector<cxy> theWayPoints;
    static std::vector<cPage> thePages;

    cMapify();
    void addWaypoint(double x, double y)
    {
        theWayPoints.emplace_back(x, y);
    }
    void paper(double w, double h)
    {
        cPage::thePaper.x = w;
        cPage::thePaper.y = h;
    }
    void generateRandom();
    void readWaypoints(const std::string &fname);
    void calculate();

    void uncoveredDisplay(wex::shapes &S);
    std::string text();

    void algoCluster()
    {
        myAlgorithm = eAlgorithm::cluster;
    }
    void algoGreedy()
    {
        myAlgorithm = eAlgorithm::greedy;
    }

    int pageCount() const
    {
        return thePages.size();
    }

    bool unitTest();

private:
    std::vector<bool> myCovered;
    KMeans K;

    enum class eAlgorithm
    {
        cluster,
        greedy
    };
    eAlgorithm myAlgorithm;

    enum class eFit
    {
        nofit,
        fit,
        fitrotated
    };

    void cluster();
    void greedy();
    void adjacentThenCluster();
    bool isMaxPaperDimOK();
    std::vector<cxy> missedWaypoints();
    void clusterMissed(const std::vector<cxy> &missed);
    bool isMaxPaperDimOKPass2(std::vector<cPage> &pagesForMissed);

    /// @brief locate first page
    /// @param bestlast
    /// @param bestadded

    void firstPage(
        int &bestlast);

    int newPointsInPage(
        const cPage &page,
        std::vector<int> &added,
        int &last);

    cxy bestPageLocation(
        const cxy &wpFirstInPage,
        const std::vector<cxy> &voff,
        std::vector<bool> &covered,
        int &bestlast,
        std::vector<int> &bestadded);

    /// @brief  find best page adjacent to last page
    /// @param bestlast  the last waypoint covered by last page
    /// @param bestadded waypoint indices of new points covered by last page
    /// @return adjacent page

    bool bestAdjacent(
        int &bestlast);

    /// @brief find best page adjacent last along a margin
    /// @param[in] margin margin of prev page
    /// @param[in] prevlast last waypoint in previous page
    /// @param[out] bestlast last waypoint in new page
    /// @param[out] bestadded additiona waypoints covered by be page
    /// @return new page

    cPage bestAdjacent(
        eMargin margin,
        int prevlast,
        int &bestlast,
        std::vector<int> &bestadded);

    eMargin exitMargin(
        const cxy &lastPoint) const;
    int uncoveredCount();
    void clusterUncovered();
    void setKMeansToUncovered();

    eFit clusterFit(int clusterIndex);

    /// @brief locate next page
    /// @param page previous page
    /// @param off fraction of possible location range to apply
    /// @param em prev page margin next page is adjacent to
    /// @param rotated true if next page is to be rotated
    /// @return next page

    cPage nextPageLocate(
        const cPage &page,
        double off,
        eMargin em,
        bool rotated);

    std::vector<cxy> pageOffsets();
};