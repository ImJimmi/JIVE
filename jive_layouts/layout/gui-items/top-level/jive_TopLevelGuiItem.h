#pragma once

#include <jive_layouts/layout/gui-items/jive_GuiItemDecorator.h>

namespace jive
{
    class TopLevelGuiItem : public GuiItemDecorator
    {
    public:
        explicit TopLevelGuiItem(std::unique_ptr<GuiItem> itemToDecorate)
            : GuiItemDecorator{ std::move(itemToDecorate) }
        {
            getComponent()->setInterceptsMouseClicks(true, true);
            resetDefaultPadding();
        }

        virtual void replaceDecoratedItem(std::unique_ptr<GuiItem>) = 0;

    protected:
        static constexpr auto defaultPadding = 10.0f;

        void resetDefaultPadding(float top = defaultPadding)
        {
            hasDefaultPadding = !state.hasProperty(ids::padding());
            applyDefaultPadding(top);
        }

        void applyDefaultPadding(float top)
        {
            if (!hasDefaultPadding)
                return;

            const juce::BorderSize<float> padding{ top, defaultPadding, defaultPadding, defaultPadding };
            state.setProperty(ids::padding(), juce::VariantConverter<juce::BorderSize<float>>::toVar(padding), nullptr);
        }

    private:
        bool hasDefaultPadding{ false };
    };
} // namespace jive
