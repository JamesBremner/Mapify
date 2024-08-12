

#include "cMapify.h"
#include "cGUI.h"

// construct paper with default size
cxy cPage::thePaper(6925, 10000);

std::vector<cPage> cMapify::thePages;
std::vector<cxy> cMapify::theWayPoints;

cMapify::cMapify()
    : myAlgorithm(eAlgorithm::cluster)
{
    thePages.clear();
    theWayPoints.clear();
}

void cMapify::generateRandom()
{
    paper(10.0, 7.0);
    for (int k = 0; k < 20; k++)
    {
        addWaypoint(
            rand() % 100,
            rand() % 100);
    }
}

void cMapify::readWaypoints(const std::string &fname)
{
    theWayPoints.clear();
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
        addWaypoint(
            atof(line.substr(0, p).c_str()),
            atof(line.substr(p + 1).c_str()));
    }
}

void cMapify::calculate()
{
    // switch (myAlgorithm)
    // {
    // case eAlgorithm::cluster:
    //     cluster();
    //     break;
    // case eAlgorithm::greedy:
    //     greedy();
    //     break;
    // }
    adjacentThenCluster();
}

// void cMapify::greedy()
// {
//     if (!myWayPoints.size())
//         return;

//     myPages.clear();

//     std::vector<bool> covered(myWayPoints.size(), false);

//     // calc offsets from waypoint to paper center
//     // to position the paper so the waypoint is on margin

//     auto voff = pageOffsets();

//     auto &wpFirstInPage = myWayPoints[0];

//     cxy bestpage;
//     int bestlast;
//     std::vector<int> bestadded;
//     for (int p = 0;; p++)
//     {

//         bestpage = bestPageLocation(
//             wpFirstInPage,
//             voff,
//             covered,
//             bestlast,
//             bestadded);

//         // check if last waypoint has been included
//         if (bestlast == myWayPoints.size() - 1)
//         {
//             myPages.emplace_back(bestpage);
//             covered.insert(
//                 covered.end(),
//                 bestadded.begin(), bestadded.end());

//             // add pages to cover missed waypoints
//             // clusterMissed(missedWaypoints());
//             // missedWaypoints();

//             return;
//         }

//         if (p > 300)
//             return;

//         if (!myPages.size())
//         {
//             // always add first page
//             myPages.emplace_back(bestpage);
//             covered.insert(
//                 covered.end(),
//                 bestadded.begin(), bestadded.end());
//         }
//         else
//         {
//             // add page if different enough from previous
//             if (myPages.back().center.dist2(bestpage) > 5)
//             {
//                 myPages.emplace_back(bestpage);
//                 covered.insert(
//                     covered.end(),
//                     bestadded.begin(), bestadded.end());
//                 std::cout << " add\n";
//             }
//             else
//             {
//                 std::cout << " skip ";
//             }

//             wpFirstInPage = myWayPoints[bestlast + 1];
//         }
//     }
// }

void cMapify::adjacentThenCluster()
{
    // check for loaded waypoints
    if (!theWayPoints.size())
        return;

    // add first page
    firstPage();

    // add adjacent pages until end of trail
    addAdjacent();

    // Display fit found on terminal
    std::cout << text();

    // add pages on missed waypoints
    clusterUncovered();

    // Display fit found on terminal
    std::cout << text();
}

void cMapify::firstPage()
{
    // clear previous solution
    thePages.clear();
    myCovered.clear();
    myCovered.resize(theWayPoints.size(), false);

    // locate the first page with the first waypoint at the center
    cPage page(theWayPoints[0]);

    int outCount = 0;
    for (int i = 0; i < theWayPoints.size(); i++)
    {
        if (page.isInside(theWayPoints[i]))
        {
            outCount = 0;
            page.lastCovered = i;
            myCovered[i] = true;
        }
        else
        {
            outCount++;
            if (outCount > 200)
            {

                // for circular routes
                // the first page will cover the first few points
                // AND the last few points
                // the last 200 points were uncovered
                // so we assume we have counted all the first points
                break;
            }
        }
    }

    // always add first page
    thePages.push_back(page);
}

