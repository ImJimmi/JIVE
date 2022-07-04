#include <jive_layouts/jive_layouts.h>

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    Interpreter::Interpreter()
    {
        resetFactories();
    }

    //==================================================================================================================
    std::unique_ptr<GuiItem> Interpreter::interpret(juce::ValueTree tree) const
    {
        // Can't render a view from an invalid tree!
        jassert(tree.isValid());

        return interpret(tree, nullptr);
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

    std::unique_ptr<GuiItem> Interpreter::interpret(const juce::String& xmlString) const
    {
        auto xml = juce::parseXML(xmlString);
        recursivelyReplaceSubTextElementsWithTextProperties(*xml);

        return interpret(juce::ValueTree::fromXml(*xml));
    }

    //==================================================================================================================
    void Interpreter::setFactory(const juce::Identifier& treeType, ComponentFactory factory)
    {
        factories.set(treeType.toString(), factory);
    }

    void Interpreter::resetFactories()
    {
        factories.clear();

        setFactory("Button", []() {
            return std::make_unique<juce::TextButton>();
        });
        setFactory("Checkbox", []() {
            return std::make_unique<juce::ToggleButton>();
        });
        setFactory("ComboBox", []() {
            return std::make_unique<juce::ComboBox>();
        });
        setFactory("Component", []() {
            return std::make_unique<IgnoredComponent>();
        });
        setFactory("Hyperlink", []() {
            return std::make_unique<juce::HyperlinkButton>();
        });
        setFactory("Label", []() {
            return std::make_unique<juce::Label>();
        });
        setFactory("Slider", []() {
            return std::make_unique<juce::Slider>();
        });
        setFactory("Spinner", []() {
            return std::make_unique<juce::Slider>();
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
        if (tree.hasType("Button") || tree.hasType("Checkbox"))
            return std::make_unique<Button>(std::move(item));
        if (tree.hasType("ComboBox"))
            return std::make_unique<ComboBox>(std::move(item));
        if (tree.hasType("Hyperlink"))
            return std::make_unique<Hyperlink>(std::move(item));
        if (tree.hasType("Label"))
            return std::make_unique<Label>(std::move(item));
        if (tree.hasType("Slider"))
            return std::make_unique<Slider>(std::move(item));
        if (tree.hasType("Spinner"))
            return std::make_unique<Spinner>(std::move(item));
        if (tree.hasType("Window"))
            return std::make_unique<Window>(std::move(item));

        return item;
    }

    std::unique_ptr<GuiItem> Interpreter::interpret(juce::ValueTree tree, GuiItem* const parent) const
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

    std::unique_ptr<GuiItem> Interpreter::createGuiItem(juce::ValueTree tree, GuiItem* const parent) const
    {
        return std::make_unique<GuiItem>(createComponent(tree), tree, parent);
    }

    void Interpreter::appendChildItems(GuiItem& item, juce::ValueTree tree) const
    {
        for (auto childTree : tree)
            item.addChild(interpret(childTree, &item));
    }

    std::unique_ptr<juce::Component> Interpreter::createComponent(juce::ValueTree tree) const
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
        : juce::UnitTest{ "jive::Interpreter", "jive" }
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

        jive::Interpreter interpreter;

        auto componentView = interpreter.interpret(juce::ValueTree{ "Component" });
        expect(dynamic_cast<jive::GuiItem*>(componentView.get()) != nullptr);

        constexpr auto windowStyleFlags = 0;
        componentView->getComponent().addToDesktop(windowStyleFlags);
        auto* handler = componentView->getComponent().getAccessibilityHandler();
        expect(handler->getRole() == juce::AccessibilityRole::ignored);

        auto labelView = interpreter.interpret(juce::ValueTree{ "Label" });
        expect(dynamic_cast<jive::Label*>(labelView.get()) != nullptr);
        expect(dynamic_cast<juce::Label*>(&labelView->getComponent()) != nullptr);

        auto buttonView = interpreter.interpret(juce::ValueTree{ "Button" });
        expect(dynamic_cast<jive::Button*>(buttonView.get()) != nullptr);
        expect(dynamic_cast<juce::TextButton*>(&buttonView->getComponent()) != nullptr);

        auto checkboxView = interpreter.interpret(juce::ValueTree{ "Checkbox" });
        expect(dynamic_cast<jive::Button*>(checkboxView.get()) != nullptr);
        expect(dynamic_cast<juce::ToggleButton*>(&checkboxView->getComponent()) != nullptr);

        auto hyperlinkView = interpreter.interpret(juce::ValueTree{ "Hyperlink" });
        expect(dynamic_cast<jive::Hyperlink*>(hyperlinkView.get()) != nullptr);
        expect(dynamic_cast<juce::HyperlinkButton*>(&hyperlinkView->getComponent()) != nullptr);

        auto comboBox = interpreter.interpret(juce::ValueTree{ "ComboBox" });
        expect(dynamic_cast<jive::ComboBox*>(comboBox.get()) != nullptr);
        expect(dynamic_cast<juce::ComboBox*>(&comboBox->getComponent()) != nullptr);

        auto window = interpreter.interpret(juce::ValueTree{ "Window" });
        expect(dynamic_cast<jive::Window*>(window.get()) != nullptr);
        expect(dynamic_cast<juce::DocumentWindow*>(&window->getComponent()) != nullptr);

        struct TestComponent : public juce::Component
        {
        };

        interpreter.setFactory("TestComponent", []() {
            return std::make_unique<TestComponent>();
        });

        auto testView = interpreter.interpret(juce::ValueTree{ "TestComponent" });
        expect(dynamic_cast<jive::GuiItem*>(testView.get()) != nullptr);
        expect(dynamic_cast<TestComponent*>(&testView->getComponent()) != nullptr);
    }

    void testNestedComponents()
    {
        beginTest("nested components");

        const jive::Interpreter interpreter;

        {
            juce::ValueTree tree{ "Component" };
            auto view = interpreter.interpret(juce::ValueTree{ "Component" });

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
            auto view = interpreter.interpret(tree);

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
            auto view = interpreter.interpret(tree);

            expect(view->getNumChildren() == tree.getNumChildren());
            expect(view->getComponent().getNumChildComponents() == tree.getNumChildren());

            expect(view->getChild(0).getNumChildren() == tree.getChild(0).getNumChildren());
            expect(view->getChild(0).getComponent().getNumChildComponents() == tree.getChild(0).getNumChildren());
        }
    }

    void testDisplayTypes()
    {
        beginTest("display");

        const jive::Interpreter interpreter;

        auto basicView = interpreter.interpret(juce::ValueTree{ "Component" });
        expect(dynamic_cast<jive::GuiItem*>(basicView.get()));

        auto flexView = interpreter.interpret(juce::ValueTree{
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

        auto gridView = interpreter.interpret(juce::ValueTree{
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

        auto blockView = interpreter.interpret(juce::ValueTree{
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

        const jive::Interpreter interpreter;

        const auto xml = R"(
                <Component>
                    <Label text="Some text"/>
                    <Label>This is some sub-test</Label>
                </Component>
            )";

        expect(interpreter.interpret(xml) != nullptr);
    }

    void testInitialLayout()
    {
        beginTest("initial layout");

        const jive::Interpreter interpreter;
        const auto view = interpreter.interpret(juce::ValueTree{
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

        const jive::Interpreter interpreter;
        const auto view = interpreter.interpret(juce::ValueTree{
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
