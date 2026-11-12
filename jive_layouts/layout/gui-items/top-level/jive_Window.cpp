#include "jive_Window.h"

#include <jive_layouts/layout/gui-items/jive_CommonGuiItem.h>

#if JUCE_MAC
    #include "jive_Window_mac.h"
#endif

#ifndef JUCE_APPLICATION_NAME
    #define JUCE_APPLICATION_NAME ""
#endif

namespace jive
{
    Window::Window(std::unique_ptr<GuiItem> itemToDecorate)
        : TopLevelGuiItem{ std::move(itemToDecorate) }
        , integratedTitleBar{ state, ids::integratedTitleBar() }
        , windowTitleBarHeight{ state, ids::titleBarHeight() }
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

        const auto onSizeLimitChange = [this]() {
            updateResizeLimits();
        };
        minWidth.onValueChange = onSizeLimitChange;
        minHeight.onValueChange = onSizeLimitChange;
        maxWidth.onValueChange = onSizeLimitChange;
        maxHeight.onValueChange = onSizeLimitChange;
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
            updateIntegratedTitleBar();
        };
        setTitleBarHeight(juce::roundToInt(windowTitleBarHeight.getOr(26)));

        integratedTitleBar.onValueChange = [this]() {
            updateIntegratedTitleBar();
        };

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

        getComponent()->addMouseListener(&titleBarDragger, true);
        updateIntegratedTitleBar();

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

        component->removeMouseListener(&titleBarDragger);
        component = item->getComponent();
        component->setLookAndFeel(&jiveLookAndFeel);
        component->addMouseListener(&titleBarDragger, true);
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
        getComponent()->removeMouseListener(&titleBarDragger);
        getComponent()->setLookAndFeel(nullptr);
    }

    void Window::paint(juce::Graphics& g)
    {
        DocumentWindow::paint(g);

#if JUCE_MAC
        // A new peer is created whenever the window's style changes. Its title
        // bar is configured after this paint rather than during it.
        if (auto* peer = getPeer(); peer != nullptr && peer->getNativeHandle() != integratedTitleBarHandle)
        {
            integratedTitleBarHandle = peer->getNativeHandle();
            juce::MessageManager::callAsync([safeThis = juce::Component::SafePointer<Window>{ this }] {
                if (safeThis != nullptr)
                    safeThis->updateIntegratedTitleBar();
            });
        }
#endif

        updateTitleBarColour();
    }

    void Window::resized()
    {
        DocumentWindow::resized();

#if JUCE_MAC
        if (integratedTitleBar.getOr(true) && windowTitleBarHeight.exists())
        {
            if (auto* peer = getPeer())
                macos::layoutTitleBarButtons(peer->getNativeHandle(), windowTitleBarHeight.get());
        }
#endif
    }

    void Window::updateIntegratedTitleBar()
    {
#if JUCE_MAC
        if (auto* peer = getPeer(); peer != nullptr && isUsingNativeTitleBar())
        {
            macos::setIntegratedTitleBar(peer->getNativeHandle(), integratedTitleBar.getOr(true));
            resized();
        }
#endif
    }

#if JUCE_WINDOWS
    [[nodiscard]] static std::optional<juce::Colour> toColour(const std::optional<Fill>& fill)
    {
        if (!fill.has_value())
            return std::nullopt;

        if (const auto gradient = fill->getJiveGradient())
            return gradient->toJuceColourGradient().getColour(0);

        return fill->toJuceFillType().colour;
    }
#endif

    void Window::updateTitleBarColour()
    {
#if JUCE_WINDOWS
        auto* const peer = getPeer();

        if (peer == nullptr || !isUsingNativeTitleBar())
            return;

        const auto colour = toColour(jiveLookAndFeel.findMostApplicableStyles(*getComponent()).find<Fill>("title-bar"));

        if (colour == appliedTitleBarColour && peer->getNativeHandle() == titleBarColourHandle)
            return;

        using SetWindowAttribute = long(__stdcall*)(void*, unsigned long, const void*, unsigned long);
        static juce::DynamicLibrary dwmapi{ "dwmapi.dll" };
        static const auto setWindowAttribute = reinterpret_cast<SetWindowAttribute>(dwmapi.getFunction("DwmSetWindowAttribute"));

        if (setWindowAttribute == nullptr)
            return;

        static constexpr unsigned long captionColourAttribute = 35;
        static constexpr unsigned long defaultColour = 0xFFFFFFFF;
        const auto colourRef = colour.has_value()
                                 ? static_cast<unsigned long>(colour->getRed())
                                       | (static_cast<unsigned long>(colour->getGreen()) << 8)
                                       | (static_cast<unsigned long>(colour->getBlue()) << 16)
                                 : defaultColour;
        setWindowAttribute(peer->getNativeHandle(), captionColourAttribute, &colourRef, sizeof(colourRef));

        appliedTitleBarColour = colour;
        titleBarColourHandle = peer->getNativeHandle();
#endif
    }

    Window::TitleBarDragger::TitleBarDragger(Window& owner)
        : window{ owner }
    {
    }

