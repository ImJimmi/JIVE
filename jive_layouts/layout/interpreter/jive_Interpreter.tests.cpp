#include <jive_layouts/jive_layouts.h>

static jive::UnitTest componentFactoryTest{
    "jive",
    "jive::Interpreter",
    "Component Factory",
    [](auto& test) {
        jive::Interpreter interpreter;
        test.expect(interpreter.getComponentFactory().create("MyComponent") == nullptr);

        struct MyComponent : public juce::Component
        {
        };
        interpreter.getComponentFactory().set("MyComponent", []() {
            return std::make_unique<MyComponent>();
        });
        test.expect(dynamic_cast<MyComponent*>(interpreter.getComponentFactory().create("MyComponent").get()) != nullptr);

        struct YourComponent : public juce::Component
        {
        };
        jive::ComponentFactory factory;
        factory.set("YourComponent", []() {
            return std::make_unique<YourComponent>();
        });
        interpreter.setComponentFactory(factory);
        test.expect(dynamic_cast<YourComponent*>(interpreter.getComponentFactory().create("YourComponent").get()) != nullptr);
    },
};

static jive::UnitTest fileSourcesTest{
    "jive",
    "jive::Interpreter",
    "File Sources",
    [](auto& test) {
        const auto cwd = juce::File::getSpecialLocation(juce::File::currentApplicationFile)
                             .getParentDirectory()
                             .getChildFile("interpreter-tests/");
        cwd.deleteRecursively();
        auto basicView = cwd.getChildFile("basic-view.xml");
        basicView.create();
        basicView.replaceWithText(R"(<Window width="100" height="100"/>)");

        jive::Interpreter interpreter;
        const auto item = interpreter.interpret(basicView);
        test.expect(item != nullptr);

        auto& window = *item->getComponent()->getTopLevelComponent();
        test.expectEquals(window.getWidth(), 100);
        test.expectEquals(window.getHeight(), 100);

        basicView.replaceWithText(R"(<Window width="200" height="200"/>)");
        jive::FileObserver::triggerAllTimerCallbacks();
        test.expectEquals(window.getWidth(), 200);
        test.expectEquals(window.getHeight(), 200);
    },
};

static jive::UnitTest nestedFileSourcesTest{
    "jive",
    "jive::Interpreter",
    "Nested File Sources",
    [](auto& test) {
        const auto cwd = juce::File::getSpecialLocation(juce::File::currentApplicationFile)
                             .getParentDirectory()
                             .getChildFile("interpreter-tests/");
        cwd.deleteRecursively();
        auto alpha = cwd.getChildFile("alpha.xml");
        alpha.create();
        alpha.replaceWithText(R"(<Component id="alpha"/>)");
        auto beta = cwd.getChildFile("beta.xml");
        beta.create();
        beta.replaceWithText(R"(<Component id="beta"/>)");

        auto mainView = cwd.getChildFile("nested-view.xml");
        mainView.create();
        mainView.replaceWithText(R"(
            <Window width="100" height="100">
                <Component source="alpha.xml" name="Alpha"/>
            </Window>
        )");

        // Create the window from the original source
        jive::Interpreter interpreter;
        const auto item = interpreter.interpret(mainView);
        test.expect(item != nullptr, "Item should have been created");

        auto* window = dynamic_cast<jive::Window*>(item->getComponent()->getTopLevelComponent());
        auto* child = window->getComponent()->findChildWithID("alpha");
        test.expectEquals(child->getName(),
                          juce::String{ "Alpha" },
                          "Window should have an 'alpha' child with the name 'Alpha'");

        // Replace the Alpha component with a Beta component in the main source
        mainView.replaceWithText(R"(
            <Window width="100" height="100">
                <Component source="beta.xml" name="Beta"/>
            </Window>
        )");
        jive::FileObserver::triggerAllTimerCallbacks();
        test.expect(window->getComponent()->findChildWithID("alpha") == nullptr,
                    "Window should no longer have an 'alpha' child");
        child = window->getComponent()->findChildWithID("beta");
        test.expectEquals(child->getName(),
                          juce::String{ "Beta" },
                          "Window should have a 'beta' child with the name 'Beta'");

        // Change a property in the beta source
        beta.replaceWithText(R"(<Component id="gamma"/>)");
        jive::FileObserver::triggerAllTimerCallbacks();
        test.expect(window->getComponent()->findChildWithID("alpha") == nullptr,
                    "Window should not have an 'alpha' child");
        test.expect(window->getComponent()->findChildWithID("beta") == nullptr,
                    "Window should not have a 'beta' child");
        test.expect(window->getComponent()->findChildWithID("gamma") != nullptr,
                    "Window should have a 'gamma' child");
    },
};

