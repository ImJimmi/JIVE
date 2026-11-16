#include "jive_TooltipWindow.h"

#include <jive_style_sheets/geometry/jive_BorderWidth.h>
#include <jive_style_sheets/graphics/default-styles/jive_DefaultComponentStyles.h>
#include <jive_style_sheets/graphics/default-styles/jive_DefaultLabelStyles.h>
#include <jive_style_sheets/jive_style_sheets.h>

namespace jive
{
    static constexpr juce::Point<float> tooltipPadding{ 6.0f, 3.0f };

    TooltipWindow::Tooltip::Tooltip()
    {
        setAccessible(false);
        setInterceptsMouseClicks(false, false);
        getProperties().set(ids::type(), "Tooltip");
    }

    TooltipWindow::TooltipWindow(juce::Component* parent, int millisecondsBeforeTipAppears)
        : juce::TooltipWindow{ parent, millisecondsBeforeTipAppears }
    {
        setOpaque(false);
        setLookAndFeel(&metrics);
    }

    TooltipWindow::~TooltipWindow()
    {
        setLookAndFeel(nullptr);
    }

    juce::String TooltipWindow::getTipFor(juce::Component& component)
    {
        auto tip = juce::TooltipWindow::getTipFor(component);

        if (tip.isNotEmpty())
            attachTo(component);

        return tip;
    }

    void TooltipWindow::attachTo(juce::Component& component)
    {
        if (standIn.getParentComponent() != &component)
            component.addChildComponent(standIn);
    }

    const TooltipWindow::Tooltip& TooltipWindow::getStandIn() const noexcept
    {
        return standIn;
    }

    TooltipWindow::Metrics::Metrics(TooltipWindow& tooltipWindow)
        : window{ tooltipWindow }
    {
    }

    juce::Rectangle<int> TooltipWindow::Metrics::getTooltipBounds(const juce::String& text,
                                                                  juce::Point<int> position,
                                                                  juce::Rectangle<int> parentArea)
    {
        auto& tooltip = window.standIn;
        const auto* jiveLookAndFeel = dynamic_cast<const jive::LookAndFeel*>(&tooltip.getLookAndFeel());

        if (jiveLookAndFeel == nullptr)
            return tooltip.getLookAndFeel().getTooltipBounds(text, position, parentArea);

        const auto styles = jiveLookAndFeel->findTooltipStyles(tooltip);
        const auto font = getFont(tooltip, styles);
        const auto lines = juce::StringArray::fromLines(text);
        const auto textWidth = std::transform_reduce(std::begin(lines), std::end(lines), 0.0f, [](auto a, auto b) {
            return juce::jmax(a, b);
        },
                                                     [&font](const auto& line) {
                                                         return calculateStringWidth(line, font);
                                                     });
        const auto size = getBorderWidth(tooltip, styles)
                              .addedTo(juce::Rectangle{ textWidth, font.getHeight() * static_cast<float>(lines.size()) }
                                           .expanded(tooltipPadding.x, tooltipPadding.y))
                              .withZeroOrigin()
                              .getSmallestIntegerContainer()
                              .getBottomRight();

        tooltip.setSize(size.x, size.y);

        return juce::Rectangle{
            position.x > parentArea.getCentreX() ? position.x - (size.x + 12) : position.x + 24,
            position.y > parentArea.getCentreY() ? position.y - (size.y + 6) : position.y + 6,
            size.x,
            size.y,
        }
            .constrainedWithin(parentArea);
    }

    void TooltipWindow::Metrics::drawTooltip(juce::Graphics& g, const juce::String& text, int width, int height)
    {
        auto& tooltip = window.standIn;
        const auto* jiveLookAndFeel = dynamic_cast<const jive::LookAndFeel*>(&tooltip.getLookAndFeel());

        if (jiveLookAndFeel == nullptr)
            return tooltip.getLookAndFeel().drawTooltip(g, text, width, height);

        const auto styles = jiveLookAndFeel->findTooltipStyles(tooltip);
        tooltip.setSize(width, height);
        default_styles::paintComponent(g, tooltip, styles);
        default_styles::paintText(g,
                                  tooltip,
                                  tooltip.getLocalBounds().toFloat().reduced(tooltipPadding.x, tooltipPadding.y),
                                  styles,
                                  text,
                                  juce::Justification::centredLeft);
    }
} // namespace jive

#if JIVE_UNIT_TESTS
class TooltipWindowTest : public juce::UnitTest
{
public:
    TooltipWindowTest()
        : juce::UnitTest{ "jive::TooltipWindow", "jive" }
    {
    }

