#include <catch2/catch_test_macros.hpp>
#include <jive/jive.h>

//======================================================================================================================
SCENARIO("view renderers can render different components")
{
    GIVEN("a view renderer")
    {
        jive::ViewRenderer renderer;

        WHEN("a view is rendered from a value-tree with a 'ToggleButton' type")
        {
            auto view = renderer.renderView(juce::ValueTree{ "ToggleButton" });

            THEN("the view's component should be of the type 'juce::ToggleButton'")
            {
                REQUIRE(dynamic_cast<juce::ToggleButton*>(&view->getComponent()) != nullptr);
            }
        }
        WHEN("a view is rendered from a value-tree with a 'TextButton' type")
        {
            auto view = renderer.renderView(juce::ValueTree{ "TextButton" });

            THEN("the view's component should be of the type 'juce::TextButton'")
            {
                REQUIRE(dynamic_cast<juce::TextButton*>(&view->getComponent()) != nullptr);
            }
        }
    }
}

//======================================================================================================================
SCENARIO("view renderers can render nested components")
{
    GIVEN("a view renderer")
    {
        jive::ViewRenderer renderer;

        WHEN("a view is rendered from a value-tree with no children")
        {
            juce::ValueTree tree{ "ToggleButton" };
            auto view = renderer.renderView(juce::ValueTree{ "ToggleButton" });

            THEN("the view has the same number of children")
            {
                REQUIRE(view->getNumChildren() == tree.getNumChildren());
            }
            THEN("the top-level component has the same number of children")
            {
                REQUIRE(view->getComponent().getNumChildComponents() == tree.getNumChildren());
            }
        }
        WHEN("a view is rendered from a value-tree with some children")
        {
            juce::ValueTree tree{
                "ToggleButton",
                {},
                { juce::ValueTree{ "ToggleButton" },
                  juce::ValueTree{ "ToggleButton" } }
            };
            auto view = renderer.renderView(tree);

            THEN("the view has the same number of children")
            {
                REQUIRE(view->getNumChildren() == tree.getNumChildren());
            }
            THEN("the top-level component has the same number of children")
            {
                REQUIRE(view->getComponent().getNumChildComponents() == tree.getNumChildren());
            }
        }
        WHEN("a view is rendered from a value-tree with a single child tree that itself has some children")
        {
            juce::ValueTree tree{
                "ToggleButton",
                {},
                { juce::ValueTree{
                    "ToggleButton",
                    {},
                    { juce::ValueTree{ "ToggleButton" },
                      juce::ValueTree{ "ToggleButton" },
                      juce::ValueTree{ "ToggleButton" } } } }
            };
            auto view = renderer.renderView(tree);

            THEN("the view has the name number of children as the tree's top-level node")
            {
                REQUIRE(view->getNumChildren() == tree.getNumChildren());
            }
            THEN("the top-level component has the same number of children as the tree's top-level node")
            {
                REQUIRE(view->getComponent().getNumChildComponents() == tree.getNumChildren());
            }
            THEN("the view's first child has the same number of children as the first child in the tree")
            {
                REQUIRE(view->getChild(0).getNumChildren() == tree.getChild(0).getNumChildren());
            }
            THEN("the first child in the view's top-level component has the same number of children as the first child "
                 "in the tree")
            {
                REQUIRE(view->getComponent().getChildComponent(0)->getNumChildComponents() == tree.getChild(0).getNumChildren());
            }
        }
    }
}

//======================================================================================================================
SCENARIO("view renderers can render custom components")
{
    GIVEN("a view renderer")
    {
        jive::ViewRenderer renderer;

        WHEN("the renderer is given a factory method for trees with the type 'MyCustomComponent'")
        {
            struct MyCustomComponent : public juce::Component
            {
            };

            renderer.setFactory("MyCustomComponent", []() {
                return std::make_unique<MyCustomComponent>();
            });

            AND_WHEN("a view is rendered from a value-tree with a 'MyCustomComponent' type")
            {
                auto view = renderer.renderView(juce::ValueTree{ "MyCustomComponent" });

                THEN("the top-level component is of the custom type")
                {
                    REQUIRE(dynamic_cast<MyCustomComponent*>(&view->getComponent()) != nullptr);
                }
            }
        }
        WHEN("the renderer has its 'TextButton' factory overwritten to return a custom component")
        {
            struct MyCustomTextButton : public juce::Component
            {
            };

            renderer.setFactory("TextButton", []() {
                return std::make_unique<MyCustomTextButton>();
            });

            AND_WHEN("a view is rendered from a value-tree with a 'TextButton' type")
            {
                auto view = renderer.renderView(juce::ValueTree{ "TextButton" });

                THEN("the top-level component is of the custom type")
                {
                    REQUIRE(dynamic_cast<MyCustomTextButton*>(&view->getComponent()) != nullptr);
                }
            }
            AND_WHEN("the renderer's component factories are reset to their defaults")
            {
                renderer.resetFactories();

                AND_WHEN("a view is rendered from a value-tree with a 'TextButton' type")
                {
                    auto view = renderer.renderView(juce::ValueTree{ "TextButton" });

                    THEN("the top-level component has the original type")
                    {
                        REQUIRE(dynamic_cast<juce::TextButton*>(&view->getComponent()) != nullptr);
                    }
                }
            }
        }
    }
}

//======================================================================================================================
SCENARIO("view renderers can render items with different display types")
{
    GIVEN("a view renderer")
    {
        jive::ViewRenderer renderer;

        WHEN("a view is rendered from a value-tree with no display type specified")
        {
            auto item = renderer.renderView(juce::ValueTree{ "TextButton" });

            THEN("the rendered item is a basic GUI item")
            {
                REQUIRE(juce::String{ typeid(item).name() }.contains("GuiItem"));
            }
        }
        WHEN("a view is rendered from a value-tree with a flex display type, and a child")
        {
            juce::ValueTree tree{
                "TextButton",
                { { "display", juce::VariantConverter<jive::GuiItem::Display>::toVar(jive::GuiItem::Display::flex) } },
                { juce::ValueTree{ "ToggleButton" } }
            };
            auto item = renderer.renderView(tree);

            THEN("the rendered item is a flex container")
            {
                REQUIRE(dynamic_cast<jive::GuiFlexContainer*>(item.get()) != nullptr);
            }
            THEN("the first child is a flex item")
            {
                auto& child = item->getChild(0);
                REQUIRE(dynamic_cast<jive::GuiFlexItem*>(&child) != nullptr);
            }
        }
    }
}
