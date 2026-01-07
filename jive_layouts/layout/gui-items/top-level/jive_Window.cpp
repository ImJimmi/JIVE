#include "jive_Window.h"

#include <jive_layouts/layout/gui-items/jive_CommonGuiItem.h>

#ifndef JUCE_APPLICATION_NAME
    #define JUCE_APPLICATION_NAME ""
#endif

namespace jive
{
    Window::Window(std::unique_ptr<GuiItem> itemToDecorate)
        : TopLevelGuiItem{ std::move(itemToDecorate) }
        , hasShadow{ state, "shadow" }
        , isNative{ state, "native" }
        , canBeResized{ state, "resizable" }
        , useCornerResizer{ state, "corner-resizer" }
        , minWidth{ state, "min-width" }
        , minHeight{ state, "min-height" }
        , maxWidth{ state, "max-width" }
        , maxHeight{ state, "max-height" }
        , draggable{ state, "draggable" }
        , fullScreen{ state, "full-screen" }
        , minimised{ state, "minimised" }
        , visibility{ state, "visibility" }
        , name{ state, "name" }
        , windowTitleBarHeight{ state, "title-bar-height" }
        , windowTitleBarButtons{ state, "title-bar-buttons" }
        , width{ state, "width" }
        , height{ state, "height" }
    {
        const BoxModel::ScopedCallbackLock boxModelLock{ boxModel(*this) };

        hasShadow.onValueChange = [this]() {
            setDropShadowEnabled(hasShadow.getOr(true));
        };
        setDropShadowEnabled(hasShadow.getOr(true));

#if JIVE_UNIT_TESTS
        static constexpr auto nativeByDefault = false;
#else
        static constexpr auto nativeByDefault = true;
#endif
        isNative.onValueChange = [this]() {
            setUsingNativeTitleBar(isNative.getOr(nativeByDefault));
        };
        setUsingNativeTitleBar(isNative.getOr(nativeByDefault));

        canBeResized.onValueChange = [this]() {
            setResizable(canBeResized.getOr(true), useCornerResizer.getOr(false));
        };
        useCornerResizer.onValueChange = [this]() {
            setResizable(canBeResized.getOr(true), useCornerResizer.getOr(false));
        };
        setResizable(canBeResized.getOr(true), useCornerResizer.getOr(false));

        const auto updateResizeLimits = [this]() {
            setResizeLimits(static_cast<int>(std::ceil(minWidth.getOr(1.0f))),
                            static_cast<int>(std::ceil(minHeight.getOr(1.0f))),
                            static_cast<int>(std::floor(maxWidth.getOr(static_cast<float>(std::numeric_limits<juce::int16>::max())))),
                            static_cast<int>(std::floor(maxHeight.getOr(static_cast<float>(std::numeric_limits<juce::int16>::max())))));
        };
        minWidth.onValueChange = updateResizeLimits;
        minHeight.onValueChange = updateResizeLimits;
        maxWidth.onValueChange = updateResizeLimits;
        maxHeight.onValueChange = updateResizeLimits;
        updateResizeLimits();

        draggable.onValueChange = [this]() {
            setDraggable(draggable.getOr(true));
        };
        setDraggable(draggable.getOr(true));

        fullScreen.onValueChange = [this]() {
            setFullScreen(fullScreen);
        };
        setFullScreen(fullScreen);

        minimised.onValueChange = [this]() {
            setMinimised(minimised);
        };
        setMinimised(minimised);

        visibility.onValueChange = [this]() {
            setVisible(visibility.getOr(true));
        };
        setVisible(visibility.getOr(true));

        name.onValueChange = [this]() {
            setName(name.getOr(JUCE_APPLICATION_NAME));
        };
        setName(name.getOr(JUCE_APPLICATION_NAME));

        windowTitleBarHeight.onValueChange = [this]() {
            setTitleBarHeight(juce::roundToInt(windowTitleBarHeight.getOr(26)));
        };
        setTitleBarHeight(juce::roundToInt(windowTitleBarHeight.getOr(26)));

#if JUCE_MAC
        static constexpr auto leftAlignButtons = true;
#else
        static constexpr auto leftAlignButtons = false;
#endif

        windowTitleBarButtons.onValueChange = [this]() {
            setTitleBarButtonsRequired(windowTitleBarButtons.getOr(juce::DocumentWindow::allButtons), leftAlignButtons);
        };
        setTitleBarButtonsRequired(windowTitleBarButtons.getOr(juce::DocumentWindow::allButtons), leftAlignButtons);

        static constexpr auto resizeWindowWhenComponentSizeChanges = true;
        setContentNonOwned(getComponent().get(),
                           resizeWindowWhenComponentSizeChanges);

        const auto& boxModel = toType<CommonGuiItem>()->boxModel;
        const auto windowWidth = juce::roundToInt(boxModel.getWidth());
        const auto windowHeight = juce::roundToInt(boxModel.getHeight());
        centreWithSize(windowWidth, windowHeight);
    }