    void runTest() final
    {
        testStandIn();
        testSelectors();
        testThemes();
        testPainting();
    }

    void testStandIn()
    {
        beginTest("the stand-in moves into the component it's attached to");

        juce::Component parent;
        jive::LookAndFeel lookAndFeel{ parent };
        jive::TooltipWindow window{ &parent };
        juce::Component first;
        juce::Component second;
        parent.addAndMakeVisible(first);
        parent.addAndMakeVisible(second);

        expect(window.getStandIn().getParentComponent() == nullptr);

        window.attachTo(first);
        expect(window.getStandIn().getParentComponent() == &first);
        expect(!window.getStandIn().isVisible());

        window.attachTo(second);
        expect(window.getStandIn().getParentComponent() == &second);
    }

    void testSelectors()
    {
        beginTest("tooltips are styled by the component they belong to");

        juce::Component parent;
        jive::LookAndFeel lookAndFeel{ parent };
        lookAndFeel.addStyles("Tooltip", jive::Styles{}.withBackground(juce::Colours::red));
        lookAndFeel.addStyles("Button > Tooltip", jive::Styles{}.withBackground(juce::Colours::blue));
        lookAndFeel.addStyles("#special > Tooltip", jive::Styles{}.withBackground(juce::Colours::lime));

        jive::TooltipWindow window{ &parent };
        juce::TextButton button;
        button.getProperties().set("jive::type", "Button");
        juce::TextButton special;
        special.setComponentID("special");
        special.getProperties().set("jive::type", "Button");
        juce::Label label;

        for (auto* component : std::initializer_list<juce::Component*>{ &button, &special, &label })
            parent.addAndMakeVisible(*component);

        const auto backgroundFor = [&](juce::Component& component) {
            window.attachTo(component);
            return lookAndFeel.findTooltipStyles(window.getStandIn()).background.get();
        };

        expect(backgroundFor(label) == juce::Colours::red);
        expect(backgroundFor(button) == juce::Colours::blue);
        expect(backgroundFor(special) == juce::Colours::lime);
    }

    void testThemes()
    {
        beginTest("every theme styles tooltips");

        for (const auto theme : { jive::Theme::juce,
                                  jive::Theme::steel,
                                  jive::Theme::material,
                                  jive::Theme::synthwave,
                                  jive::Theme::highContrast })
        {
            juce::Component parent;
            jive::LookAndFeel lookAndFeel{ parent };
            lookAndFeel.applyTheme(theme);
            jive::TooltipWindow window{ &parent };
            juce::TextButton button;
            parent.addAndMakeVisible(button);
            window.attachTo(button);

            const auto styles = lookAndFeel.findTooltipStyles(window.getStandIn());
            expect(styles.background.hasValue());
            expect(styles.foreground.hasValue());
        }
    }

    void testPainting()
    {
        beginTest("tooltips are sized and painted from their styles");

        juce::Component parent;
        parent.setSize(400, 400);
        jive::LookAndFeel lookAndFeel{ parent };
        lookAndFeel.applyTheme(jive::Theme::juce);
        lookAndFeel.addStyles("#owner Tooltip",
                              jive::Styles{}
                                  .withBackground(juce::Colours::lime)
                                  .withBorderWidth(juce::BorderSize{ 10.0f })
                                  .withBorderFill(juce::Colours::red));

        jive::TooltipWindow window{ &parent };
        juce::Component owner;
        owner.setComponentID("owner");
        parent.addAndMakeVisible(owner);
        window.attachTo(owner);

        const auto bounds = window.getLookAndFeel().getTooltipBounds("Tip", { 10, 10 }, parent.getLocalBounds());
        expectGreaterThan(bounds.getWidth(), 20 + 12);
        expectGreaterThan(bounds.getHeight(), 20 + 6);

        const auto font = jive::getFont(window.getStandIn(), lookAndFeel.findTooltipStyles(window.getStandIn()));
        expectGreaterOrEqual(static_cast<float>(bounds.getWidth()), jive::calculateStringWidth("Tip", font) + 20.0f + 12.0f);
        expectGreaterOrEqual(static_cast<float>(bounds.getHeight()), font.getHeight() + 20.0f + 6.0f);

        window.setBounds(bounds);
        const auto snapshot = window.createComponentSnapshot(window.getLocalBounds());
        expect(snapshot.getPixelAt(2, bounds.getHeight() / 2) == juce::Colours::red);
        expect(snapshot.getPixelAt(12, 12) == juce::Colours::lime);
    }
};

static TooltipWindowTest tooltipWindowTest;
#endif
