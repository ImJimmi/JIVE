#include <jive_layouts/jive_layouts.h>

namespace jive
{
    const ComponentFactory& Interpreter::getComponentFactory() const
    {
        return componentFactory;
    }

    ComponentFactory& Interpreter::getComponentFactory()
    {
        return componentFactory;
    }

    void Interpreter::setComponentFactory(const ComponentFactory& newFactory)
    {
        componentFactory = newFactory;
    }

    void Interpreter::setAlias(juce::Identifier aliasType, const juce::ValueTree& treeToReplaceWith)
    {
        aliases.emplace(aliasType, treeToReplaceWith.createCopy());
    }

    template <typename Decorator>
    void Interpreter::addDecorator(const juce::Identifier& itemType)
    {
        customDecorators.emplace_back(itemType, [](std::unique_ptr<GuiItem> item) {
            return std::make_unique<Decorator>(std::move(item));
        });
    }

    std::unique_ptr<GuiItem> Interpreter::interpret(const juce::ValueTree& tree) const
    {
        return interpret(tree, nullptr);
    }

    std::unique_ptr<GuiItem> Interpreter::interpret(const juce::XmlElement& xml) const
    {
        return interpret(parseXML(xml));
    }

    std::unique_ptr<GuiItem> Interpreter::interpret(const juce::String& xmlString) const
    {
        return interpret(jive::parseXML(xmlString));
    }

    std::unique_ptr<GuiItem> Interpreter::interpret(const void* xmlStringData, int xmlStringDataSize) const
    {
        return interpret(parseXML(xmlStringData, xmlStringDataSize));
    }

    void Interpreter::listenTo(GuiItem& item)
    {
        if (observedItem != nullptr)
            observedItem->state.removeListener(this);

        observedItem = &item;
        observedItem->state.addListener(this);
    }

    [[nodiscard]] static GuiItem* findItem(GuiItem& root, const juce::ValueTree& state)
    {
        if (root.state == state)
            return &root;

        for (auto* const child : root.getChildren())
        {
            if (auto item = findItem(*child, state))
                return item;
        }

        return nullptr;
    }

    void Interpreter::valueTreeChildAdded(juce::ValueTree& parentTree,
                                          juce::ValueTree& childWhichHasBeenAdded)
    {
        if (observedItem != nullptr)
        {
            if (auto* parentItem = findItem(*observedItem, parentTree))
            {
                const auto index = parentTree.indexOf(childWhichHasBeenAdded);
                insertChild(*parentItem, index, childWhichHasBeenAdded);
            }
        }
    }

    static std::unique_ptr<GuiItem> decorateWithDisplayBehaviour(std::unique_ptr<GuiItem> item)
    {
        Property<Display> display{ item->state, "display" };

        switch (display.get())
        {
        case Display::flex:
            return std::make_unique<FlexContainer>(std::move(item));
        case Display::grid:
            return std::make_unique<GridContainer>(std::move(item));
        case Display::block:
            return std::make_unique<BlockContainer>(std::move(item));
        }

        // Unhandled display type!
        jassertfalse;
        return nullptr;
    }

    static std::unique_ptr<GuiItem> decorateWithHereditaryBehaviour(std::unique_ptr<GuiItem> item)
    {
        if (item->getParent() == nullptr)
            return item;

        Property<Display> display{ item->state.getParent(), "display" };

        switch (display.get())
        {
        case Display::flex:
            return std::make_unique<FlexItem>(std::move(item));
        case Display::grid:
            return std::make_unique<GridItem>(std::move(item));
        case Display::block:
            return std::make_unique<BlockItem>(std::move(item));
        }

        // Unhandled display type!
        jassertfalse;
        return nullptr;
    }

    static std::unique_ptr<GuiItem> decorateWithWidgetBehaviour(std::unique_ptr<GuiItem> item)
    {
        const auto name = item->state.getType().toString();

        if (name == "Button" || name == "Checkbox")
            return std::make_unique<Button>(std::move(item));
        if (name == "ComboBox")
            return std::make_unique<ComboBox>(std::move(item));
        if (name == "Hyperlink")
            return std::make_unique<Hyperlink>(std::move(item));
        if (name == "Image" || name.compareIgnoreCase("svg") == 0)
            return std::make_unique<Image>(std::move(item));
        if (name == "Knob")
            return std::make_unique<Knob>(std::move(item));
        if (name == "Label")
            return std::make_unique<Label>(std::move(item));
        if (name == "ProgressBar")
            return std::make_unique<ProgressBar>(std::move(item));
        if (name == "Slider")
            return std::make_unique<Slider>(std::move(item));
        if (name == "Spinner")
            return std::make_unique<Spinner>(std::move(item));
        if (name == "Text")
            return std::make_unique<Text>(std::move(item));
        if (name == "Window")
            return std::make_unique<Window>(std::move(item));

        return item;
    }

