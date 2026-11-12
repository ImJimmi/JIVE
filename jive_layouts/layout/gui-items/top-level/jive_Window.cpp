#include "jive_Window.h"

#include <jive_layouts/layout/gui-items/jive_CommonGuiItem.h>

#if JUCE_MAC
    #include "jive_Window_mac.h"
#elif JUCE_WINDOWS
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #include <windows.h>
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
            applyDefaultPadding(getDefaultTopPadding());
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
            applyDefaultPadding(getDefaultTopPadding());
            updateIntegratedTitleBar();
        };
        setTitleBarHeight(juce::roundToInt(windowTitleBarHeight.getOr(26)));

        integratedTitleBar.onValueChange = [this]() {
            applyDefaultPadding(getDefaultTopPadding());
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

        applyDefaultPadding(getDefaultTopPadding());

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
        resetDefaultPadding(getDefaultTopPadding());
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
        updateBackdrop();
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

    float Window::getDefaultTopPadding() const
    {
#if JUCE_MAC
        if (isUsingNativeTitleBar() && integratedTitleBar.getOr(true))
            return windowTitleBarHeight.getOr(defaultMacTitleBarHeight);
#endif

        return defaultPadding;
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

        const auto colour = [this]() -> std::optional<juce::Colour> {
            const auto styles = jiveLookAndFeel.findMostApplicableStyles(*getComponent());

            if (const auto titleBar = styles.find<Fill>("title-bar"))
                return toColour(titleBar);

            // By default, the title bar matches the window's background. A
            // backdrop is left to DWM so the material extends into the title bar.
            if (const auto background = styles.find<Fill>("background"))
            {
                if (background->getBackdrop().has_value())
                    return std::nullopt;

                return toColour(background);
            }

            return getBackgroundColour();
        }();

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

#if JUCE_WINDOWS
    namespace windows
    {
        static constexpr auto backdropProperty = L"jive.backdrop";
        static constexpr auto originalProcProperty = L"jive.backdrop.wndproc";

        static void fillRedirectionSurfaceBlack(HWND hwnd)
        {
            RECT clientArea{};
            GetClientRect(hwnd, &clientArea);

            if (auto* const dc = GetDC(hwnd))
            {
                FillRect(dc, &clientArea, static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH)));
                ReleaseDC(hwnd, dc);
            }
        }

        // DWM only draws the backdrop where the window's GDI redirection
        // surface is black; JUCE's Direct2D content is composited on top of
        // it. JUCE validates paint regions without BeginPaint, so the surface
        // is never erased and stays white. This fills it with black whenever
        // the window paints.
        static LRESULT CALLBACK backdropWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
        {
            const auto originalProc = reinterpret_cast<WNDPROC>(GetPropW(hwnd, originalProcProperty));

            if (GetPropW(hwnd, backdropProperty) != nullptr)
            {
                if (message == WM_ERASEBKGND)
                    return 1;

                if (message == WM_PAINT)
                    fillRedirectionSurfaceBlack(hwnd);
            }

            if (message == WM_NCDESTROY)
            {
                SetWindowLongPtrW(hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(originalProc));
                RemovePropW(hwnd, backdropProperty);
                RemovePropW(hwnd, originalProcProperty);
            }

            return CallWindowProcW(originalProc, hwnd, message, wParam, lParam);
        }

        static void setBlackRedirectionSurface(HWND hwnd, bool shouldBeBlack)
        {
            if (GetPropW(hwnd, originalProcProperty) == nullptr)
            {
                const auto originalProc = GetWindowLongPtrW(hwnd, GWLP_WNDPROC);
                SetPropW(hwnd, originalProcProperty, reinterpret_cast<HANDLE>(originalProc));
                SetWindowLongPtrW(hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&backdropWindowProc));
            }

            if (shouldBeBlack)
            {
                SetPropW(hwnd, backdropProperty, reinterpret_cast<HANDLE>(1));
                fillRedirectionSurfaceBlack(hwnd);
            }
            else
            {
                RemovePropW(hwnd, backdropProperty);
            }

            InvalidateRect(hwnd, nullptr, FALSE);
        }

        static void setBackdrop(void* hwnd,
                                std::optional<Backdrop> backdrop,
                                bool darkMode,
                                bool hasNativeShadow)
        {
            struct Margins
            {
                int left, right, top, bottom;
            };

            using SetWindowAttribute = long(__stdcall*)(void*, unsigned long, const void*, unsigned long);
            using ExtendFrameIntoClientArea = long(__stdcall*)(void*, const Margins*);
            static juce::DynamicLibrary dwmapi{ "dwmapi.dll" };
            static const auto setWindowAttribute = reinterpret_cast<SetWindowAttribute>(dwmapi.getFunction("DwmSetWindowAttribute"));
            static const auto extendFrame = reinterpret_cast<ExtendFrameIntoClientArea>(dwmapi.getFunction("DwmExtendFrameIntoClientArea"));

            if (setWindowAttribute == nullptr || extendFrame == nullptr)
                return;

            static constexpr unsigned long immersiveDarkModeAttribute = 20;
            static constexpr unsigned long systemBackdropTypeAttribute = 38;
            static constexpr int backdropNone = 1;
            static constexpr int backdropMica = 2;
            static constexpr int backdropAcrylic = 3;

            const int type = !backdrop.has_value()
                               ? backdropNone
                               : (*backdrop == Backdrop::glass ? backdropAcrylic : backdropMica);

            // The material's light/dark variant follows the window's dark-mode flag.
            const int useDarkMode = darkMode ? 1 : 0;
            setWindowAttribute(hwnd, immersiveDarkModeAttribute, &useDarkMode, sizeof(useDarkMode));

            // DWM only draws the material behind the client area if the frame
            // is extended over it. JUCE's Direct2D renderer presents through a
            // premultiplied-alpha composition swap chain, so transparent pixels
            // let the material show through. Without a backdrop, restore the
            // margins JUCE itself uses.
            const auto margins = backdrop.has_value()
                                   ? Margins{ -1, -1, -1, -1 }
                                   : (hasNativeShadow ? Margins{ 1, 1, 0, 0 } : Margins{ 0, 0, 0, 0 });
            extendFrame(hwnd, &margins);

            setWindowAttribute(hwnd, systemBackdropTypeAttribute, &type, sizeof(type));
            setBlackRedirectionSurface(static_cast<HWND>(hwnd), backdrop.has_value());
        }
    } // namespace windows
