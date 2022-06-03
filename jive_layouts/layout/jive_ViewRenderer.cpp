#include <jive_layouts/jive_layouts.h>

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    ViewRenderer::ViewRenderer()
    {
        resetFactories();
    }

    //==================================================================================================================
    std::unique_ptr<GuiItem> ViewRenderer::renderView(juce::ValueTree tree) const
    {
        // Can't render a view from an invalid tree!
        jassert(tree.isValid());

        return renderView(tree, nullptr);
    }

    void replaceTextElementWithTextProperty(juce::XmlElement& parentXML, juce::XmlElement& textChild)
    {
        parentXML.setAttribute("text", textChild.getText());
        parentXML.removeChildElement(&textChild, true);
    }

    void recursivelyReplaceSubTextElementsWithTextProperties(juce::XmlElement& xml)
    {
        for (auto i = xml.getNumChildElements() - 1; i >= 0; i--)
        {
            auto* child = xml.getChildElement(i);

            recursivelyReplaceSubTextElementsWithTextProperties(*child);

            if (child->getTagName().isEmpty())
                replaceTextElementWithTextProperty(xml, *child);
        }
    }

    std::unique_ptr<GuiItem> ViewRenderer::renderView(const juce::String& xmlString) const
    {
        auto xml = juce::parseXML(xmlString);
        recursivelyReplaceSubTextElementsWithTextProperties(*xml);

        return renderView(juce::ValueTree::fromXml(*xml));
    }

    //==================================================================================================================
    void ViewRenderer::setFactory(const juce::Identifier& treeType, ComponentFactory factory)
    {
        factories.set(treeType.toString(), factory);
    }

    void ViewRenderer::resetFactories()
    {
        factories.clear();

        setFactory("Component", []() {
            return std::make_unique<IgnoredComponent>();
        });
        setFactory("Label", []() {
            return std::make_unique<juce::Label>();
        });
        setFactory("Button", []() {
            return std::make_unique<juce::TextButton>();
        });
        setFactory("Checkbox", []() {
            return std::make_unique<juce::ToggleButton>();
        });
        setFactory("Hyperlink", []() {
            return std::make_unique<juce::HyperlinkButton>();
        });
        setFactory("ComboBox", []() {
            return std::make_unique<juce::ComboBox>();
        });
        setFactory("Window", []() {
            return std::make_unique<DocumentWindow>();
        });
    }

    //==================================================================================================================
    std::unique_ptr<GuiItem> decorateWithDisplayBehaviour(std::unique_ptr<GuiItem> item)
    {
        switch (item->getDisplay())
        {
        case GuiItem::Display::flex:
            return std::make_unique<FlexContainer>(std::move(item));
        case GuiItem::Display::grid:
            return std::make_unique<GridContainer>(std::move(item));
        case GuiItem::Display::block:
            return std::make_unique<BlockContainer>(std::move(item));
        }

        // Unhandled display type!
        jassertfalse;
        return nullptr;
    }

    std::unique_ptr<GuiItem> decorateWithHereditaryBehaviour(std::unique_ptr<GuiItem> item)
    {
        if (item->getParent() == nullptr)
            return item;

        switch (item->getParent()->getDisplay())
        {
        case GuiItem::Display::flex:
            return std::make_unique<FlexItem>(std::move(item));
        case GuiItem::Display::grid:
            return std::make_unique<GridItem>(std::move(item));
        case GuiItem::Display::block:
            return std::make_unique<BlockItem>(std::move(item));
        }

        // Unhandled display type!
        jassertfalse;
        return nullptr;
    }

    std::unique_ptr<GuiItem> decorateWithWidgetBehaviour(std::unique_ptr<GuiItem> item, const juce::ValueTree& tree)
    {
        if (tree.hasType("Label"))
            return std::make_unique<Label>(std::move(item));
        if (tree.hasType("Button") || tree.hasType("Checkbox"))
            return std::make_unique<Button>(std::move(item));
        if (tree.hasType("Hyperlink"))
            return std::make_unique<Hyperlink>(std::move(item));
        if (tree.hasType("ComboBox"))
            return std::make_unique<ComboBox>(std::move(item));
        if (tree.hasType("Window"))
            return std::make_unique<Window>(std::move(item));

        return item;
    }

    std::unique_ptr<GuiItem> ViewRenderer::renderView(juce::ValueTree tree, GuiItem* const parent) const
    {
        auto guiItem = createGuiItem(tree, parent);

        guiItem = decorateWithDisplayBehaviour(std::move(guiItem));
        guiItem = decorateWithWidgetBehaviour(std::move(guiItem), tree);
        guiItem = decorateWithHereditaryBehaviour(std::move(guiItem));

        if (guiItem->isContainer())
            appendChildItems(*guiItem, tree);

        guiItem->updateLayout();

        return guiItem;
    }

    bool treeHasMatchingTypeIgnoringCase(const juce::ValueTree& tree, const juce::String& expectedType)
    {
        return tree.getType().toString().equalsIgnoreCase(expectedType);
    }

    std::unique_ptr<GuiItem> ViewRenderer::createGuiItem(juce::ValueTree tree, GuiItem* const parent) const
    {
        return std::make_unique<GuiItem>(createComponent(tree), tree, parent);
    }

    void ViewRenderer::appendChildItems(GuiItem& item, juce::ValueTree tree) const
    {
        for (auto childTree : tree)
            item.addChild(renderView(childTree, &item));
    }

    std::unique_ptr<juce::Component> ViewRenderer::createComponent(juce::ValueTree tree) const
    {
        const auto treeType = tree.getType().toString();

        if (factories.contains(treeType))
        {
            const auto create = factories[treeType];
            return create();
        }

        // No creator for the given ValueTree's type.
        jassertfalse;
        return nullptr;
    }
} // namespace jive

