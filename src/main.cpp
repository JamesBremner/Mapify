

#include "cMapify.h"
#include "cGUI.h"

cxy cPage::thePaper;

cMapify::cMapify()
    : myScale(1.0 / 200.0),
      myXoff(350000),
      myYoff(380000),
      myDisplayTab(eDisplayTab::viz),
      myAlgorithm(eAlgorithm::cluster)
{
}

void cMapify::generateRandom()
{

    paper(10.0, 7.0);
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
    paper(6925, 10000);
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

    scale();
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
    if (!myWayPoints.size())
        return;

    myPages.clear();
    myCovered.clear();
    myCovered.resize(myWayPoints.size(), false);

    int bestlast;
    std::vector<int> bestadded;

    firstPage(bestlast, bestadded);

    for (int p = 0;; p++)
    {
        cPage page = bestAdjacent(
            bestlast,
            bestadded);

        if (!bestadded.size())
        {
            std::cout << "did not find new page to add\n";
            break;
        }

        if (page.center == myPages.back().center)
            break;

        myPages.emplace_back(page);
        for (int i = 0; i < bestadded.size(); i++)
            myCovered[bestadded[i]] = true;

        // reached end of trail
        if (bestlast == myWayPoints.size() - 1)
            break;

        // too many pages
        if (p > 300)
            break;
    }

    // Display fit found on terminal
    std::cout << text();

    clusterUncovered();

    // Display fit found on terminal
    std::cout << text();
}

void cMapify::firstPage(
    int &bestlast,
    std::vector<int> &bestadded)
{
    // locate the first page with the first waypoint at the center
    cPage page(myWayPoints[0]);

    int outCount = 0;
    for (int i = 0; i < myWayPoints.size(); i++)
    {
        if (page.isInside(myWayPoints[i]))
        {
            outCount = 0;
            bestlast = i;
            bestadded.push_back(i);
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
    myPages.push_back(page);
}

cPage cMapify::bestAdjacent(
    int &bestlast,
    std::vector<int> &bestadded)
{
    std::cout << "bestAdjacent " << myPages.size();

    cPage page, bestpage;
    bestadded.clear();
    int last;
    std::vector<int> added;
    int prevlast = bestlast;

    // exit margin of last page closest to last waypoint in page
    auto em = exitMargin(
        myWayPoints[bestlast]);

    std::cout << " exit margin " << (int)em << "\n";

    // best page adjacent to exit margin
    page = bestAdjacent(
        em,
        prevlast,
        last,
        added);

    if (added.size())
    {
        // found page that covers new points
        bestlast = last;
        bestadded = added;
        return page;
    }

    // try other margins
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
        }
    }
    return bestpage;
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

    cPage page, bestpage;
    int c, cmax = 0;
    cxy wpFirst = myWayPoints[prevlast + 1];
    bestadded.clear();

    // loop over pages adjacent to exit margin
    for (double off = 0; off <= 1; off += 0.1)
    {
        cPage next = nextPageLocate(
            myPages.back(),
            off,
            margin);

        // insist that first waypoint is covered
        if (!next.isInside(wpFirst))
            continue;

        // check if page is best found so far

        int last;
        std::vector<int> added;
        c = newPointsInPage(next.center, added, last);
        // std::cout << c << " new points for "
        //           << next.center.x << " " << next.center.y
        //           << "\n";
        if (c > cmax)
        {
            cmax = c;
            bestpage = next;
            newlast = last;
            bestadded = added;
        }

        page.rotated = true;
        next = nextPageLocate(page, off, margin);

        // insist that first waypoint is covered
        if (!next.isInside(wpFirst))
            ;
        continue;

        c = newPointsInPage(next.center, added, last);
        // std::cout << c << " new points for rotated "
        //           << next.center.x << " " << next.center.y
        //           << "\n";
        if (c > cmax)
        {
            cmax = c;
            bestpage = next;
            newlast = last;
            bestadded = added;
        }
    }

    return bestpage;
}

cPage cMapify::nextPageLocate(
    const cPage &page,
    double off,
    eMargin em)
{
    cPage next;
    next.em = em;

    // auto prevpoly = myPaper.polygon(page);

    cxy nextdim = cPage::thePaper;
    if (page.rotated)
    {
        nextdim.x = cPage::thePaper.y;
        nextdim.y = cPage::thePaper.x;
    }
    double start, range;

    switch (em)
    {
    case eMargin::bottom:
        range = 2 * nextdim.x;
        start = page.bottomleft().x - nextdim.x;
        next.center.x = start + off * range + nextdim.x / 2;
        next.center.y = page.bottomright().y + nextdim.y / 2;
        break;
    case eMargin::left:
        range = 2 * nextdim.y;
        start = page.topleft().y - nextdim.y;
        next.center.x = page.topleft().x - nextdim.x / 2;
        next.center.y = start + off * range - nextdim.y / 2;
        break;
    case eMargin::right:
        range = 2 * nextdim.y;
        start = page.topright().y - nextdim.y;
        next.center.x = page.topright().x + nextdim.x / 2;
        next.center.y = start + off * range + nextdim.y / 2;
        break;
    case eMargin::top:
        range = 2 * nextdim.x;
        start = page.topright().x - nextdim.x;
        next.center.x = start + off * range + nextdim.x / 2;
        next.center.y = page.topright().y - nextdim.y / 2;
        break;
    }

    return next;
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
    std::vector<cxy> poly = page.polygon();

    int count = 0;
    for (int pi = 0; pi < myWayPoints.size(); pi++)
    {
        if (!myCovered[pi])
            if (myWayPoints[pi].isInside(poly))
            {
                count++;
                last = pi;
                added.push_back(pi);
            }
    }
    return count;
}

