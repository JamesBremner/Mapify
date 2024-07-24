class cMapify
{

public:
    cMapify();
    void generateRandom();
    void readWaypoints(const std::string &fname);
    void cluster();
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

private:
    std::vector<cxy> myWayPoints;
    std::pair<double, double> myPaperDim;
    KMeans K;
    std::vector<cxy> myPageCenters;

    double myScale, myXoff, myYoff;
    bool isMaxPaperDimOK();
    std::vector<cxy> missedWaypoints();
    void clusterMissed(const std::vector<cxy> &missed);
    bool isMaxPaperDimOKPass2(std::vector<cxy> &pagesForMissed);
};