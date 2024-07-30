

#include "cMapify.h"
#include "cGUI.h"

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
    // myPaperDim = std::make_pair(10.0, 7.0);
    myPaper.set(10.0, 7.0);
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
    myPaper.set(6925, 10000);
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
void cMapify::greedy()
{
    if (!myWayPoints.size())
        return;

    myPages.clear();

    std::vector<bool> covered(myWayPoints.size(), false);

    // calc offsets from waypoint to paper center
    // to position the paper so the waypoint is on margin

    auto voff = pageOffsets();

    auto &wpFirstInPage = myWayPoints[0];

    cxy bestpage;
    int bestlast;
    std::vector<int> bestadded;
    for (int p = 0;; p++)
    {

        bestpage = bestPageLocation(
            wpFirstInPage,
            voff,
            covered,
            bestlast,
            bestadded);

        // check if last waypoint has been included
        if (bestlast == myWayPoints.size() - 1)
        {
            myPages.emplace_back(bestpage);
            covered.insert(
                covered.end(),
                bestadded.begin(), bestadded.end());

            // add pages to cover missed waypoints
            // clusterMissed(missedWaypoints());
            // missedWaypoints();

            return;
        }

        if (p > 300)
            return;

        if (!myPages.size())
        {
            // always add first page
            myPages.emplace_back(bestpage);
            covered.insert(
                covered.end(),
                bestadded.begin(), bestadded.end());
        }
        else
        {
            // add page if different enough from previous
            if (myPages.back().center.dist2(bestpage) > 5)
            {
                myPages.emplace_back(bestpage);
                covered.insert(
                    covered.end(),
                    bestadded.begin(), bestadded.end());
                std::cout << " add\n";
            }
            else
            {
                std::cout << " skip ";
            }

            wpFirstInPage = myWayPoints[bestlast + 1];
        }
    }
}