    using DecoratorCreator = std::function<std::unique_ptr<GuiItemDecorator>(std::unique_ptr<GuiItem>)>;

    static std::vector<const DecoratorCreator*> collectDecoratorCreators(const juce::Identifier& itemType,
                                                                         const std::vector<std::pair<juce::Identifier, DecoratorCreator>>& decorators)
    {
        std::vector<const DecoratorCreator*> creators;

        for (const auto& decorator : decorators)
        {
            if (decorator.first == itemType)
                creators.push_back(&decorator.second);
        }

        return creators;
    }

    static std::unique_ptr<GuiItem> decorate(std::unique_ptr<GuiItem> item,
                                             const std::vector<std::pair<juce::Identifier, DecoratorCreator>>& customDecorators)
    {
        item = std::make_unique<CommonGuiItem>(std::move(item));
        item = decorateWithHereditaryBehaviour(std::move(item));
        item = decorateWithWidgetBehaviour(std::move(item));

        if (!item->isContent())
            item = decorateWithDisplayBehaviour(std::move(item));

        for (const auto* decorateWithCustomDecorations : collectDecoratorCreators(item->state.getType(), customDecorators))
            item = (*decorateWithCustomDecorations)(std::move(item));

        return item;
    }

    std::unique_ptr<GuiItem> Interpreter::interpret(const juce::ValueTree& tree, GuiItem* const parent) const
    {
        auto item = createUndecoratedItem(tree, parent);

        if (item != nullptr)
        {
            item = decorate(std::move(item), customDecorators);
            appendChildItems(*item);
        }

        return item;
    }

    void Interpreter::expandAlias(juce::ValueTree& tree) const
    {
        if (const auto alias = aliases.find(tree.getType());
            alias != std::end(aliases))
        {
            auto replacement = alias->second.createCopy();

            for (auto i = 0; i < tree.getNumProperties(); i++)
            {
                auto propertyName = tree.getPropertyName(i);
                replacement.setProperty(propertyName, tree[propertyName], nullptr);
            }

            for (auto child : tree)
            {
                replacement.addChild(child.createCopy(),
                                     tree.indexOf(child),
                                     nullptr);
            }

            auto parent = tree.getParent();
            const auto indexInParent = parent.indexOf(tree);

            parent.removeChild(tree, nullptr);
            parent.addChild(replacement, indexInParent, nullptr);
            tree = replacement;
        }
    }

    std::unique_ptr<GuiItem> Interpreter::createUndecoratedItem(const juce::ValueTree& tree, GuiItem* const parent) const
    {
        auto expandedTree = tree;
        expandAlias(expandedTree);

        if (auto component = createComponent(expandedTree))
        {
            return std::make_unique<GuiItem>(std::move(component),
                                             expandedTree,
#if JIVE_GUI_ITEMS_HAVE_STYLE_SHEETS
                                             new StyleSheet{ *component, expandedTree },
#endif
                                             parent);
        }

        return nullptr;
    }

    void Interpreter::insertChild(GuiItem& item, int index, const juce::ValueTree& childState) const
    {
        auto childItem = interpret(childState, &item);

        if (childItem != nullptr)
        {
            if (item.isContainer() || childItem->isContent())
                item.insertChild(std::move(childItem), index);
        }
    }

    void Interpreter::appendChildItems(GuiItem& item) const
    {
        for (auto i = 0; i < item.state.getNumChildren(); i++)
            insertChild(item, i, item.state.getChild(i));
    }