    void Window::replaceDecoratedItem(std::unique_ptr<GuiItem> newItem)
    {
        item = std::move(newItem);
        state = item->state;

        component = item->getComponent();
        component->setLookAndFeel(&jiveLookAndFeel);

        static constexpr auto resizeWindowWhenComponentSizeChanges = true;
        setContentNonOwned(getComponent().get(),
                           resizeWindowWhenComponentSizeChanges);

        const auto& boxModel = toType<CommonGuiItem>()->boxModel;
        const auto windowWidth = juce::roundToInt(boxModel.getWidth());
        const auto windowHeight = juce::roundToInt(boxModel.getHeight());
        setBounds(getBounds().withSize(windowWidth, windowHeight));
    }

    Window::~Window()
    {
        setLookAndFeel(nullptr);
        getComponent()->setLookAndFeel(nullptr);
    }
} // namespace jive

#if JIVE_UNIT_TESTS
    #include <jive_layouts/layout/interpreter/jive_Interpreter.h>

class WindowTest : public juce::UnitTest
{
public:
    WindowTest()
        : juce::UnitTest{ "jive::Window", "jive" }
    {
    }

    void runTest() final
    {
        testShadow();
        testNative();
        testResizable();
        testSizeLimits();
        testDraggable();
        testFullScreen();
        testMinimised();
        testName();
        testTitleBarHeight();
    }

private:
    void testShadow()
    {
        beginTest("shadow");

        {
            juce::ValueTree tree{
                "Window",
                {
                    { "width", 100 },
                    { "height", 100 },
                },
            };
            jive::Interpreter interpreter;
            auto item = interpreter.interpret(tree);
            auto& window = *dynamic_cast<jive::GuiItemDecorator&>(*item)
                                .toType<jive::Window>();
            expect(window.isDropShadowEnabled());
            expect(window.isOnDesktop());

            tree.setProperty("shadow", false, nullptr);
            expect(!window.isDropShadowEnabled());
        }
        {
            juce::ValueTree tree{
                "Window",
                {
                    { "shadow", false },
                    { "width", 100 },
                    { "height", 100 },
                },
            };
            jive::Interpreter interpreter;
            auto item = interpreter.interpret(tree);
            auto& window = *dynamic_cast<jive::GuiItemDecorator&>(*item)
                                .toType<jive::Window>();
            expect(!window.isDropShadowEnabled());
        }
    }