#if JIVE_UNIT_TESTS
class ViewRendererUnitTest : public juce::UnitTest
{
public:
    ViewRendererUnitTest()
        : juce::UnitTest{ "jive::ViewRenderer", "jive" }
    {
    }

    void runTest() final
    {
        testComponentFactory();
        testNestedComponents();
        testDisplayTypes();
        testXML();
        testInitialLayout();
        testWindowContent();
    }

private:
    void testComponentFactory()
    {
        beginTest("component factory");

        jive::ViewRenderer renderer;

        auto componentView = renderer.renderView(juce::ValueTree{ "Component" });
        expect(dynamic_cast<jive::GuiItem*>(componentView.get()) != nullptr);

        constexpr auto windowStyleFlags = 0;
        componentView->getComponent().addToDesktop(windowStyleFlags);
        auto* handler = componentView->getComponent().getAccessibilityHandler();
        expect(handler->getRole() == juce::AccessibilityRole::ignored);

        auto labelView = renderer.renderView(juce::ValueTree{ "Label" });
        expect(dynamic_cast<jive::Label*>(labelView.get()) != nullptr);
        expect(dynamic_cast<juce::Label*>(&labelView->getComponent()) != nullptr);

        auto buttonView = renderer.renderView(juce::ValueTree{ "Button" });
        expect(dynamic_cast<jive::Button*>(buttonView.get()) != nullptr);
        expect(dynamic_cast<juce::TextButton*>(&buttonView->getComponent()) != nullptr);

        auto checkboxView = renderer.renderView(juce::ValueTree{ "Checkbox" });
        expect(dynamic_cast<jive::Button*>(checkboxView.get()) != nullptr);
        expect(dynamic_cast<juce::ToggleButton*>(&checkboxView->getComponent()) != nullptr);

        auto hyperlinkView = renderer.renderView(juce::ValueTree{ "Hyperlink" });
        expect(dynamic_cast<jive::Hyperlink*>(hyperlinkView.get()) != nullptr);
        expect(dynamic_cast<juce::HyperlinkButton*>(&hyperlinkView->getComponent()) != nullptr);

        auto comboBox = renderer.renderView(juce::ValueTree{ "ComboBox" });
        expect(dynamic_cast<jive::ComboBox*>(comboBox.get()) != nullptr);
        expect(dynamic_cast<juce::ComboBox*>(&comboBox->getComponent()) != nullptr);

        auto window = renderer.renderView(juce::ValueTree{ "Window" });
        expect(dynamic_cast<jive::Window*>(window.get()) != nullptr);
        expect(dynamic_cast<juce::DocumentWindow*>(&window->getComponent()) != nullptr);

        struct TestComponent : public juce::Component
        {
        };

        renderer.setFactory("TestComponent", []() {
            return std::make_unique<TestComponent>();
        });

        auto testView = renderer.renderView(juce::ValueTree{ "TestComponent" });
        expect(dynamic_cast<jive::GuiItem*>(testView.get()) != nullptr);
        expect(dynamic_cast<TestComponent*>(&testView->getComponent()) != nullptr);
    }

