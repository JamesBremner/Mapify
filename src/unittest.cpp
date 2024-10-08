#include "cMapify.h"

bool test1()
{
    std::cout << "test1\n";
    cMapify m;
    m.paper(6925, 10000);
    m.readWaypoints("../dat/unittest1.txt");
    m.calculate();
    auto result = m.text();
    if (result.find("7 pages") != 0)
        return false;
    return true;
}
bool test3()
{
    std::cout << "test3\n";
    cMapify m;
    m.paper(10, 20);
    m.addWaypoint(10, 10);
    m.addWaypoint(10, 19);
    m.addWaypoint(10, 30);
    m.calculate();
    if (m.pageCount() != 2)
        return false;
    return true;
}
bool test4()
{
    std::cout << "test4\n";
    cMapify m;
    m.paper(10, 20);
    m.addWaypoint(5, 10);
    m.addWaypoint(5, 19);
    m.addWaypoint(25, 25);
    m.calculate();
    if (m.pageCount() != 2)
        return false;
    return true;
}

bool cMapify::unitTest()
{
    try
    {
                if (!test1())
            return false;
        if (!test4())
            return false;
        if (!test3())
            return false;


        paper(10, 10);
        thePages.clear();
        thePages.emplace_back(10, 10);
        if (exitMargin(cxy(1, 10)) != eMargin::left)
            return false;
        if (exitMargin(cxy(9, 1)) != eMargin::top)
            return false;
        if (exitMargin(cxy(19, 10)) != eMargin::right)
            return false;
        if (exitMargin(cxy(12, 19)) != eMargin::bottom)
            return false;
        thePages.clear();

        paper(10, 20);
        cPage page(100, 100);
        auto poly = page.polygon();
        if (poly[0].x != 95 || poly[0].y != 90)
            return false;
        if (poly[1].x != 105 || poly[1].y != 90)
            return false;
        if (poly[2].x != 105 || poly[2].y != 110)
            return false;
        if (poly[3].x != 95 || poly[3].y != 110)
            return false;

        page.rotated = true;
        poly = page.polygon();
        if (poly[0].x != 90 || poly[0].y != 95)
            return false;
        if (poly[1].x != 110 || poly[1].y != 95)
            return false;
        if (poly[2].x != 110 || poly[2].y != 105)
            return false;
        if (poly[3].x != 90 || poly[3].y != 105)
            return false;

        // cPage next = nextPageLocate(page, 0, eMargin::bottom, false);
        // if (next.center.x != 90 || next.center.y != 120)
        //     return false;
        // next = nextPageLocate(page, 0, eMargin::left,false);
        // if (next.center.x != 90 || next.center.y != 60)
        //     return false;
        // next = nextPageLocate(page, 0, eMargin::bottom, true);
        // if (next.center.x != 80 || next.center.y != 110)
        //     return false;
    }
    catch (...)
    {
        return false;
    }

    // set paper size back to default
    paper(6925, 10000);

    return true;
}