#if JUCE_MAC
    [[nodiscard]] static bool isInteractive(const juce::Component* component, const juce::Component& root)
    {
        for (; component != nullptr && component != &root; component = component->getParentComponent())
        {
            if (dynamic_cast<const juce::Button*>(component) != nullptr
                || dynamic_cast<const juce::Slider*>(component) != nullptr
                || dynamic_cast<const juce::ComboBox*>(component) != nullptr
                || dynamic_cast<const juce::TextEditor*>(component) != nullptr
                || dynamic_cast<const juce::ScrollBar*>(component) != nullptr)
                return true;
        }

        return false;
    }
#endif

    void Window::TitleBarDragger::mouseDown(const juce::MouseEvent& event)
    {
#if JUCE_MAC
        static constexpr auto defaultTitleBarHeight = 28.0f;
        const auto& content = *window.getComponent();

        if (!window.integratedTitleBar.getOr(true)
            || !window.isUsingNativeTitleBar()
            || event.getEventRelativeTo(window.getComponent().get()).position.y >= window.windowTitleBarHeight.getOr(defaultTitleBarHeight)
            || isInteractive(event.originalComponent, content))
            return;

        if (auto* peer = window.getPeer())
            macos::performWindowDrag(peer->getNativeHandle());
#else
        juce::ignoreUnused(event, window);
#endif
    }

    juce::Rectangle<int> Window::getDisplayArea() const
    {
        const auto userAreas = juce::Desktop::getInstance().getDisplays().getRectangleList(true);
        const auto windowCentre = getScreenBounds().getCentre();
        const auto distanceToWindow = [windowCentre](const auto& userArea) {
            return userArea.getConstrainedPoint(windowCentre).getDistanceSquaredFrom(windowCentre);
        };
        const auto display = std::min_element(std::begin(userAreas),
                                              std::end(userAreas),
                                              [&distanceToWindow](const auto& a, const auto& b) {
                                                  return distanceToWindow(a) < distanceToWindow(b);
                                              });

        return display != std::end(userAreas) ? *display : juce::Rectangle<int>{};
    }

    void Window::updateResizeLimits()
    {
        resizeLimitsArea = getDisplayArea();

        const auto area = resizeLimitsArea.toFloat();
        const auto unlimited = static_cast<float>(std::numeric_limits<juce::int16>::max());

        setResizeLimits(static_cast<int>(std::ceil(minWidth.getOr(1.0f).toPixels(area.getWidth()))),
                        static_cast<int>(std::ceil(minHeight.getOr(1.0f).toPixels(area.getHeight()))),
                        static_cast<int>(std::floor(maxWidth.getOr(unlimited).toPixels(area.getWidth()))),
                        static_cast<int>(std::floor(maxHeight.getOr(unlimited).toPixels(area.getHeight()))));
    }

    void Window::moved()
    {
        DocumentWindow::moved();

        if (getDisplayArea() != resizeLimitsArea)
            updateResizeLimits();
    }

    void Window::parentSizeChanged()
    {
        DocumentWindow::parentSizeChanged();
        updateResizeLimits();
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
                              windowTitleBarButtons,
                              integratedTitleBar);
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
        testIntegratedTitleBar();
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
        {
            juce::ValueTree tree{
                "Window",
                {
                    { "max-width", "50%" },
                    { "width", 100 },
                    { "height", 100 },
                },
            };
            jive::Interpreter interpreter;
            auto item = interpreter.interpret(tree);
            auto& window = *dynamic_cast<jive::GuiItemDecorator&>(*item)
                                .toType<jive::Window>();

            const auto userAreas = juce::Desktop::getInstance().getDisplays().getRectangleList(true);
            const auto windowCentre = window.getScreenBounds().getCentre();

            for (const auto& userArea : userAreas)
            {
                if (userArea.contains(windowCentre))
                    expectEquals(window.getConstrainer()->getMaximumWidth(), userArea.getWidth() / 2);
            }
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

    void testIntegratedTitleBar()
    {
        beginTest("integrated-title-bar");

    #if JUCE_MAC
        juce::ValueTree tree{
            "Window",
            {
                { "native", true },
                { "width", 200 },
                { "height", 150 },
            },
        };
        jive::Interpreter interpreter;
        auto item = interpreter.interpret(tree);
        auto& window = *dynamic_cast<jive::GuiItemDecorator&>(*item)
                            .toType<jive::Window>();
        expectEquals(window.getPeer()->getFrameSize().getTop(), 0);
        expectEquals(item->getComponent()->getWidth(), 200);
        expectEquals(item->getComponent()->getHeight(), 150);

        tree.setProperty("integrated-title-bar", false, nullptr);
        expectGreaterThan(window.getPeer()->getFrameSize().getTop(), 0);
        expectEquals(item->getComponent()->getHeight(), 150);

        tree.setProperty("integrated-title-bar", true, nullptr);
        expectEquals(window.getPeer()->getFrameSize().getTop(), 0);
        expectEquals(item->getComponent()->getHeight(), 150);
    #endif
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
