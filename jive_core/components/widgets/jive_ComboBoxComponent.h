#pragma once

#include <jive_core/graphics/jive_Icon.h>

#include <juce_gui_basics/juce_gui_basics.h>

namespace jive
{
    // A combo box that reports when it's styled with a different font, icon, or
    // border, any of which can change the size it needs to be.
    class ComboBoxComponent : public juce::ComboBox
    {
    public:
        // JUCE only creates a combo box's popup, and the items in it, while it's
        // open, and not as children of the combo box. These invisible stand-ins
        // take their place so they can be styled like any other component.
        class Popup : public juce::Component
        {
        public:
            Popup();
        };

        class Item : public juce::Component
        {
        public:
            explicit Item(int itemID);

            [[nodiscard]] int getItemID() const noexcept;

            // Highlighted items are styled as if they're being hovered.
            void setHighlighted(bool shouldBeHighlighted);

        private:
            const int itemID;
        };

        ComboBoxComponent();

        void paint(juce::Graphics& g) override;
        void lookAndFeelChanged() override;

        [[nodiscard]] Popup& getPopup() noexcept;
        [[nodiscard]] const Popup& getPopup() const noexcept;

        // Returns the stand-in for the item with the given ID, if it has one.
        [[nodiscard]] Item* findItem(int itemID) const;

        std::function<void()> onMetricsChange;

    private:
        void updateCachedStyles();

        Popup popup;
        std::optional<juce::Font> font;
        Icon icon;
        juce::BorderSize<float> borderWidth;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ComboBoxComponent)
    };
} // namespace jive
