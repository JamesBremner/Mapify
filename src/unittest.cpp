#include "cMapify.h"

bool cMapify::unitTest()
{
    try {
    myPaper.set(10, 10);
    myPages.clear();
    myPages.emplace_back(10,10);
    if (exitMargin( cxy(1, 10)) != eMargin::left)
        return false;
    if (exitMargin( cxy(9, 1)) != eMargin::top)
        return false;
    if (exitMargin( cxy(19, 10)) != eMargin::right)
        return false;
    if (exitMargin( cxy(12, 19)) != eMargin::bottom)
        return false;
    myPages.clear();

    cMapify m;
    m.readWaypoints("../dat/unittest1.txt");
    m.calculate();
    auto result = m.text();
    if( result.find("5 pages") != 0 )
        return false;
    }
    catch( ... ) {
        return false;
    }

    return true;
}