void cMapify::addAdjacent()
{
    // std::cout << "bestAdjacent " << thePages.size();

    while (true)
    {
        if (thePages.back().lastCovered == theWayPoints.size() - 1)
        {
            std::cout << "reached end of trail\n";
            return;
        }
        if (thePages.size() > 300)
        {
            std::cout << "too many pages\n";
            return;
        }

        cPage page, bestpage;
        std::vector<int> bestadded;
        int last, bestlast;
        std::vector<int> added;
        int prevlast = thePages.back().lastCovered;

        // exit margin of last page closest to last waypoint in page
        auto em = exitMargin(
            theWayPoints[prevlast]);

        std::cout << " exit margin " << (int)em << "\n";

        // best page adjacent to exit margin
        bestpage = bestAdjacent(
            em,
            prevlast,
            bestlast,
            bestadded);
        bestpage.lastCovered = bestlast;

        if (!bestadded.size())
        {
            // try other margins
            bestadded.clear();
            for (int iem2 = 0; iem2 < 4; iem2++)
            {
                if (iem2 == (int)em)
                    continue;

                page = bestAdjacent(
                    (eMargin)iem2,
                    prevlast,
                    last,
                    added);

                if (added.size() > bestadded.size())
                {
                    bestadded = added;
                    bestlast = last;
                    bestpage = page;
                    bestpage.lastCovered = last;
                }
            }
        }

        // check good page found
        if (!bestadded.size())
        {
            std::cout << "did not find new page to add\n";
            return;
        }
        if (bestpage.center == thePages.back().center)
        {
            std::cout << "did not find new page to add\n";
            return;
        }

        // add page
        thePages.emplace_back(bestpage);
        for (int i = 0; i < bestadded.size(); i++)
            myCovered[bestadded[i]] = true;
    }
}

cPage cMapify::bestAdjacent(
    eMargin margin,
    int prevlast,
    int &newlast,
    std::vector<int> &bestadded)
{
    // std::cout << "bestAdjacent " << myPages.size()
    // << " margin " << (int) margin
    // << " prevlast " << prevlast
    // << "\n";

    int last;
    std::vector<int> added;
    cPage page, bestpage;
    int c, cmax = 0;
    cxy wpFirst = theWayPoints[prevlast + 1];
    bestadded.clear();

    // loop over pages adjacent to exit margin
    for (double off = 0; off <= 1; off += 0.1)
    {
        cPage next = nextPageLocate(
            thePages.back(),
            off,
            margin,
            false); // not rotated

        // insist that first waypoint is covered
        if (next.isInside(wpFirst))
        {
            // check if page is best found so far

            c = newPointsInPage(next, added, last);
            // std::cout << c << " new points for "
            //           << next.center.x << " " << next.center.y
            //           << "\n";
            if (c > cmax)
            {
                cmax = c;
                bestpage = next;
                bestpage.lastCovered = last;
                newlast = last;
                bestadded = added;
            }
        }

        next = nextPageLocate(
            thePages.back(),
            off,
            margin,
            true); // rotated

        // insist that first waypoint is covered
        if (next.isInside(wpFirst))
        {
            c = newPointsInPage(next, added, last);
            // std::cout << c << " new points for rotated "
            //           << next.center.x << " " << next.center.y
            //           << "\n";
            if (c > cmax)
            {
                cmax = c;
                bestpage = next;
                bestpage.lastCovered = last;
                newlast = last;
                bestadded = added;
            }
        }
    }

    return bestpage;
}

cPage cMapify::nextPageLocate(
    const cPage &prevpage,
    double off,
    eMargin em,
    bool rotated)
{
    cPage nextpage;
    nextpage.rotated = rotated;
    cxy nextdim = cPage::thePaper;
    if (rotated)
    {
        nextdim.x = cPage::thePaper.y;
        nextdim.y = cPage::thePaper.x;
    }
    double start, range;

    switch (em)
    {
    case eMargin::bottom:
        range = 2 * nextdim.x;
        start = prevpage.bottomleft().x - nextdim.x;
        nextpage.center.x = start + off * range + nextdim.x / 2;
        nextpage.center.y = prevpage.bottomright().y + nextdim.y / 2;
        break;
    case eMargin::left:
        range = 2 * nextdim.y;
        start = prevpage.topleft().y - nextdim.y;
        nextpage.center.x = prevpage.topleft().x - nextdim.x / 2;
        nextpage.center.y = start + off * range - nextdim.y / 2;
        break;
    case eMargin::right:
        range = 2 * nextdim.y;
        start = prevpage.topright().y - nextdim.y;
        nextpage.center.x = prevpage.topright().x + nextdim.x / 2;
        nextpage.center.y = start + off * range + nextdim.y / 2;
        break;
    case eMargin::top:
        range = 2 * nextdim.x;
        start = prevpage.topright().x - nextdim.x;
        nextpage.center.x = start + off * range + nextdim.x / 2;
        nextpage.center.y = prevpage.topright().y - nextdim.y / 2;
        break;
    }

    return nextpage;
}

cxy cMapify::bestPageLocation(
    const cxy &wpFirstInPage,
    const std::vector<cxy> &voff,
    std::vector<bool> &covered,
    int &bestlast,
    std::vector<int> &bestadded)
{
    cxy page, bestpage;
    int c, cmax = 0;
    int last;
    std::vector<int> added;
    for (const cxy &off : voff)
    {
        page.x = wpFirstInPage.x + off.x;
        page.y = wpFirstInPage.y + off.y;
        c = newPointsInPage(page, added, last);
        // std::cout << page.x <<" "<< page.y <<" "<< c <<" "<< last
        //     << "\n";
        if (c > cmax)
        {
            cmax = c;
            bestpage = page;
            bestlast = last;
            bestadded = added;
        }
    }

    // std::cout << "last " << bestlast << "\n";

    return bestpage;
}

