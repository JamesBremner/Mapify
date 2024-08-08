class cStarterGUI
{
public:
    /** CTOR
     * @param[in] title will appear in application window title
     * @param[in] vlocation set location and size of appplication window
     * 
     * Usage:
     * 
     * <pre>
class appGUI : public cStarterGUI
{
public:
    appGUI()
        : cStarterGUI(
              "The Appliccation",
              {50, 50, 1000, 500})
    {

        // initialize the solution
        ...

        show();
        run();
    }
    </pre>
    */
    cStarterGUI(
        const std::string &title,
        const std::vector<int> &vlocation)
        : fm(wex::maker::make())
    {
        fm.move(vlocation);
        fm.text(title);

        fm.events().draw(
            [&](PAINTSTRUCT &ps)
            {
                wex::shapes S(ps);
                draw(S);
            });
    }
    /** Draw nothing
     * 
     * An application should over-ride this method
     * to perform any drawing reuired
     */
    virtual void draw( wex::shapes& S )
    {

    }
    void show()
    {
        fm.show();
    }
    void run()
    {
        fm.run();
    }

protected:
    wex::gui &fm;
};

class cGUI : public cStarterGUI
{
public:
    cGUI();

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

    void scale();
    void waypointsDisplay(wex::shapes &S);
    void uncoveredDisplay(wex::shapes &S);
    void pageDisplay(wex::shapes &S);

private:
    cMapify M;
    double myScale, myXoff, myYoff;
        enum class eDisplayTab
    {
        viz,
        page,
        uncovered
    };
    eDisplayTab myDisplayTab;

    void constructMenus();
    void registerEventHandlers();
};