#endif

    void Window::updateBackdrop()
    {
#if JUCE_MAC || JUCE_WINDOWS
        auto* const peer = getPeer();

        if (peer == nullptr)
            return;

        const auto backdrop = jiveLookAndFeel.findMostApplicableStyles(*getComponent())
                                  .find<Fill>("background")
                                  .value_or(Fill{})
                                  .getBackdrop();

        if (backdrop == appliedBackdrop && peer->getNativeHandle() == backdropHandle)
            return;

        appliedBackdrop = backdrop;
        backdropHandle = peer->getNativeHandle();

        // AppKit throws if the view hierarchy or the window's opacity changes
        // mid-paint, so the backdrop is applied after this paint. Windows does
        // the same so that changing the background doesn't repaint mid-paint.
        juce::MessageManager::callAsync([safeThis = juce::Component::SafePointer<Window>{ this }, backdrop] {
            if (safeThis != nullptr)
                safeThis->applyBackdrop(backdrop);
        });
#endif
    }

    void Window::applyBackdrop([[maybe_unused]] std::optional<Backdrop> backdrop)
    {
#if JUCE_MAC
        if (getPeer() == nullptr)
            return;

        if (backdrop.has_value())
            setBackgroundColour(juce::Colours::transparentBlack);
        else
            removeColour(backgroundColourId);

        setOpaque(getBackgroundColour().isOpaque());

        if (auto* const peer = getPeer())
            macos::setBackdrop(peer->getNativeHandle(), backdrop);
#elif JUCE_WINDOWS
        auto* const peer = getPeer();

        if (peer == nullptr)
            return;

        // The window stays opaque as far as JUCE is concerned: a non-opaque
        // window becomes a layered window, which DWM can't draw a backdrop
        // behind (and isn't allowed with a native title bar). The transparent
        // background is still presented with alpha by the Direct2D renderer.
        // setBackgroundColour() would make the window non-opaque, so the
        // colour is set directly.
        if (backdrop.has_value())
            setColour(backgroundColourId, juce::Colours::transparentBlack);
        else
            removeColour(backgroundColourId);

        setOpaque(true);
        const auto themeBackground = getLookAndFeel().findColour(backgroundColourId);
        windows::setBackdrop(peer->getNativeHandle(),
                             backdrop,
                             themeBackground.getPerceivedBrightness() < 0.5f,
                             !isUsingNativeTitleBar() && isDropShadowEnabled());
        repaint();
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
        testDefaultPadding();
        testBackdrop();
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

    void testDefaultPadding()
    {
        const auto getPadding = [](juce::ValueTree tree) {
            jive::Interpreter interpreter;
            auto item = interpreter.interpret(tree);
            return jive::boxModel(*item).getPadding();
        };

        beginTest("windows have a default padding");
        expectEquals(getPadding(juce::ValueTree{
                         "Window",
                         {
                             { "native", false },
                             { "width", 200 },
                             { "height", 150 },
                         },
                     }),
                     juce::BorderSize<float>{ 10.0f });

        beginTest("a window's padding overrides the default");
        expectEquals(getPadding(juce::ValueTree{
                         "Window",
                         {
                             { "padding", "4 12" },
                             { "width", 200 },
                             { "height", 150 },
                         },
                     }),
                     juce::BorderSize<float>{ 4.0f, 12.0f, 4.0f, 12.0f });

    #if JUCE_MAC
        beginTest("a window's default padding clears an integrated title bar");
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
        expectEquals(jive::boxModel(*item).getPadding(),
                     juce::BorderSize<float>{ 28.0f, 10.0f, 10.0f, 10.0f });

        tree.setProperty("title-bar-height", 40, nullptr);
        expectEquals(jive::boxModel(*item).getPadding(),
                     juce::BorderSize<float>{ 40.0f, 10.0f, 10.0f, 10.0f });

        tree.setProperty("integrated-title-bar", false, nullptr);
        expectEquals(jive::boxModel(*item).getPadding(),
                     juce::BorderSize<float>{ 10.0f });
    #endif
    }

    void testBackdrop()
    {
        beginTest("a backdrop is applied after painting, since AppKit throws if the window changes mid-paint");

    #if JUCE_MAC
        juce::ValueTree tree{
            "Window",
            {
                { "native", true },
                { "width", 200 },
                { "height", 150 },
                { "style", new jive::Object{ { "background", "glass" } } },
            },
        };
        jive::Interpreter interpreter;
        auto item = interpreter.interpret(tree);
        auto& window = *dynamic_cast<jive::GuiItemDecorator&>(*item)
                            .toType<jive::Window>();
        const auto* const peer = window.getPeer();
        juce::ignoreUnused(window.createComponentSnapshot(window.getLocalBounds()));
        expect(window.isOpaque());
        expect(window.getPeer() == peer);
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