class ViewRendererUnitTest : public juce::UnitTest
{
public:
    ViewRendererUnitTest()
        : juce::UnitTest{ "jive::Interpreter", "jive" }
    {
    }

    void runTest() final
    {
        testNestedComponents();
        testDisplayTypes();
        testInitialLayout();
        testWindowContent();
        testCustomDecorators();
        testInterpretingDifferentSources();
        testInterpretingContentAndContainers();
        testListening();
    }

private:
    void testNestedComponents()
    {
        beginTest("nested components");

        jive::Interpreter interpreter;

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

        jive::Interpreter interpreter;

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

        jive::Interpreter interpreter;
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

        jive::Interpreter interpreter;
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
        auto& window = *dynamic_cast<jive::GuiItemDecorator&>(*view)
                            .toType<jive::Window>();
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

    void testInterpretingDifferentSources()
    {
        {
            beginTest("interpreting juce::ValueTree");

            jive::Interpreter interpreter;
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

            jive::Interpreter interpreter;
            const auto result = interpreter.interpret(*juce::parseXML(R"(
                <Component width="123" height="456"/>
            )"));
            expect(result != nullptr);
        }
        {
            beginTest("interpreting juce::String");

            jive::Interpreter interpreter;
            const auto result = interpreter.interpret(R"(
                <Component width="123" height="456"/>
            )");
            expect(result != nullptr);
        }
        {
            beginTest("interpreting string data");

            jive::Interpreter interpreter;
            const juce::String source = R"(
                <Component width="123" height="456"/>
            )";
            const auto result = interpreter.interpret(source.toRawUTF8(), source.length());
            expect(result != nullptr);
        }
    }

    void testInterpretingContentAndContainers()
    {
        beginTest("Content and containers");

        jive::Interpreter interpreter;

        static const juce::ValueTree state{
            "Component",
            {
                { "width", 100 },
                { "height", 100 },
            },
            {
                juce::ValueTree{ "Component" },
                juce::ValueTree{
                    "Text",
                    {},
                    {
                        juce::ValueTree{
                            "Text",
                            {
                                { "text", "first" },
                            },
                        },
                        juce::ValueTree{
                            "Text",
                            {
                                { "text", "second" },
                            },
                        },
                        juce::ValueTree{ "Component" },
                    },
                },
            },
        };
        const auto item = interpreter.interpret(state);
        expectEquals(item->getChildren().size(), 2);
        expectEquals(item->getChildren()[1]->getChildren().size(), 2);
        expectEquals<juce::String>(item->getChildren()[1]->getChildren()[0]->state["text"], "first");
        expectEquals<juce::String>(item->getChildren()[1]->getChildren()[1]->state["text"], "second");
    }

    void testListening()
    {
        beginTest("listening");

        jive::Interpreter interpreter;
        auto item = interpreter.interpret(juce::ValueTree{
            "Component",
            {
                { "width", 100 },
                { "height", 100 },
            },
            {
                juce::ValueTree{ "Component" },
            },
        });
        expectEquals(item->getChildren().size(), 1);

        interpreter.listenTo(*item);
        item->state.appendChild(juce::ValueTree{ "Component" }, nullptr);
        expectEquals(item->getChildren().size(), 2);
    }
};

static ViewRendererUnitTest viewRendererUnitTest;