void cMapify::adjacentThenCluster()
{
    if (!myWayPoints.size())
        return;

    myPages.clear();
    myCovered.clear();
    myCovered.resize(myWayPoints.size(), false);

    // calc offsets from waypoint to paper center
    // to position the paper so the waypoint is on margin

    auto voff = pageOffsets();

    cxy bestpage;
    int bestlast;
    std::vector<int> bestadded;

    bestpage = bestPageLocation(
        myWayPoints[0],
        voff,
        myCovered,
        bestlast,
        bestadded);

    // always add first page
    myPages.emplace_back(bestpage);
    for (int i = 0; i < bestadded.size(); i++)
        myCovered[i] = true;

    for (int p = 0;; p++)
    {
        cxy page = bestAdjacent(
            myCovered,
            bestlast,
            bestadded);

        if (!bestadded.size())
            break;

        if (page == myPages.back().center)
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

     clusterUncovered();
}

cxy cMapify::bestAdjacent(
    std::vector<bool> &covered,
    int &bestlast,
    std::vector<int> &bestadded)
{
    cxy page, bestpage;
    int c, cmax = 0;
    cxy wpFirst = myWayPoints[bestlast + 1];
    bestadded.clear();

    // margin of last page closest to last waypoint in page
    auto em = exitMargin(
        myWayPoints[bestlast]);

    auto prevpoly = myPaper.polygon(myPages.back().center);
    cxy emp1 = prevpoly[(int)em];
    int m2 = ((int)em) + 1;
    if (m2 > 3)
        m2 = 0;
    cxy emp2 = prevpoly[m2];

    // loop over pages adjacent to exit margin
    for (double off = 0; off <= 1; off += 0.2)
    {
        switch (em)
        {
        case eMargin::bottom:
            page.x = emp1.x + off * (emp2.x - emp1.x);
            page.y = emp1.y + myPaper.dim.y / 2;
            break;
        case eMargin::left:
            page.x = emp1.x - myPaper.dim.x / 2;
            page.y = emp1.y + off * (emp2.y - emp1.y);
            break;
        case eMargin::right:
            page.x = emp1.x + myPaper.dim.x / 2;
            page.y = emp1.y + off * (emp2.y - emp1.y);
            break;
        case eMargin::top:
            page.x = emp1.x + off * (emp2.x - emp1.x);
            page.y = emp1.y - myPaper.dim.y / 2;
            break;
        }
        // insist that first waypoint is covered
        auto testpoly = myPaper.polygon(page);
        if (!wpFirst.isInside(testpoly))
            continue;

        // check if page is best found so far

        int last;
        std::vector<int> added;
        c = NewPointsInPage(page, covered, added, last);
        if (c > cmax)
        {
            cmax = c;
            bestpage = page;
            bestlast = last;
            bestadded = added;
        }
    }
    return bestpage;
}

std::vector<cxy> cMapify::pageOffsets()
{
    std::vector<cxy> voff;

    double top = myPaper.dim.y / 2;
    double bottom = -top;
    double left = myPaper.dim.x / 2;
    double right = -left;

    voff.emplace_back(left, top);
    voff.emplace_back(0, top);
    voff.emplace_back(right, top);

    voff.emplace_back(right, 0);
    voff.emplace_back(right, bottom);

    // voff.emplace_back(0,bottom);
    // voff.emplace_back(left,bottom);

    // voff.emplace_back(left,0);

    return voff;
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
        c = NewPointsInPage(page, covered, added, last);
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

int cMapify::NewPointsInPage(
    const cxy &page,
    const std::vector<bool> &covered,
    std::vector<int> &added,
    int &last)
{
    std::vector<cxy> poly = myPaper.polygon(page);

    int count = 0;
    for (int pi = 0; pi < myWayPoints.size(); pi++)
    {
        if (!covered[pi])
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
    for( bool c : myCovered )
        if( ! c )
            count++;
    return count;
}
void cMapify::clusterUncovered()
{
    if( ! uncoveredCount())
    {
        std::cout << "all waypoints covered\n";
        return;
    }

    // add uncovered waypoints to KMeans class instance K
    K.clearData();
    for (int ip = 0; ip < myWayPoints.size(); ip++)
    {
        if (!myCovered[ip])
            K.Add({myWayPoints[ip].x, myWayPoints[ip].y});
    }
    // increment number of pages until fit found
    for (int PageCount = 1; PageCount < 100; PageCount++)
    {
        // Init KMeans
        K.Init(PageCount);

        // Run KMeans
        K.Iter(10);

        // check that every cluster fits into one page
        std::vector<cxy> pagesForMissed;
        if (!isMaxPaperDimOKPass2(pagesForMissed))
        {
            std::cout << "Cannot fit into " << PageCount << " pages\n";

            // continue to increase number of pages
            continue;
        }

        myPages.insert(
            myPages.end(),
            pagesForMissed.begin(), pagesForMissed.end());

        std::cout << "Cluster pass added " << pagesForMissed.size()
            << ", total pages " << myPages.size()
            << "\n";

        // Display fit found on terminal
        std::cout << text();

        break;
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
        myPages.insert(
            myPages.end(),
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

        // std::cout << "Page " << c << " min size: "
        //           << clusterWidth << " " << clusterHeight
        //           << " range: " << minx << " " << maxx << " " << miny << " " << maxy
        //           << "\n";

        // If entire cluster will NOT fit inside a single page
        // abandon this solution
        if (clusterWidth > myPaper.dim.x ||
            clusterHeight > myPaper.dim.y)
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
    for (auto &pageCenter : myPages)
    {
        std::vector<cxy> page = myPaper.polygon(pageCenter.center);

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

    ss << myPages.size() << " pages of " << myPaper.dim.x
       << " by " << myPaper.dim.y << "\r\n";

    for (int c = 0; c < myPages.size(); c++)
    {
        ss << "Page " << c + 1
           << " center " << myPages[c].center.x 
           << ", " << myPages[c].center.y
           //<< " " << K.clusters()[c].points().size()
           << "\r\n";
    }
    return ss.str();
}

void cMapify::waypointsDisplay(wex::shapes &S)
{
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
        int w = myScale * (myPaper.dim.x);
        int h = myScale * (myPaper.dim.y);
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

cMapify::eMargin cMapify::exitMargin(
    const cxy &lastPoint) const
{
    eMargin margin;
    double bestdist = INT_MAX;
    int imbest;
    auto poly = myPaper.polygon(myPages.back().center);

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



void cPaper::corners()
{
    cornerOffsets.clear();
    cornerOffsets.emplace_back(
        -dim.x / 2,
        -dim.y / 2);
    cornerOffsets.emplace_back(
        +dim.x / 2,
        -dim.y / 2);
    cornerOffsets.emplace_back(
        +dim.x / 2,
        +dim.y / 2);
    cornerOffsets.emplace_back(
        -dim.x / 2,
        +dim.y / 2);
}

cGUI::cGUI()
    : cStarterGUI(
          "Mapify",
          {50, 50, 1000, 1000})
{
    constructMenus();
    registerEventHandlers();
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
                     // prompt user to select waypoint file
                     wex::filebox fb(fm);
                     auto fname = fb.open();
                     if (fname.empty())
                         return;
                     fm.text("Mapify " + fname);
                     if (!M.unitTest())
                     {
                         wex::msgbox("Unit test failed");
                         exit(1);
                     }
                     M.readWaypoints(fname);
                     M.calculate();
                     fm.update();
                 });
    mbar.append("File", mfile);

    // static wex::menu malgo(fm);
    // malgo.append("Cluster",
    //              [&](const std::string &title)
    //              {
    //                  M.algoCluster();
    //                  M.calculate();
    //                  malgo.check(0);
    //                  malgo.check(1, false);
    //                  fm.update();
    //              });
    // malgo.append("Greedy",
    //              [&](const std::string &title)
    //              {
    //                  M.algoGreedy();
    //                  M.calculate();
    //                  malgo.check(0, false);
    //                  malgo.check(1);
    //                  fm.update();
    //              });
    // mbar.append("Algorithm", malgo);

    static wex::menu mdisplay(fm);
    mdisplay.append("Visualization",
                    [&](const std::string &title)
                    {
                        M.DisplayViz();
                        mdisplay.check(0);
                        mdisplay.check(1, false);
                        mdisplay.check(2, false);
                        fm.update();
                    });
    mdisplay.append("Page Locations",
                    [&](const std::string &title)
                    {
                        M.DisplayPages();
                        mdisplay.check(0, false);
                        mdisplay.check(1);
                        mdisplay.check(2, false);
                        fm.update();
                    });
    mdisplay.append("Uncovered",
                    [&](const std::string &title)
                    {
                        M.DisplayUncovered();
                        mdisplay.check(0, false);
                        mdisplay.check(1, false);
                        mdisplay.check(2);
                        fm.update();
                    });
    mdisplay.appendSeparator();
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
    mdisplay.check(0);
    mbar.append("Display", mdisplay);
}
void cGUI::registerEventHandlers()
{
    fm.events().draw(
        [&](PAINTSTRUCT &ps)
        {
            wex::shapes S(ps);
            M.waypointsDisplay(S);
            M.pageDisplay(S);
            M.uncoveredDisplay(S);
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
}

main()
{

    cGUI theGUI;
    return 0;
}
