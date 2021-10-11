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
        testTreeWithToggleButtonType();
        testTreeWithTextButtonType();
    }

private:
    //==================================================================================================================
    void testEmptyTree()
    {
        beginTest("Rendering a view from an empty value tree should return a nullptr");

        jive::ViewRenderer renderer;

        const auto view = renderer.createView({});
        expect(view == nullptr);
    }

    void testTreeWithToggleButtonType()
    {
        beginTest("Rendering a view from a value tree with a type of 'ToggleButton' should return a "
                  "juce::ToggleButton");

        jive::ViewRenderer renderer;

        const auto view = renderer.createView(juce::ValueTree{ "ToggleButton" });
        expect(dynamic_cast<juce::ToggleButton*>(view.get()) != nullptr);
    }

    void testTreeWithTextButtonType()
    {
        beginTest("Rendering a view from a value tree with a type of 'TextButton' should return a "
                  "juce::TextButton");

        jive::ViewRenderer renderer;

        const auto view = renderer.createView(juce::ValueTree{ "TextButton" });
        expect(dynamic_cast<juce::TextButton*>(view.get()) != nullptr);
    }
};

//======================================================================================================================
static ViewRendererTests viewRendererTests;
