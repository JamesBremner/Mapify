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
    eMargin em;   // exit margin from previous page

    cPage() : rotated(false) {}
    cPage(const cxy &c)
        : center(c), rotated(false)
    {
    }

    /// @brief Construct centered at specified point
    /// @param width
    /// @param height

    cPage(double width, double height)
        : center(width, height), rotated(false)
    {
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
    cMapify();
    void addWaypoint(double x, double y)
    {
        myWayPoints.emplace_back(x, y);
    }
    void paper(double w, double h)
    {
        cPage::thePaper.x = w;
        cPage::thePaper.y = h;
    }
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

    int pageCount()
    {
        return myPages.size();
    }

    bool unitTest();

private:
    std::vector<cxy> myWayPoints;
    std::vector<bool> myCovered;
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

    enum class eFit
    {
        nofit,
        fit,
        fitrotated
    };

    double myScale, myXoff, myYoff;

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
        int &bestlast,
        std::vector<int> &bestadded);

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

    cPage bestAdjacent(
        int &bestlast,
        std::vector<int> &bestadded);

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
    /// @return next page

    cPage nextPageLocate(
        const cPage &page,
        double off,
        eMargin em);

    void scale();

    std::vector<cxy> pageOffsets();
};