    std::unique_ptr<juce::Component> Interpreter::createComponent(const juce::ValueTree& tree) const
    {
        const auto name = tree.getType();
        return componentFactory.create(name);
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
        testInitialLayout();
        testWindowContent();
        testCustomDecorators();
        testAliases();
        testInterpretingDifferentSources();
    }

private:
    void testComponentFactory()
    {
        beginTest("component factory");

        jive::Interpreter interpreter;
        expect(interpreter.getComponentFactory().create("MyComponent") == nullptr);

        struct MyComponent : public juce::Component
        {
        };
        interpreter.getComponentFactory().set("MyComponent", []() {
            return std::make_unique<MyComponent>();
        });
        expect(dynamic_cast<MyComponent*>(interpreter.getComponentFactory().create("MyComponent").get()) != nullptr);

        struct YourComponent : public juce::Component
        {
        };
        jive::ComponentFactory factory;
        factory.set("YourComponent", []() {
            return std::make_unique<YourComponent>();
        });
        interpreter.setComponentFactory(factory);
        expect(dynamic_cast<YourComponent*>(interpreter.getComponentFactory().create("YourComponent").get()) != nullptr);
    }

    void testNestedComponents()
    {
        beginTest("nested components");

        const jive::Interpreter interpreter;

        {
            auto view = interpreter.interpret(juce::ValueTree{
                "Component",
                {
                    { "width", 222 },
                    { "height", 333 },
                },
            });

            expectEquals(view->getChildren().size(), 0);
            expectEquals(view->getComponent()->getNumChildComponents(), 0);
        }
        {
            juce::ValueTree tree{
                "Component",
                {
                    { "width", 222 },
                    { "height", 333 },
                },
                {
                    juce::ValueTree{ "Component" },
                    juce::ValueTree{ "Component" },
                },
            };
            auto view = interpreter.interpret(tree);

            expect(view->getChildren().size() == tree.getNumChildren());
            expect(view->getComponent()->getNumChildComponents() == tree.getNumChildren());
        }
        {
            juce::ValueTree tree{
                "Component",
                {
                    { "width", 222 },
                    { "height", 333 },
                },
                {
                    juce::ValueTree{
                        "Component",
                        {},
                        {
                            juce::ValueTree{ "Component" },
                            juce::ValueTree{ "Component" },
                            juce::ValueTree{ "Component" },
                        },
                    },
                },
            };
            auto view = interpreter.interpret(tree);

            expect(view->getChildren().size() == tree.getNumChildren());
            expect(view->getComponent()->getNumChildComponents() == tree.getNumChildren());

            expect(view->getChildren()[0]->getChildren().size() == tree.getChild(0).getNumChildren());
            expect(view->getChildren()[0]->getComponent()->getNumChildComponents() == tree.getChild(0).getNumChildren());
        }
    }

