#include <jive_layouts/jive_layouts.h>

//======================================================================================================================
#ifndef JUCE_APPLICATION_NAME
    #define JUCE_APPLICATION_NAME ""
#endif

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    Window::Window(std::unique_ptr<GuiItem> itemToDecorate)
        : GuiItemDecorator{ std::move(itemToDecorate) }
        , hasShadow{ tree, "shadow", true }
        , isNative{ tree, "native", true }
        , isResizable{ tree, "resizable", true }
        , useCornerResizer{ tree, "corner-resizer" }
        , minWidth{ tree, "min-width", 128.f }
        , minHeight{ tree, "min-height", 128.f }
        , maxWidth{ tree, "max-width", 32768.f }
        , maxHeight{ tree, "max-height", 32768.f }
        , isDraggable{ tree, "draggable", true }
        , isFullScreen{ tree, "full-screen" }
        , isMinimised{ tree, "minimised" }
        , name{ tree, "name", JUCE_APPLICATION_NAME }
        , titleBarHeight{ tree, "title-bar-height", 26 }
        , titleBarButtons{ tree, "title-bar-buttons", juce::DocumentWindow::allButtons }
    {
        static constexpr auto resizeWindowWhenViewportSizeChanges = true;
        getWindow().setContentNonOwned(&getViewport(), resizeWindowWhenViewportSizeChanges);

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

        getWindow().centreWithSize(getWindow().getWidth(), getWindow().getHeight());
    }

    //==================================================================================================================
    juce::DocumentWindow& Window::getWindow()
    {
        return window;
    }

    const juce::DocumentWindow& Window::getWindow() const
    {
        return window;
    }
} // namespace jive

//======================================================================================================================
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
    std::unique_ptr<jive::Window> createWindow(juce::ValueTree tree)
    {
        jive::Interpreter interpreter;

        return std::make_unique<jive::Window>(interpreter.interpret(tree));
    }

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
            auto item = createWindow(tree);
            expect(item->getWindow().isDropShadowEnabled());
            expect(item->getWindow().isOnDesktop());
            expect(item->getWindow().isVisible());
            expect(item->getWindow().isShowing());

            tree.setProperty("shadow", false, nullptr);
            expect(!item->getWindow().isDropShadowEnabled());
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
            auto item = createWindow(tree);
            expect(!item->getWindow().isDropShadowEnabled());
        }
    }

    void testNative()
    {
        beginTest("native");

        {
            juce::ValueTree tree{
                "Window",
                {
                    { "width", 100 },
                    { "height", 150 },
                },
            };
            auto item = createWindow(tree);
            expect(item->getWindow().isUsingNativeTitleBar());

            tree.setProperty("native", false, nullptr);
            expect(!item->getWindow().isUsingNativeTitleBar());
        }
        {
            juce::ValueTree tree{
                "Window",
                {
                    { "native", false },
                    { "width", 100 },
                    { "height", 100 },
                },
            };
            auto item = createWindow(tree);
            expect(!item->getWindow().isUsingNativeTitleBar());
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
            auto item = createWindow(tree);
            expect(item->getWindow().isResizable());

            tree.setProperty("resizable", false, nullptr);
            expect(!item->getWindow().isResizable());
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
            auto item = createWindow(tree);
            expect(!item->getWindow().isResizable());
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
            auto item = createWindow(tree);
            expectEquals(item->getWindow().getConstrainer()->getMinimumWidth(), 128);
            expectEquals(item->getWindow().getConstrainer()->getMinimumHeight(), 128);
            expectEquals(item->getWindow().getConstrainer()->getMaximumWidth(), 32768);
            expectEquals(item->getWindow().getConstrainer()->getMaximumHeight(), 32768);

            tree.setProperty("min-width", 256, nullptr);
            expectEquals(item->getWindow().getConstrainer()->getMinimumWidth(), 256);

            tree.setProperty("min-height", 256, nullptr);
            expectEquals(item->getWindow().getConstrainer()->getMinimumHeight(), 256);

            tree.setProperty("max-width", 512, nullptr);
            expectEquals(item->getWindow().getConstrainer()->getMaximumWidth(), 512);

            tree.setProperty("max-height", 512, nullptr);
            expectEquals(item->getWindow().getConstrainer()->getMaximumHeight(), 512);
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
            auto item = createWindow(tree);
            expectEquals(item->getWindow().getConstrainer()->getMinimumWidth(), 246);
            expectEquals(item->getWindow().getConstrainer()->getMinimumHeight(), 369);
            expectEquals(item->getWindow().getConstrainer()->getMaximumWidth(), 1122);
            expectEquals(item->getWindow().getConstrainer()->getMaximumHeight(), 3344);
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
            auto item = createWindow(tree);
            expect(item->getWindow().isDraggable());

            tree.setProperty("draggable", false, nullptr);
            expect(!item->getWindow().isDraggable());
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
            auto item = createWindow(tree);
            expect(!item->getWindow().isDraggable());
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
        auto item = createWindow(tree);
        expect(!item->getWindow().isFullScreen());
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
        auto item = createWindow(tree);
        expect(!item->getWindow().isMinimised());
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
            auto item = createWindow(tree);
            expectEquals(item->getWindow().getName(), juce::String{ JUCE_APPLICATION_NAME });

            tree.setProperty("name", "foo", nullptr);
            expectEquals(item->getWindow().getName(), juce::String{ "foo" });
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
            auto item = createWindow(tree);
            expectEquals(item->getWindow().getName(), juce::String{ "foo" });
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
            auto item = createWindow(tree);
            expectEquals(item->getWindow().getTitleBarHeight(), 26);

            tree.setProperty("title-bar-height", 32, nullptr);
            expectEquals(item->getWindow().getTitleBarHeight(), 32);
        }
        {
            juce::ValueTree tree{
                "Window",
                {
                    { "native", false },
                    { "title-bar-height", 100 },
                    { "width", 100 },
                    { "height", 100 },
                },
            };
            auto item = createWindow(tree);
            expectEquals(item->getWindow().getTitleBarHeight(), 100);
        }
    }
};

static WindowTest windowTest;
#endif
