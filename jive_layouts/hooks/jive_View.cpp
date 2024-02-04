#include <jive_layouts/jive_layouts.h>

namespace jive
{
    View::operator juce::ValueTree()
    {
        return toValueTree();
    }

    juce::ValueTree View::toValueTree()
    {
        return initialise()
            .setProperty("view-object", this, nullptr);
    }
} // namespace jive

#if JIVE_UNIT_TESTS
class ViewTest : public juce::UnitTest
{
public:
    ViewTest()
        : juce::UnitTest{ "jive::View", "jive" }
    {
    }

    void runTest() final
    {
        testBasicView();
        testGuiItemInstantiated();
        testCustomComponents();
    }

private:
    void testBasicView()
    {
        struct View : public jive::View
        {
            [[nodiscard]] juce::ValueTree initialise() final
            {
                return juce::ValueTree{
                    "Component",
                    {
                        { "width", 300 },
                        { "height", 200 },
                    },
                };
            }
        };

        jive::Interpreter interpreter;

        beginTest("basic view");
        {
            const auto window = interpreter.interpret(jive::makeView<View>());
            expect(window != nullptr);
            expectEquals(window->getComponent()->getWidth(), 300);
            expectEquals(window->getComponent()->getHeight(), 200);
        }

        beginTest("basic nested view");
        {
            juce::ValueTree state{
                "Window",
                {
                    { "width", 300 },
                    { "height", 200 },
                },
                {
                    *jive::makeView<View>(),
                },
            };
            const auto window = interpreter.interpret(state);
            expect(window != nullptr);
            expectEquals(window->getComponent()->getNumChildComponents(), 1);
        }
    }

    void testGuiItemInstantiated()
    {
        struct SpyView : public jive::View
        {
            [[nodiscard]] juce::ValueTree initialise() final
            {
                return juce::ValueTree{
                    "Window",
                    {
                        { "width", 300 },
                        { "height", 200 },
                        { "native", true },
                    },
                };
            }

            void setup(jive::GuiItem& item) final
            {
                setupCallCount++;
                lastItemPassedToCallback = &item;
            }

            int setupCallCount = 0;
            jive::GuiItem* lastItemPassedToCallback = nullptr;
        };

        beginTest("setup()");
        jive::Interpreter interpreter;
        const auto window = interpreter.interpret(jive::makeView<SpyView>());
        expect(window != nullptr);
        auto* view = dynamic_cast<SpyView*>(window->state["view-object"].getObject());
        expectEquals(view->setupCallCount, 1);
        expect(view->lastItemPassedToCallback == window.get());
    }

    void testCustomComponents()
    {
        class MyComponent : public juce::Component
        {
        };

        struct MyView : public jive::View
        {
            [[nodiscard]] juce::ValueTree initialise() final
            {
                return juce::ValueTree{
                    "Window",
                    {
                        { "width", 300 },
                        { "height", 200 },
                    },
                    {
                        juce::ValueTree{ "YourComponent" },
                        juce::ValueTree{ "MyComponent" },
                        juce::ValueTree{ "TheirComponent" },
                    },
                };
            }

            std::unique_ptr<juce::Component> createComponent(const juce::ValueTree& tree)
            {
                if (tree.getType().toString() == "MyComponent")
                    return std::make_unique<MyComponent>();

                return nullptr;
            }
        };

        beginTest("custom components");
        jive::Interpreter interpreter;
        const auto item = interpreter.interpret(jive::makeView<MyView>());
        expect(item != nullptr);
        const auto children = item->getChildren();
        expectEquals(children.size(), 1);
        expect(dynamic_cast<MyComponent*>(item->getChildren()[0]->getComponent().get()) != nullptr);
    }
};

static const ViewTest viewTest;
#endif
