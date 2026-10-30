#include "jive_ComboBoxComponent.h"

#include <jive_style_sheets/geometry/jive_BorderWidth.h>
#include <jive_style_sheets/jive_style_sheets.h>

namespace jive
{
    ComboBoxComponent::Popup::Popup()
    {
        setAccessible(false);
        getProperties().set(ids::type(), "Popup");
    }

    ComboBoxComponent::Item::Item(int id)
        : itemID{ id }
    {
        setAccessible(false);
        getProperties().set(ids::type(), "Option");
    }

    int ComboBoxComponent::Item::getItemID() const noexcept
    {
        return itemID;
    }

    void ComboBoxComponent::Item::setHighlighted(bool shouldBeHighlighted)
    {
        getProperties().set(ids::highlighted(), shouldBeHighlighted);
    }

    ComboBoxComponent::ComboBoxComponent()
    {
        addChildComponent(popup);
    }

    void ComboBoxComponent::paint(juce::Graphics& g)
    {
        updateCachedStyles();
        juce::ComboBox::paint(g);
    }

    void ComboBoxComponent::lookAndFeelChanged()
    {
        juce::ComboBox::lookAndFeelChanged();
        updateCachedStyles();
    }

    ComboBoxComponent::Popup& ComboBoxComponent::getPopup() noexcept
    {
        return popup;
    }

    const ComboBoxComponent::Popup& ComboBoxComponent::getPopup() const noexcept
    {
        return popup;
    }

    ComboBoxComponent::Item* ComboBoxComponent::findItem(int itemID) const
    {
        for (auto* child : popup.getChildren())
        {
            if (auto* item = dynamic_cast<Item*>(child);
                item != nullptr && item->getItemID() == itemID)
            {
                return item;
            }
        }

        return nullptr;
    }

    void ComboBoxComponent::updateCachedStyles()
    {
        auto* laf = dynamic_cast<LookAndFeel*>(&getLookAndFeel());

        if (laf == nullptr)
            return;

        const auto styles = laf->findMostApplicableStyles(*this);
        const auto newFont = getFont(*this, styles);
        const auto newIcon = styles.icon.getValueOr({});
        const auto newBorderWidth = getBorderWidth(*this, styles);

        if (newFont == font && newIcon == icon && newBorderWidth == borderWidth)
            return;

        font = newFont;
        icon = newIcon;
        borderWidth = newBorderWidth;

        // The text needs repositioning as the space for it has changed.
        resized();

        if (onMetricsChange != nullptr)
            onMetricsChange();
    }
} // namespace jive
