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
        testTreeWithID();
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

    void testTreeWithID()
    {
        jive::ViewRenderer renderer;

        {
            beginTest("Rendering a view from a value tree with a type of 'ToggleButton' and a blank 'id' property "
                      "should return a component with no ID");

            juce::ValueTree tree{ "ToggleButton" };

            const auto view = renderer.createView(tree);
            expectEquals(view->getComponentID(), juce::String{});
        }

        {
            beginTest("Rendering a view from a value tree with a type of 'ToggleButton' and an 'id' property with the "
                      "value '123' should return a component with the ID '123'");

            juce::ValueTree tree{ "ToggleButton" };
            tree.setProperty("id", "123", nullptr);

            const auto view = renderer.createView(tree);
            expectEquals(view->getComponentID(), juce::String{ "123" });
        }

        {
            beginTest("Rendering a view from a value tree with a type of 'ToggleButton' and an 'id' property with the "
                      "value '567' should return a component with the ID '567'");

            juce::ValueTree tree{ "ToggleButton" };
            tree.setProperty("id", "567", nullptr);

            const auto view = renderer.createView(tree);
            expectEquals(view->getComponentID(), juce::String{ "567" });
        }

        {
            beginTest("Rendering a view from a value tree with a type of 'TextButton' and an 'id' property with the "
                      "value '987' should return a component with the ID '987'");

            juce::ValueTree tree{ "TextButton" };
            tree.setProperty("id", "987", nullptr);

            const auto view = renderer.createView(tree);
            expectEquals(view->getComponentID(), juce::String{ "987" });
        }

        {
            beginTest("Rendering a view from a value tree with a type of 'TextButton' and an 'id' property with the "
                      "value '543' should return a component with the ID '543'");

            juce::ValueTree tree{ "TextButton" };
            tree.setProperty("id", "543", nullptr);

            const auto view = renderer.createView(tree);
            expectEquals(view->getComponentID(), juce::String{ "543" });
        }
    }
};

//======================================================================================================================
static ViewRendererTests viewRendererTests;
