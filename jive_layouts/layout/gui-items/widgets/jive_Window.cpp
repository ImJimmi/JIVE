#include <jive_layouts/jive_layouts.h>

#ifndef JUCE_APPLICATION_NAME
    #define JUCE_APPLICATION_NAME ""
#endif

namespace jive
{
    Window::Window(std::unique_ptr<GuiItem> itemToDecorate)
        : GuiItemDecorator{ std::move(itemToDecorate) }
        , hasShadow{ state, "shadow" }
        , isNative{ state, "native" }
        , isResizable{ state, "resizable" }
        , useCornerResizer{ state, "corner-resizer" }
        , minWidth{ state, "min-width" }
        , minHeight{ state, "min-height" }
        , maxWidth{ state, "max-width" }
        , maxHeight{ state, "max-height" }
        , isDraggable{ state, "draggable" }
        , isFullScreen{ state, "full-screen" }
        , isMinimised{ state, "minimised" }
        , name{ state, "name" }
        , titleBarHeight{ state, "title-bar-height" }
        , titleBarButtons{ state, "title-bar-buttons" }
        , width{ state, "width" }
        , height{ state, "height" }
    {
        if (!hasShadow.exists())
            hasShadow = true;
        if (!isNative.exists())
            isNative = true;
        if (!isResizable.exists())
            isResizable = true;
        if (!minWidth.exists())
            minWidth = 1.0f;
        if (!minHeight.exists())
            minHeight = 1.0f;
        if (!maxWidth.exists())
            maxWidth = static_cast<float>(std::numeric_limits<juce::int16>::max());
        if (!maxHeight.exists())
            maxHeight = static_cast<float>(std::numeric_limits<juce::int16>::max());
        if (!isDraggable.exists())
            isDraggable = true;
        if (!name.exists())
            name = JUCE_APPLICATION_NAME;
        if (!titleBarHeight.exists())
            titleBarHeight = 26;
        if (!titleBarButtons.exists())
            titleBarButtons = juce::DocumentWindow::allButtons;

        hasShadow.onValueChange = [this]() {
            getWindow().setDropShadowEnabled(hasShadow);
        };
        getWindow().setDropShadowEnabled(hasShadow);

        isNative.onValueChange = [this]() {
            getWindow().setUsingNativeTitleBar(isNative);
        };
        getWindow().setUsingNativeTitleBar(isNative);

        isResizable.onValueChange = [this]() {
            getWindow().setResizable(isResizable, useCornerResizer);
        };
        useCornerResizer.onValueChange = [this]() {
            getWindow().setResizable(isResizable, useCornerResizer);
        };
        getWindow().setResizable(isResizable, useCornerResizer);

        minWidth.onValueChange = [this]() {
            getWindow().setResizeLimits(static_cast<int>(std::ceil(minWidth)),
                                        static_cast<int>(std::ceil(minHeight)),
                                        static_cast<int>(std::floor(maxWidth)),
                                        static_cast<int>(std::floor(maxHeight)));
        };
        minHeight.onValueChange = [this]() {
            getWindow().setResizeLimits(static_cast<int>(std::ceil(minWidth)),
                                        static_cast<int>(std::ceil(minHeight)),
                                        static_cast<int>(std::floor(maxWidth)),
                                        static_cast<int>(std::floor(maxHeight)));
        };
        maxWidth.onValueChange = [this]() {
            getWindow().setResizeLimits(static_cast<int>(std::ceil(minWidth)),
                                        static_cast<int>(std::ceil(minHeight)),
                                        static_cast<int>(std::floor(maxWidth)),
                                        static_cast<int>(std::floor(maxHeight)));
        };
        maxHeight.onValueChange = [this]() {
            getWindow().setResizeLimits(static_cast<int>(std::ceil(minWidth)),
                                        static_cast<int>(std::ceil(minHeight)),
                                        static_cast<int>(std::floor(maxWidth)),
                                        static_cast<int>(std::floor(maxHeight)));
        };
        getWindow().setResizeLimits(static_cast<int>(std::ceil(minWidth)),
                                    static_cast<int>(std::ceil(minHeight)),
                                    static_cast<int>(std::floor(maxWidth)),
                                    static_cast<int>(std::floor(maxHeight)));

        isDraggable.onValueChange = [this]() {
            getWindow().setDraggable(isDraggable);
        };
        getWindow().setDraggable(isDraggable);

        isFullScreen.onValueChange = [this]() {
            getWindow().setFullScreen(isFullScreen);
        };
        getWindow().setFullScreen(isFullScreen);

        isMinimised.onValueChange = [this]() {
            getWindow().setMinimised(isMinimised);
        };
        getWindow().setMinimised(isMinimised);

        name.onValueChange = [this]() {
            getWindow().setName(name);
        };
        getWindow().setName(name);

        titleBarHeight.onValueChange = [this]() {
            getWindow().setTitleBarHeight(juce::roundToInt(titleBarHeight));
        };
        getWindow().setTitleBarHeight(juce::roundToInt(titleBarHeight));

#if JUCE_MAC
        static constexpr auto leftAlignButtons = true;
#else
        static constexpr auto leftAlignButtons = false;
#endif

        titleBarButtons.onValueChange = [this]() {
            getWindow().setTitleBarButtonsRequired(titleBarButtons, leftAlignButtons);
        };
        getWindow().setTitleBarButtonsRequired(titleBarButtons, leftAlignButtons);

        const auto& boxModel = toType<CommonGuiItem>()->boxModel;
        const auto windowWidth = juce::roundToInt(boxModel.getWidth());
        const auto windowHeight = juce::roundToInt(boxModel.getHeight());

        static constexpr auto resizeWindowWhenComponentSizeChanges = true;
        getWindow().setContentNonOwned(component.get(), resizeWindowWhenComponentSizeChanges);
        getWindow().centreWithSize(windowWidth, windowHeight);
        getWindow().setLookAndFeel(&lookAndFeel);
    }