int cMapify::newPointsInPage(
    const cPage &page,
    std::vector<int> &added,
    int &last)
{
    added.clear();
    std::vector<cxy> poly = page.polygon();

    int count = 0;
    for (int pi = 0; pi < theWayPoints.size(); pi++)
    {
        if (!myCovered[pi])
            if (theWayPoints[pi].isInside(poly))
            {
                count++;
                last = pi;
                added.push_back(pi);
            }
    }
    // for( auto& p : added )
    //     std::cout << "( "<<theWayPoints[p].x<<" "<< theWayPoints[p].y << " ), ";

    return count;
}

void cMapify::cluster()
{
    if (!theWayPoints.size())
        return;

    K.clearData();
    thePages.clear();

    // add waypoints to KMeans class instance K
    for (auto &p : theWayPoints)
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

int cMapify::uncoveredCount()
{
    int count = 0;
    for (bool c : myCovered)
        if (!c)
            count++;
    return count;
}

void cMapify::setKMeansToUncovered()
{
    K.clearData();
    for (int ip = 0; ip < theWayPoints.size(); ip++)
    {
        if (!myCovered[ip])
            K.Add({theWayPoints[ip].x, theWayPoints[ip].y});
    }
}
cMapify::eFit cMapify::clusterFit(int clusterIndex)
{
    // calculate cluster width and height
    double minx, miny, maxx, maxy;
    minx = miny = INT_MAX;
    maxx = maxy = -INT_MAX;
    for (auto p : K.clusters()[clusterIndex].points())
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

    // check that cluster fits inside page
    double w = maxx - minx;
    double h = maxy - miny;
    if (w <= cPage::thePaper.x &&
        h <= cPage::thePaper.y)
        return eFit::fit;

    if (w <= cPage::thePaper.y &&
        h <= cPage::thePaper.x)
    {
        return eFit::fitrotated;
    }
    return eFit::nofit;
}
void cMapify::clusterUncovered()
{
    int missedcount = uncoveredCount();
    if (!missedcount)
    {
        std::cout << "all waypoints covered\n";
        return;
    }
    int maxClusterCount = 3;
    if (maxClusterCount > missedcount)
        maxClusterCount = missedcount;

    for (;;)
    {
        if (!uncoveredCount())
        {
            std::cout << "all waypoints covered\n";
            return;
        }

        setKMeansToUncovered();

        // Init KMeans
        K.Init(maxClusterCount);

        // Run KMeans
        K.Iter(10);

        // loop over clusters
        bool fpageAdded = false;
        for (int c = 0; c < K.clusters().size(); c++)
        {
            // ignore empty clusters
            if (K.clusters()[c].points().size() <= 1)
                continue;

            eFit fit = clusterFit(c);

            if (fit == eFit::nofit)
                continue;

            // place page on cluster
            thePages.emplace_back(
                K.clusters()[c].center().d[0],
                K.clusters()[c].center().d[1]);
            thePages.back().rotated = (fit == eFit::fitrotated);
            fpageAdded = true;
            std::cout << "clustering added page at "
                      << thePages.back().center.x
                      << " " << thePages.back().center.y
                      << "\n";

            // update covered points
            std::vector<int> added;
            int last;
            newPointsInPage(
                thePages.back(),
                added,
                last);
            for (int ip : added)
                myCovered[ip] = true;
        }
        if (!fpageAdded)
            maxClusterCount++;
        else if (maxClusterCount > 3)
            maxClusterCount = 3;
    }
}

bool cMapify::isMaxPaperDimOK()
{
    // double minx, miny, maxx, maxy, clusterWidth, clusterHeight;

    // myPages.clear();

    // int countOversizedClusters = 0;
    // int countAcceptOversized = 0.3 * K.clusters().size();

    // // loop over clusters
    // for (int c = 0; c < K.clusters().size(); c++)
    // {
    //     // ignore empty clusters
    //     if (K.clusters()[c].points().size() <= 1)
    //         continue;

    //     // calculate cluster width and height
    //     minx = miny = INT_MAX;
    //     maxx = maxy = -INT_MAX;
    //     for (auto p : K.clusters()[c].points())
    //     {
    //         double x = p->d[0];
    //         double y = p->d[1];
    //         if (x < minx)
    //             minx = x;
    //         if (x > maxx)
    //             maxx = x;
    //         if (y < miny)
    //             miny = y;
    //         if (y > maxy)
    //             maxy = y;
    //     }
    //     clusterWidth = maxx - minx;
    //     clusterHeight = maxy - miny;

    //     // std::cout << "Page " << c << " min size: "
    //     //           << clusterWidth << " " << clusterHeight
    //     //           << " range: " << minx << " " << maxx << " " << miny << " " << maxy
    //     //           << "\n";

    //     // If entire cluster will NOT fit inside a single page
    //     // abandon this solution
    //     if (clusterWidth > myPaper.dim.x ||
    //         clusterHeight > myPaper.dim.y)
    //     {
    //         countOversizedClusters++;
    //     }

    //     myPages.emplace_back(
    //         (minx + maxx) / 2, (miny + maxy) / 2);
    // }

    // if (!countOversizedClusters)
    //     // every cluster fits inside a single page
    //     return true;

    // if (countOversizedClusters <= countAcceptOversized)
    // {
    //     // accept a few oversized clusters
    //     auto missed = missedWaypoints();
    //     std::cout << "oversized " << countOversizedClusters
    //               << " missed points " << missed.size()
    //               << "\n";

    //     // add pages to cover the missed waypoints
    //     clusterMissed(missed);

    //     return true;
    // }

    return false;
}

bool cMapify::isMaxPaperDimOKPass2(std::vector<cPage> &pagesForMissed)
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
                  << " pts " << K.clusters()[c].points().size()
                  << " range: " << minx << " " << maxx << " " << miny << " " << maxy
                  << "\n";

        // check that cluster fits inside page
        if (clusterWidth <= cPage::thePaper.x &&
            clusterHeight <= cPage::thePaper.y)
        {
            // place page on cluster
            pagesForMissed.emplace_back(
                (minx + maxx) / 2, (miny + maxy) / 2);
            continue;
        }
        if (clusterWidth <= cPage::thePaper.y &&
            clusterHeight <= cPage::thePaper.x)
        {
            // place page on cluster
            pagesForMissed.emplace_back(
                (minx + maxx) / 2, (miny + maxy) / 2);
            pagesForMissed.back().rotated = true;
            continue;
        }
        return false;
    }

    return true;
}