    void testDisplayTypes()
    {
        beginTest("display");

        const jive::Interpreter interpreter;

        auto basicView = interpreter.interpret(juce::ValueTree{
            "Component",
            {
                { "width", 222 },
                { "height", 333 },
            },
        });
        expect(dynamic_cast<jive::GuiItemDecorator&>(*basicView)
                   .toType<jive::CommonGuiItem>()
               != nullptr);

        auto flexView = interpreter.interpret(juce::ValueTree{
            "Component",
            {
                { "width", 222 },
                { "height", 333 },
                { "display", juce::VariantConverter<jive::Display>::toVar(jive::Display::flex) },
            },
            {
                juce::ValueTree{ "Label" },
            },
        });
        expect(dynamic_cast<jive::GuiItemDecorator&>(*flexView)
                   .toType<jive::FlexContainer>()
               != nullptr);
        expect(dynamic_cast<jive::GuiItemDecorator&>(*flexView->getChildren()[0])
                   .toType<jive::FlexItem>()
               != nullptr);

        auto gridView = interpreter.interpret(juce::ValueTree{
            "Component",
            {
                { "width", 222 },
                { "height", 333 },
                { "display", "grid" },
            },
            {
                juce::ValueTree{ "Component" },
            },
        });
        expect(dynamic_cast<jive::GuiItemDecorator&>(*gridView)
                   .toType<jive::GridContainer>()
               != nullptr);
        expect(dynamic_cast<jive::GuiItemDecorator&>(*gridView->getChildren()[0])
                   .toType<jive::GridItem>()
               != nullptr);

        auto blockView = interpreter.interpret(juce::ValueTree{
            "Component",
            {
                { "width", 222 },
                { "height", 333 },
                { "display", "block" },
            },
            {
                juce::ValueTree{ "Component" },
            },
        });
        expect(dynamic_cast<jive::GuiItemDecorator&>(*blockView)
                   .toType<jive::BlockContainer>()
               != nullptr);
        expect(dynamic_cast<jive::GuiItemDecorator&>(*blockView->getChildren()[0])
                   .toType<jive::BlockItem>()
               != nullptr);
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
                },
                {
                    "Component",
                    {
                        { "width", 100 },
                        { "height", 100 },
                    },
                },
            },
        });
        expectNotEquals(view->getChildren()[1]->getComponent()->getPosition(),
                        juce::Point<int>{});
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
        auto& window = dynamic_cast<jive::GuiItemDecorator&>(*view)
                           .toType<jive::Window>()
                           ->getWindow();
        expect(window.getContentComponent() != nullptr);
    }

    void testCustomDecorators()
    {
        beginTest("custom decorators");

        struct MyDecorator : public jive::GuiItemDecorator
        {
            using jive::GuiItemDecorator::GuiItemDecorator;
        };

        jive::Interpreter interpreter;
        auto item = interpreter.interpret(juce::ValueTree{
            "Button",
            {
                { "width", 222 },
                { "height", 333 },
            },
        });
        expect(dynamic_cast<MyDecorator*>(item.get()) == nullptr);

        interpreter.addDecorator<MyDecorator>("Button");
        item = interpreter.interpret(juce::ValueTree{
            "Button",
            {
                { "width", 222 },
                { "height", 333 },
            },
        });
        auto* decorator = dynamic_cast<jive::GuiItemDecorator*>(item.get());
        expect(decorator->toType<MyDecorator>() != nullptr);

        struct MyOtherDecorator : public jive::GuiItemDecorator
        {
            using jive::GuiItemDecorator::GuiItemDecorator;
        };
        interpreter.addDecorator<MyOtherDecorator>("Button");
        item = interpreter.interpret(juce::ValueTree{
            "Button",
            {
                { "width", 222 },
                { "height", 333 },
            },
        });
        decorator = dynamic_cast<jive::GuiItemDecorator*>(item.get());
        expect(decorator->toType<MyDecorator>() != nullptr);
        expect(decorator->toType<MyOtherDecorator>() != nullptr);
    }

    void testAliases()
    {
        beginTest("aliases");

        const juce::ValueTree state{
            "Window",
            {
                { "width", 123 },
                { "height", 456 },
            },
            {
                juce::ValueTree{
                    "SomeAlias",
                    {
                        { "padding", 10 },
                        { "margin", "1 2 3 4" },
                    },
                },
            },
        };

        jive::Interpreter interpreter;
        auto window = interpreter.interpret(state);
        expect(window->getChildren().size() == 0);

        interpreter.setAlias("SomeAlias",
                             juce::ValueTree{
                                 "Button",
                                 {
                                     { "margin", 33 },
                                     { "enabled", false },
                                 },
                             });
        window = interpreter.interpret(state);
        expectEquals(window->getChildren().size(), 1);
        expectEquals(window->getChildren()[0]->state["padding"].toString(), juce::String{ "10" });
        expectEquals(window->getChildren()[0]->state["margin"].toString(), juce::String{ "1 2 3 4" });
        expect(!static_cast<bool>(window->getChildren()[0]->state["enabled"]));
    }

    void testInterpretingDifferentSources()
    {
        {
            beginTest("interpreting juce::ValueTree");

            const jive::Interpreter interpreter;
            const auto result = interpreter.interpret(juce::ValueTree{
                "Component",
                {
                    { "width", 123 },
                    { "height", 456 },
                },
            });
            expect(result != nullptr);
        }
        {
            beginTest("interpreting juce::XmlElement");

            const jive::Interpreter interpreter;
            const auto result = interpreter.interpret(*juce::parseXML(R"(
                <Component width="123" height="456"/>
            )"));
            expect(result != nullptr);
        }
        {
            beginTest("interpreting juce::String");

            const jive::Interpreter interpreter;
            const auto result = interpreter.interpret(R"(
                <Component width="123" height="456"/>
            )");
            expect(result != nullptr);
        }
        {
            beginTest("interpreting string data");

            const jive::Interpreter interpreter;
            const juce::String source = R"(
                <Component width="123" height="456"/>
            )";
            const auto result = interpreter.interpret(source.toRawUTF8(), source.length());
            expect(result != nullptr);
        }
    }
};

static ViewRendererUnitTest viewRendererUnitTest;
#endif