    void testNestedComponents()
    {
        beginTest("nested components");

        const jive::ViewRenderer renderer;

        {
            juce::ValueTree tree{ "Component" };
            auto view = renderer.renderView(juce::ValueTree{ "Component" });

            expect(view->getNumChildren() == tree.getNumChildren());
            expect(view->getComponent().getNumChildComponents() == tree.getNumChildren());
        }
        {
            juce::ValueTree tree{
                "Component",
                {},
                { juce::ValueTree{ "Component" },
                  juce::ValueTree{ "Component" } }
            };
            auto view = renderer.renderView(tree);

            expect(view->getNumChildren() == tree.getNumChildren());
            expect(view->getComponent().getNumChildComponents() == tree.getNumChildren());
        }
        {
            juce::ValueTree tree{
                "Component",
                {},
                { juce::ValueTree{
                    "Component",
                    {},
                    { juce::ValueTree{ "Component" },
                      juce::ValueTree{ "Component" },
                      juce::ValueTree{ "Component" } } } }
            };
            auto view = renderer.renderView(tree);

            expect(view->getNumChildren() == tree.getNumChildren());
            expect(view->getComponent().getNumChildComponents() == tree.getNumChildren());

            expect(view->getChild(0).getNumChildren() == tree.getChild(0).getNumChildren());
            expect(view->getChild(0).getComponent().getNumChildComponents() == tree.getChild(0).getNumChildren());
        }
    }

    void testDisplayTypes()
    {
        beginTest("display");

        const jive::ViewRenderer renderer;

        auto basicView = renderer.renderView(juce::ValueTree{ "Component" });
        expect(dynamic_cast<jive::GuiItem*>(basicView.get()));

        auto flexView = renderer.renderView(juce::ValueTree{
            "Component",
            {
                { "display", juce::VariantConverter<jive::GuiItem::Display>::toVar(jive::GuiItem::Display::flex) },
            },
            {
                juce::ValueTree{ "Label" },
            },
        });
        expect(dynamic_cast<jive::FlexContainer*>(flexView.get()));
        expect(dynamic_cast<jive::FlexItem*>(&flexView->getChild(0)));

        auto gridView = renderer.renderView(juce::ValueTree{
            "Component",
            {
                { "display", "grid" },
            },
            {
                juce::ValueTree{ "Component" },
            },
        });
        expect(dynamic_cast<jive::GridContainer*>(gridView.get()));
        expect(dynamic_cast<jive::GridItem*>(&gridView->getChild(0)));

        auto blockView = renderer.renderView(juce::ValueTree{
            "Component",
            {
                { "display", "block" },
            },
            {
                juce::ValueTree{ "Component" },
            },
        });
        expect(dynamic_cast<jive::GuiItem*>(blockView.get()));
        expect(dynamic_cast<jive::BlockItem*>(&blockView->getChild(0)));
    }

    void testXML()
    {
        beginTest("xml");

        const jive::ViewRenderer renderer;

        const auto xml = R"(
                <Component>
                    <Label text="Some text"/>
                    <Label>This is some sub-test</Label>
                </Component>
            )";

        expect(renderer.renderView(xml) != nullptr);
    }

    void testInitialLayout()
    {
        beginTest("initial layout");

        const jive::ViewRenderer renderer;
        const auto view = renderer.renderView(juce::ValueTree{
            "Component",
            {
                { "width", 200 },
                { "height", 200 },
            },
            {
                {
                    "Component",
                    {
                        { "width", 100 },
                        { "height", 100 },
                    },
                    {},
                },
                {
                    "Component",
                    {
                        { "width", 100 },
                        { "height", 100 },
                    },
                    {},
                },
            },
        });

        expect(view->getChild(1).getComponent().getPosition() != juce::Point<int>{});
    }

    void testWindowContent()
    {
        beginTest("window content");

        const jive::ViewRenderer renderer;
        const auto view = renderer.renderView(juce::ValueTree{
            "Window",
            {
                { "width", 200 },
                { "height", 200 },
            },
            {
                {
                    "Component",
                    {
                        { "width", 100 },
                        { "height", 100 },
                    },
                    {},
                },
            },
        });
        expect(dynamic_cast<jive::Window*>(view.get()) != nullptr);
        expect(dynamic_cast<jive::Window*>(view.get())->getWindow().getContentComponent() != nullptr);
    }
};

static ViewRendererUnitTest viewRendererUnitTest;
#endif