std::vector<cxy> cMapify::missedWaypoints()
{
    std::vector<bool> included(theWayPoints.size(), false);
    for (auto &page : thePages)
    {
        for (int wi = 0; wi < theWayPoints.size(); wi++)
        {
            if (!included[wi])
                if (page.isInside(theWayPoints[wi]))
                    included[wi] = true;
        }
    }

    std::vector<cxy> missed;
    for (int ii = 0; ii < included.size(); ii++)
    {
        if (!included[ii])
            missed.emplace_back(theWayPoints[ii]);
    }

    std::cout << "missed " << missed.size() << "\n";
    return missed;
}

std::string cMapify::text()
{
    if (!thePages.size())
        return "";

    std::stringstream ss;

    // ss << "Waypoints: ";
    // for (auto &p : myWayPoints)
    //     ss << p.x << " " << p.y << ", ";
    // ss << "\n";

    ss << thePages.size() << " pages of " << cPage::thePaper.x
       << " by " << cPage::thePaper.y << "\r\n";

    for (int c = 0; c < thePages.size(); c++)
    {
        ss << "Page " << c + 1
           << " center " << thePages[c].center.x
           << ", " << thePages[c].center.y;
        if (thePages[c].rotated)
            ss << " R";
        ss << "\n";
    }
    return ss.str();
}

eMargin cMapify::exitMargin(
    const cxy &lastPoint) const
{
    eMargin margin;
    double bestdist = INT_MAX;
    int imbest;

    auto poly = thePages.back().polygon();

    for (int im = 0; im < 4; im++)
    {
        int im2 = im + 1;
        if (im2 > 3)
            im2 = 0;
        double d2 = lastPoint.dis2toline(poly[im], poly[im2]);
        if (d2 < bestdist)
        {
            bestdist = d2;
            imbest = im;
        }
    }
    eMargin dbg = (eMargin)imbest;
    return (eMargin)imbest;
}

std::vector<cxy> cPage::polygon() const
{
    std::vector<cxy> poly(4);
    cxy rotdim2;
    if (rotated)
    {
        rotdim2.x = cPage::thePaper.y / 2;
        rotdim2.y = cPage::thePaper.x / 2;
    }
    else
    {
        rotdim2.x = cPage::thePaper.x / 2;
        rotdim2.y = cPage::thePaper.y / 2;
    }
    poly[0] = cxy(center.x - rotdim2.x, center.y - rotdim2.y);
    poly[1] = cxy(center.x + rotdim2.x, center.y - rotdim2.y);
    poly[2] = cxy(center.x + rotdim2.x, center.y + rotdim2.y);
    poly[3] = cxy(center.x - rotdim2.x, center.y + rotdim2.y);

    return poly;
}

main()
{

    cGUI theGUI;
    return 0;
}
