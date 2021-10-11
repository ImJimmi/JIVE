#include <jive/jive.h>

//======================================================================================================================
class ViewRendererTests : public juce::UnitTest
{
public:
    //==================================================================================================================
    ViewRendererTests()
        : juce::UnitTest{ "jive::ViewRenderer" }
    {
    }

    //==================================================================================================================
    void runTest() final
    {
        testEmptyTree();
    }

private:
    //==================================================================================================================
    void testEmptyTree()
    {
        beginTest("Rendering a view with an empty value tree should return a nullptr");

        jive::ViewRenderer renderer;

        const auto view = renderer.createView({});
        expect(view == nullptr);
    }
};

//======================================================================================================================
static ViewRendererTests viewRendererTests;
