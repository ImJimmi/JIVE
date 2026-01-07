#pragma once

#include <jive_layouts/layout/gui-items/jive_GuiItemDecorator.h>

namespace jive
{
    class ExternalSourceGuiItem : public GuiItemDecorator
    {
    public:
        explicit ExternalSourceGuiItem(std::unique_ptr<GuiItem> itemToDecorate)
            : GuiItemDecorator{ std::move(itemToDecorate) }
        {
        }

    private:
    };
} // namespace jive
