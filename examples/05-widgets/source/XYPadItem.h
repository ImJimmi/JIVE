#pragma once

#include "XYPad.h"

#include <jive_layouts/jive_layouts.h>

namespace jive_examples::widgets
{
    // Decorates each <XYPad> element, keeping its x-value and y-value
    // attributes and its XYPad in sync, and triggering on-change when it's
    // dragged, so C++ can treat it like any other element.
    class XYPadItem : public jive::GuiItemDecorator
    {
    public:
        explicit XYPadItem(std::unique_ptr<jive::GuiItem> itemToDecorate)
            : jive::GuiItemDecorator{ std::move(itemToDecorate) }
        {
            xValue.onValueChange = [this] {
                getPad().setValue({ xValue.get(), getPad().getValue().y });
            };
            yValue.onValueChange = [this] {
                getPad().setValue({ getPad().getValue().x, yValue.get() });
            };
            getPad().onValueChange = [this] {
                xValue = getPad().getValue().x;
                yValue = getPad().getValue().y;
                onChange.trigger();
            };

            getPad().setValue({ xValue.get(), yValue.get() }, juce::dontSendNotification);
        }

    private:
        [[nodiscard]] XYPad& getPad()
        {
            return dynamic_cast<XYPad&>(*getComponent());
        }

        jive::Property<float> xValue{ state, "x-value" };
        jive::Property<float> yValue{ state, "y-value" };
        jive::Event onChange{ state, "on-change" };
    };
} // namespace jive_examples::widgets
