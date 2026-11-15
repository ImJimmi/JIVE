#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace jive
{
    // A tooltip window styled per component. JUCE draws tooltips without
    // saying which component they belong to, so a hidden stand-in moves into
    // whichever component's tip is showing and is styled in its place,
    // matching selectors like "Button > Tooltip".
    class TooltipWindow : public juce::TooltipWindow
    {
    public:
        class Tooltip : public juce::Component
        {
        public:
            Tooltip();
        };

        explicit TooltipWindow(juce::Component* parent = nullptr, int millisecondsBeforeTipAppears = 700);
        ~TooltipWindow() override;

        juce::String getTipFor(juce::Component&) override;

        // Styles tips as belonging to the given component.
        void attachTo(juce::Component&);

        [[nodiscard]] const Tooltip& getStandIn() const noexcept;

    private:
        class Metrics : public juce::LookAndFeel_V4
        {
        public:
            explicit Metrics(TooltipWindow&);

            juce::Rectangle<int> getTooltipBounds(const juce::String&, juce::Point<int>, juce::Rectangle<int>) override;
            void drawTooltip(juce::Graphics&, const juce::String&, int, int) override;

        private:
            TooltipWindow& window;
        };

        Tooltip standIn;
        Metrics metrics{ *this };

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TooltipWindow)
    };
} // namespace jive
