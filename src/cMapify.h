#pragma once

class cPaper
{
public:
    cxy dim;
    std::vector<cxy> cornerOffsets;
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
    void corners();
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
    cPaper myPaper;
    KMeans K;
    std::vector<cxy> myPageCenters;
    enum class eDisplayTab
    {
        viz,
        page
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
    void trailer();
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
    cxy bestPageLocation(
        std::vector<bool> &covered,
        int &bestlast,
        std::vector<int> &bestadded);
    eMargin exitMargin(
        const cxy &lastPage,
        const cxy &lastPoint) const;
    std::vector<cxy> pageOffsets();
};