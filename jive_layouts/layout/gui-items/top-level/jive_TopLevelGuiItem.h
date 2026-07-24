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
        }

        virtual void replaceDecoratedItem(std::unique_ptr<GuiItem>) = 0;
    };
} // namespace jive