    Window::~Window()
    {
        getWindow().setLookAndFeel(nullptr);
    }

    juce::DocumentWindow& Window::getWindow()
    {
        return window;
    }

    const juce::DocumentWindow& Window::getWindow() const
    {
        return window;
    }
} // namespace jive

#if JIVE_UNIT_TESTS
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
            auto& window = dynamic_cast<jive::GuiItemDecorator&>(*item)
                               .toType<jive::Window>()
                               ->getWindow();
            expect(window.isDropShadowEnabled());
            expect(window.isOnDesktop());
            expect(window.isVisible());
            expect(window.isShowing());

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
            auto& window = dynamic_cast<jive::GuiItemDecorator&>(*item)
                               .toType<jive::Window>()
                               ->getWindow();
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
                    { "width", 100 },
                    { "height", 150 },
                },
            };
            jive::Interpreter interpreter;
            auto item = interpreter.interpret(state);
            auto& window = dynamic_cast<jive::GuiItemDecorator&>(*item)
                               .toType<jive::Window>()
                               ->getWindow();
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
            auto& window = dynamic_cast<jive::GuiItemDecorator&>(*item)
                               .toType<jive::Window>()
                               ->getWindow();
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
            auto& window = dynamic_cast<jive::GuiItemDecorator&>(*item)
                               .toType<jive::Window>()
                               ->getWindow();
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
            auto& window = dynamic_cast<jive::GuiItemDecorator&>(*item)
                               .toType<jive::Window>()
                               ->getWindow();
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
            auto& window = dynamic_cast<jive::GuiItemDecorator&>(*item)
                               .toType<jive::Window>()
                               ->getWindow();
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
            auto& window = dynamic_cast<jive::GuiItemDecorator&>(*item)
                               .toType<jive::Window>()
                               ->getWindow();
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
            auto& window = dynamic_cast<jive::GuiItemDecorator&>(*item)
                               .toType<jive::Window>()
                               ->getWindow();
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
            auto& window = dynamic_cast<jive::GuiItemDecorator&>(*item)
                               .toType<jive::Window>()
                               ->getWindow();
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
        auto& window = dynamic_cast<jive::GuiItemDecorator&>(*item)
                           .toType<jive::Window>()
                           ->getWindow();
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
        auto& window = dynamic_cast<jive::GuiItemDecorator&>(*item)
                           .toType<jive::Window>()
                           ->getWindow();
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
            auto& window = dynamic_cast<jive::GuiItemDecorator&>(*item)
                               .toType<jive::Window>()
                               ->getWindow();
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
            auto& window = dynamic_cast<jive::GuiItemDecorator&>(*item)
                               .toType<jive::Window>()
                               ->getWindow();
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
            auto& window = dynamic_cast<jive::GuiItemDecorator&>(*item)
                               .toType<jive::Window>()
                               ->getWindow();
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
            auto& window = dynamic_cast<jive::GuiItemDecorator&>(*item)
                               .toType<jive::Window>()
                               ->getWindow();
            expectEquals(window.getTitleBarHeight(), 100);
        }
    }
};

static WindowTest windowTest;
#endif
