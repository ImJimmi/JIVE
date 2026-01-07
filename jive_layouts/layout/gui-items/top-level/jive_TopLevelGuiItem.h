#pragma once

#include <jive_layouts/layout/gui-items/jive_GuiItemDecorator.h>

namespace jive
{
    class TopLevelGuiItem : public GuiItemDecorator
    {
    public:
        using GuiItemDecorator::GuiItemDecorator;

        virtual void replaceDecoratedItem(std::unique_ptr<GuiItem>) = 0;
    };
} // namespace jive