    void testNative()
    {
        beginTest("native");

        {
            juce::ValueTree state{
                "Window",
                {
                    { "native", true },
                    { "width", 100 },
                    { "height", 150 },
                },
            };
            jive::Interpreter interpreter;
            auto item = interpreter.interpret(state);
            auto& window = *dynamic_cast<jive::GuiItemDecorator&>(*item)
                                .toType<jive::Window>();
            expect(window.isUsingNativeTitleBar());
            expectEquals(item->getComponent()->getWidth(), 100);
            expectEquals(item->getComponent()->getHeight(), 150);
        }
        {
            juce::ValueTree state{
                "Window",
                {
                    { "native", false },
                    { "width", 338 },
                    { "height", 894 },
                },
            };
            jive::Interpreter interpreter;
            auto item = interpreter.interpret(state);
            auto& window = *dynamic_cast<jive::GuiItemDecorator&>(*item)
                                .toType<jive::Window>();
            expect(!window.isUsingNativeTitleBar());
            expectEquals(item->getComponent()->getWidth(),
                         338 - window.getBorderThickness().getLeftAndRight());
            expectEquals(item->getComponent()->getHeight(),
                         894 - window.getBorderThickness().getTopAndBottom() - window.getTitleBarHeight());
        }
    }

    void testResizable()
    {
        beginTest("resizable");

        {
            juce::ValueTree tree{
                "Window",
                {
                    { "width", 100 },
                    { "height", 100 },
                },
            };
            jive::Interpreter interpreter;
            auto item = interpreter.interpret(tree);
            auto& window = *dynamic_cast<jive::GuiItemDecorator&>(*item)
                                .toType<jive::Window>();
            expect(window.isResizable());

            tree.setProperty("resizable", false, nullptr);
            expect(!window.isResizable());
        }
        {
            juce::ValueTree tree{
                "Window",
                {
                    { "resizable", false },
                    { "width", 100 },
                    { "height", 100 },
                },
            };
            jive::Interpreter interpreter;
            auto item = interpreter.interpret(tree);
            auto& window = *dynamic_cast<jive::GuiItemDecorator&>(*item)
                                .toType<jive::Window>();
            expect(!window.isResizable());
        }
    }

    void testSizeLimits()
    {
        beginTest("size limits");

        {
            juce::ValueTree tree{
                "Window",
                {
                    { "width", 100 },
                    { "height", 100 },
                },
            };
            jive::Interpreter interpreter;
            auto item = interpreter.interpret(tree);
            auto& window = *dynamic_cast<jive::GuiItemDecorator&>(*item)
                                .toType<jive::Window>();
            expectEquals(window.getConstrainer()->getMinimumWidth(), 1);
            expectEquals(window.getConstrainer()->getMinimumHeight(), 1);
            expectEquals<int>(window.getConstrainer()->getMaximumWidth(),
                              std::numeric_limits<juce::int16>::max());
            expectEquals<int>(window.getConstrainer()->getMaximumHeight(),
                              std::numeric_limits<juce::int16>::max());

            tree.setProperty("min-width", 256, nullptr);
            expectEquals(window.getConstrainer()->getMinimumWidth(), 256);

            tree.setProperty("min-height", 256, nullptr);
            expectEquals(window.getConstrainer()->getMinimumHeight(), 256);

            tree.setProperty("max-width", 512, nullptr);
            expectEquals(window.getConstrainer()->getMaximumWidth(), 512);

            tree.setProperty("max-height", 512, nullptr);
            expectEquals(window.getConstrainer()->getMaximumHeight(), 512);
        }
        {
            juce::ValueTree tree{
                "Window",
                {
                    { "min-width", 246 },
                    { "min-height", 369 },
                    { "max-width", 1122 },
                    { "max-height", 3344 },
                    { "width", 100 },
                    { "height", 100 },
                },
            };
            jive::Interpreter interpreter;
            auto item = interpreter.interpret(tree);
            auto& window = *dynamic_cast<jive::GuiItemDecorator&>(*item)
                                .toType<jive::Window>();
            expectEquals(window.getConstrainer()->getMinimumWidth(), 246);
            expectEquals(window.getConstrainer()->getMinimumHeight(), 369);
            expectEquals(window.getConstrainer()->getMaximumWidth(), 1122);
            expectEquals(window.getConstrainer()->getMaximumHeight(), 3344);
        }
    }

