#include <utilities/jive_UnitTest.h>

#include <jive/jive.h>

//======================================================================================================================
class ViewRendererTests : public jive::UnitTest
{
public:
    //==================================================================================================================
    ViewRendererTests()
        : jive::UnitTest{ "jive::ViewRenderer" }
    {
    }

    //==================================================================================================================
    void runTest() final
    {
        testTreeTypes();
        testNestedComponents();
        testCustomComponents();
    }

private:
    //==================================================================================================================
    void testTreeTypes()
    {
        beginTest("Tree types");

        GIVEN("a view renderer");
        jive::ViewRenderer renderer;

        {
            WHEN("a view is rendered from a value-tree with the type 'ToggleButton'");
            const auto item = renderer.renderView(juce::ValueTree{ "ToggleButton" });

            THEN("the item should be of type `juce::ToggleButton`");
            expect(dynamic_cast<const juce::ToggleButton*>(&item->getComponent()) != nullptr);
        }

        {
            WHEN("a view is rendered from a value-tree with the type 'TextButton'");
            const auto item = renderer.renderView(juce::ValueTree{ "TextButton" });

            THEN("the item should be of type `juce::TextButton`");
            expect(dynamic_cast<const juce::TextButton*>(&item->getComponent()) != nullptr);
        }
    }

    void testNestedComponents()
    {
        beginTest("Nested components");

        GIVEN("a view renderer");
        jive::ViewRenderer renderer;

        {
            WHEN("a view is rendered from a value-tree without any children");
            juce::ValueTree tree{ "TextButton" };
            const auto item = renderer.renderView(tree);

            THEN("the item should have no children");
            expectEquals(item->getNumChildren(), 0);

            THEN("the item's component should have no children");
            expectEquals(item->getComponent().getNumChildComponents(), 0);
        }

        {
            WHEN("a view is rendered from a value-tree with a single child");
            juce::ValueTree tree{ "ToggleButton" };
            tree.appendChild(juce::ValueTree{ "TextButton" }, nullptr);

            const auto item = renderer.renderView(tree);

            THEN("the item should have a single child");
            expectEquals(item->getNumChildren(), 1);

            THEN("the item's component should have a single child which has the ID '123'");
            expectEquals(item->getComponent().getNumChildComponents(), 1);

            THEN("the parent of the item's first child should be the item");
            expect(item->getChild(0).getParent() == item.get());
        }

        {
            WHEN("a view is rendered from a value-tree with two children");
            juce::ValueTree tree{ "TextButton" };
            tree.appendChild(juce::ValueTree{ "ToggleButton" }, nullptr);
            tree.appendChild(juce::ValueTree{ "TextButton" }, nullptr);

            const auto item = renderer.renderView(tree);

            THEN("the item should have a two children");
            expectEquals(item->getNumChildren(), 2);

            THEN("the item's component should have two children");
            expectEquals(item->getComponent().getNumChildComponents(), 2);
        }

        {
            WHEN("a view is rendered from a value-tree with a single node which itself has a single child");
            juce::ValueTree tree{ "ToggleButton" };

            juce::ValueTree childTree{ "TextButton" };
            tree.appendChild(childTree, nullptr);

            juce::ValueTree childChildTree{ "ToggleButton" };
            childTree.appendChild(childChildTree, nullptr);

            const auto item = renderer.renderView(tree);

            THEN("the item should have a single child");
            expectEquals(item->getNumChildren(), 1);

            THEN("the item's component should have a single child");
            expectEquals(item->getComponent().getNumChildComponents(), 1);

            THEN("the item's first child should have a single child");
            expectEquals(item->getChild(0).getNumChildren(), 1);

            THEN("the item's component's first child should have a single child");
            expectEquals(item->getComponent().getChildComponent(0)->getNumChildComponents(), 1);
        }
    }

    void testCustomComponents()
    {
        beginTest("Custom components");

        GIVEN("a view renderer");
        jive::ViewRenderer renderer;

        {
            WHEN("the renderer is given a component creator for trees with the type 'MyCustomComponent' and a view is "
                 "rendered from such a tree");
            struct MyCustomComponent : public juce::Component{};
            renderer.setFactory("MyCustomComponent", []() { return std::make_unique<MyCustomComponent>(); });

            const auto item = renderer.renderView(juce::ValueTree{ "MyCustomComponent" });

            THEN("the item's component should be of the type 'MyCustomComponent'");
            expect(dynamic_cast<const MyCustomComponent*>(&item->getComponent()) != nullptr);
        }

        {
            WHEN("the renderer is given a component creator to override its default one for trees with the type "
                 "'ToggleButton' which returns a component of the type 'AnotherCustomComponent' and a view is rendered "
                 "from such a tree");
            struct AnotherCustomComponent : public juce::Component{};
            renderer.setFactory("ToggleButton", []() { return std::make_unique<AnotherCustomComponent>(); });

            const auto item = renderer.renderView(juce::ValueTree{ "ToggleButton" });

            THEN("the item's component should be of the type 'AnotherCustomComponent'");
            expect(dynamic_cast<const AnotherCustomComponent*>(&item->getComponent()) != nullptr);
        }

        {
            WHEN("the renderer is given a component creator to override its default one for trees with the type "
                 "'TextButton' but then the renderer's creators are reset to their defaults, and a view is rendered "
                 "from such a tree");
            struct YetAnotherComponent : public juce::Component{};
            renderer.setFactory("TextButton", []() { return std::make_unique<YetAnotherComponent>(); });
            renderer.resetFactories();

            const auto item = renderer.renderView(juce::ValueTree{ "TextButton" });

            THEN("the item's component should be of the type 'juce::TextButton'");
            expect(dynamic_cast<const juce::TextButton*>(&item->getComponent()) != nullptr);
        }
    }
};

//======================================================================================================================
static ViewRendererTests viewRendererTests;