void cMapify::cluster()
{
    if (!myWayPoints.size())
        return;

    K.clearData();
    myPages.clear();

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
    for (int ip = 0; ip < myWayPoints.size(); ip++)
    {
        if (!myCovered[ip])
            K.Add({myWayPoints[ip].x, myWayPoints[ip].y});
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
    if (w<= cPage::thePaper.x &&
        h <=  cPage::thePaper.y)
        return eFit::fit;

    if (w <=  cPage::thePaper.y &&
        h <=  cPage::thePaper.x)
    {
        return eFit::fitrotated;
    }
    return eFit::nofit;
}
void cMapify::clusterUncovered()
{
    for (;;)
    {
        if (!uncoveredCount())
        {
            std::cout << "all waypoints covered\n";
            return;
        }

        setKMeansToUncovered();

        // Init KMeans for 3 clusters
        K.Init(3);

        // Run KMeans
        K.Iter(10);

        // loop over clusters
        for (int c = 0; c < K.clusters().size(); c++)
        {
            // ignore empty clusters
            if (K.clusters()[c].points().size() <= 1)
                continue;

            eFit fit = clusterFit(c);

            if ( fit == eFit::nofit )
                continue;

            // place page on cluster
            myPages.emplace_back(
                K.clusters()[c].center().d[0],
                K.clusters()[c].center().d[1]);
            myPages.back().rotated = ( fit == eFit::fitrotated );

            std::cout << "clustering added page at "
                      << myPages.back().center.x
                      << " " << myPages.back().center.y
                      << "\n";

            // update coverd points
            std::vector<int> added;
            int last;
            newPointsInPage(
                myPages.back(),
                added,
                last);
            for (int ip : added)
                myCovered[ip] = true;
        }
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
    std::vector<bool> included(myWayPoints.size(), false);
    for (auto &page : myPages)
    {
        for (int wi = 0; wi < myWayPoints.size(); wi++)
        {
            if (!included[wi])
                if (page.isInside(myWayPoints[wi]))
                    included[wi] = true;
        }
    }

    std::vector<cxy> missed;
    for (int ii = 0; ii < included.size(); ii++)
    {
        if (!included[ii])
            missed.emplace_back(myWayPoints[ii]);
    }

    std::cout << "missed " << missed.size() << "\n";
    return missed;
}

std::string cMapify::text()
{
    if (!myPages.size())
        return "";

    std::stringstream ss;

    // ss << "Waypoints: ";
    // for (auto &p : myWayPoints)
    //     ss << p.x << " " << p.y << ", ";
    // ss << "\n";

    ss << myPages.size() << " pages of " <<  cPage::thePaper.x
       << " by " << cPage::thePaper.y << "\r\n";

    for (int c = 0; c < myPages.size(); c++)
    {
        ss << "Page " << c + 1
           << " center " << myPages[c].center.x
           << ", " << myPages[c].center.y;
        if (myPages[c].rotated)
            ss << " R";
        ss << "\n";
    }
    return ss.str();
}

void cMapify::scale()
{
    double xmin, xmax, ymin, ymax;
    xmax = ymax = 0;
    xmin = ymin = INT_MAX;
    for (auto &p : myWayPoints)
    {
        if (p.x < xmin)
            xmin = p.x;
        if (p.y < ymin)
            ymin = p.y;
        if (p.x > xmax)
            xmax = p.x;
        if (p.y > ymax)
            ymax = p.y;
    }

    myXoff = xmin;
    myYoff = ymin;

    double xscale = 800 / (xmax - xmin);
    double yscale = 800 / (ymax - ymin);
    myScale = xscale;
    if (yscale < xscale)
        myScale = yscale;
}

void cMapify::waypointsDisplay(wex::shapes &S)
{
    if (!myWayPoints.size())
        return;
    if (myDisplayTab != eDisplayTab::viz)
        return;

    S.color(0x0000FF);
    for (auto &p : myWayPoints)
        S.pixel(
            myScale * (p.x - myXoff),
            myScale * (p.y - myYoff));
}
void cMapify::uncoveredDisplay(wex::shapes &S)
{
    if (myDisplayTab != eDisplayTab::uncovered)
        return;
    S.color(0x0000FF);
    for (int ic = 0; ic < myCovered.size(); ic++)
        if (!myCovered[ic])
            S.pixel(
                myScale * (myWayPoints[ic].x - myXoff),
                myScale * (myWayPoints[ic].y - myYoff));
}
void cMapify::pageDisplay(wex::shapes &S)
{
    S.color(0);

    if (myDisplayTab == eDisplayTab::viz)
    {
        int w = myScale * (cPage::thePaper.x);
        int h = myScale * (cPage::thePaper.y);
        for (int c = 0; c < myPages.size(); c++)
            S.rectangle(
                {(int)(myScale * (myPages[c].center.x - myXoff) - w / 2),
                 (int)(myScale * (myPages[c].center.y - myYoff) - h / 2),
                 w, h});
    }
    if (myDisplayTab == eDisplayTab::page)
    {
        S.text(text(), {10, 10, 1000, 1000});
    }
}

eMargin cMapify::exitMargin(
    const cxy &lastPoint) const
{
    eMargin margin;
    double bestdist = INT_MAX;
    int imbest;

    auto poly = myPages.back().polygon();

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
