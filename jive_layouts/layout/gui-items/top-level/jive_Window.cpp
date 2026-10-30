#include "jive_Window.h"

#include <jive_layouts/layout/gui-items/jive_CommonGuiItem.h>

#ifndef JUCE_APPLICATION_NAME
    #define JUCE_APPLICATION_NAME ""
#endif

namespace jive
{
    Window::Window(std::unique_ptr<GuiItem> itemToDecorate)
        : TopLevelGuiItem{ std::move(itemToDecorate) }
        , hasShadow{ state, ids::shadow() }
        , isNative{ state, ids::native() }
        , canBeResized{ state, ids::resizable() }
        , useCornerResizer{ state, ids::cornerResizer() }
        , minWidth{ state, ids::minWidth() }
        , minHeight{ state, ids::minHeight() }
        , maxWidth{ state, ids::maxWidth() }
        , maxHeight{ state, ids::maxHeight() }
        , draggable{ state, ids::draggable() }
        , fullScreen{ state, ids::fullScreen() }
        , minimised{ state, ids::minimised() }
        , visibility{ state, ids::visibility() }
        , name{ state, ids::name() }
        , windowTitleBarHeight{ state, ids::titleBarHeight() }
        , windowTitleBarButtons{ state, ids::titleBarButtons() }
        , width{ state, ids::width() }
        , height{ state, ids::height() }
    {
        state.addListener(this);

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

        if (state.hasProperty(ids::theme()))
            applyTheme();

        const auto& boxModel = toType<CommonGuiItem>()->boxModel;
        const auto windowWidth = juce::roundToInt(boxModel.getWidth());
        const auto windowHeight = juce::roundToInt(boxModel.getHeight());
        centreWithSize(windowWidth, windowHeight);

        // Shown once the code that created the window has finished setting it
        // up, so that its first frame is complete.
        juce::MessageManager::callAsync([safeThis = juce::Component::SafePointer<Window>{ this }] {
            if (safeThis != nullptr)
                safeThis->setVisible(safeThis->visibility.getOr(true));
        });
    }

    void Window::replaceDecoratedItem(std::unique_ptr<GuiItem> newItem)
    {
        item = std::move(newItem);
        state = item->state;

        component = item->getComponent();
        component->setLookAndFeel(&jiveLookAndFeel);
        applyTheme();

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
        state.removeListener(this);
        setLookAndFeel(nullptr);
        getComponent()->setLookAndFeel(nullptr);
    }

    void Window::applyTheme()
    {
        jiveLookAndFeel.applyTheme(fromVar<Theme>(state.getProperty(ids::theme(), toVar(Theme::steel))));
        getComponent()->sendLookAndFeelChange();
    }

    void Window::valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property)
    {
        if (tree == state && property == ids::theme())
            applyTheme();

        forwardPropertyChange(tree,
                              property,
                              hasShadow,
                              isNative,
                              canBeResized,
                              useCornerResizer,
                              minWidth,
                              minHeight,
                              maxWidth,
                              maxHeight,
                              draggable,
                              fullScreen,
                              minimised,
                              visibility,
                              name,
                              windowTitleBarHeight,
                              windowTitleBarButtons);
    }
} // namespace jive

#if JIVE_UNIT_TESTS
    #include <jive_layouts/layout/interpreter/jive_Interpreter.h>
    #include <jive_style_sheets/graphics/themes/jive_MaterialTheme.h>
    #include <jive_style_sheets/graphics/themes/jive_SteelTheme.h>
    #include <jive_style_sheets/graphics/themes/jive_SynthwaveTheme.h>

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
        testTheme();
        testShownOnceCreated();
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

    // Theme changes transition, so this settles them before reading the
    // background.
    static auto settledBackground(const juce::Component& component)
    {
        const auto& lookAndFeel = dynamic_cast<const jive::LookAndFeel&>(component.getLookAndFeel());
        juce::ignoreUnused(lookAndFeel.findMostApplicableStyles(component));
        jive::FakeTime::incrementTime(juce::RelativeTime::seconds(1.0));

        return lookAndFeel
            .findMostApplicableStyles(component)
            .find<jive::Fill>("background")
            .value_or(jive::Fill{});
    }

    void testTheme()
    {
        beginTest("theme");

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
            expect(settledBackground(*item->getComponent()) == jive::parseColour(jive::themes::steel::inkBlack));

            tree.setProperty("theme", "material", nullptr);
            expect(settledBackground(*item->getComponent()) == jive::parseColour(jive::themes::material::surface));
        }
        {
            juce::ValueTree tree{
                "Window",
                {
                    { "theme", "synthwave" },
                    { "width", 100 },
                    { "height", 100 },
                },
            };
            jive::Interpreter interpreter;
            auto item = interpreter.interpret(tree);
            expect(settledBackground(*item->getComponent()) == jive::Fill{ jive::themes::synthwave::duskGradient });
        }
        {
            juce::ValueTree tree{
                "Window",
                {
                    { "theme", "juce" },
                    { "width", 100 },
                    { "height", 100 },
                },
            };
            jive::Interpreter interpreter;
            auto item = interpreter.interpret(tree);
            auto& window = *dynamic_cast<jive::GuiItemDecorator&>(*item)
                                .toType<jive::Window>();
            expect(window.getBackgroundColour() == jive::detail::getV4Colour(jive::detail::V4Colour::windowBackground));
        }
    }

    void testShownOnceCreated()
    {
        beginTest("a window isn't shown until the code that created it has finished, and is painted, fully laid out, as it's shown");

        struct PaintCounter : public juce::Component
        {
            void paint(juce::Graphics&) final
            {
                if (numPaints++ == 0)
                    sizeWhenFirstPainted = getLocalBounds();
            }

            int numPaints = 0;
            juce::Rectangle<int> sizeWhenFirstPainted;
        };

        jive::Interpreter interpreter;
        std::shared_ptr<PaintCounter> paintCounter;
        interpreter.getComponentFactory().set("PaintCounter", [&paintCounter] {
            paintCounter = std::make_shared<PaintCounter>();
            return paintCounter;
        });
        const auto item = interpreter.interpret(jive::parseXML(R"(
            <Window width="100" height="100">
                <PaintCounter width="40" height="30" />
            </Window>
        )"));
        auto& window = *dynamic_cast<jive::GuiItemDecorator&>(*item)
                            .toType<jive::Window>();
        expect(!window.isVisible());

        window.setVisible(true);
        expectGreaterThan(paintCounter->numPaints, 0);
        expectEquals(paintCounter->sizeWhenFirstPainted, juce::Rectangle<int>{ 40, 30 });
    }
};

static WindowTest windowTest;
#endif