    void testDraggable()
    {
        beginTest("draggable");

        {
            juce::ValueTree tree{
                "Window",
                {
                    { "width", 100 },
                    { "height", 100 },
                },
            };
            jive::Interpreter interpreter;
            auto item = interpreter.interpret(tree);
            auto& window = *dynamic_cast<jive::GuiItemDecorator&>(*item)
                                .toType<jive::Window>();
            expect(window.isDraggable());

            tree.setProperty("draggable", false, nullptr);
            expect(!window.isDraggable());
        }
        {
            juce::ValueTree tree{
                "Window",
                {
                    { "draggable", false },
                    { "width", 100 },
                    { "height", 100 },
                },
            };
            jive::Interpreter interpreter;
            auto item = interpreter.interpret(tree);
            auto& window = *dynamic_cast<jive::GuiItemDecorator&>(*item)
                                .toType<jive::Window>();
            expect(!window.isDraggable());
        }
    }

    void testFullScreen()
    {
        beginTest("full-screen");

        juce::ValueTree tree{
            "Window",
            {
                { "width", 100 },
                { "height", 100 },
            },
        };
        jive::Interpreter interpreter;
        auto item = interpreter.interpret(tree);
        auto& window = *dynamic_cast<jive::GuiItemDecorator&>(*item)
                            .toType<jive::Window>();
        expect(!window.isFullScreen());
    }

    void testMinimised()
    {
        beginTest("minimised");

        juce::ValueTree tree{
            "Window",
            {
                { "width", 100 },
                { "height", 100 },
            },
        };
        jive::Interpreter interpreter;
        auto item = interpreter.interpret(tree);
        auto& window = *dynamic_cast<jive::GuiItemDecorator&>(*item)
                            .toType<jive::Window>();
        expect(!window.isMinimised());
    }

    void testName()
    {
        beginTest("name");

        {
            juce::ValueTree tree{
                "Window",
                {
                    { "width", 100 },
                    { "height", 100 },
                },
            };
            jive::Interpreter interpreter;
            auto item = interpreter.interpret(tree);
            auto& window = *dynamic_cast<jive::GuiItemDecorator&>(*item)
                                .toType<jive::Window>();
            expectEquals(window.getName(), juce::String{ JUCE_APPLICATION_NAME });

            tree.setProperty("name", "foo", nullptr);
            expectEquals(window.getName(), juce::String{ "foo" });
        }
        {
            juce::ValueTree tree{
                "Window",
                {
                    { "name", "foo" },
                    { "width", 100 },
                    { "height", 100 },
                },
            };
            jive::Interpreter interpreter;
            auto item = interpreter.interpret(tree);
            auto& window = *dynamic_cast<jive::GuiItemDecorator&>(*item)
                                .toType<jive::Window>();
            expectEquals(window.getName(), juce::String{ "foo" });
        }
    }

    void testTitleBarHeight()
    {
        beginTest("title-bar-height");

        {
            juce::ValueTree tree{
                "Window",
                {
                    { "native", false },
                    { "width", 100 },
                    { "height", 100 },
                },
            };
            jive::Interpreter interpreter;
            auto item = interpreter.interpret(tree);
            auto& window = *dynamic_cast<jive::GuiItemDecorator&>(*item)
                                .toType<jive::Window>();
            expectEquals(window.getTitleBarHeight(), 26);

            tree.setProperty("title-bar-height", 32, nullptr);
            expectEquals(window.getTitleBarHeight(), 32);
        }
        {
            juce::ValueTree tree{
                "Window",
                {
                    { "native", false },
                    { "title-bar-height", 100 },
                    { "width", 100 },
                    { "height", 222 },
                },
            };
            jive::Interpreter interpreter;
            auto item = interpreter.interpret(tree);
            auto& window = *dynamic_cast<jive::GuiItemDecorator&>(*item)
                                .toType<jive::Window>();
            expectEquals(window.getTitleBarHeight(), 100);
        }
    }
};

static WindowTest windowTest;
#endif
