#include "jive_GuiItemDecorator.h"

#include <jive_layouts/layout/gui-items/block/jive_BlockContainer.h>
#include <jive_layouts/layout/gui-items/block/jive_BlockItem.h>
#include <jive_layouts/layout/gui-items/content/jive_Image.h>
#include <jive_layouts/layout/gui-items/content/jive_Text.h>
#include <jive_layouts/layout/gui-items/flex/jive_FlexContainer.h>
#include <jive_layouts/layout/gui-items/flex/jive_FlexItem.h>
#include <jive_layouts/layout/gui-items/grid/jive_GridContainer.h>
#include <jive_layouts/layout/gui-items/grid/jive_GridItem.h>
#include <jive_layouts/layout/gui-items/jive_CommonGuiItem.h>
#include <jive_layouts/layout/gui-items/jive_ExternalSourceGuiItem.h>
#include <jive_layouts/layout/gui-items/widgets/jive_Button.h>
#include <jive_layouts/layout/gui-items/widgets/jive_ComboBox.h>
#include <jive_layouts/layout/gui-items/widgets/jive_Hyperlink.h>
#include <jive_layouts/layout/gui-items/widgets/jive_Knob.h>
#include <jive_layouts/layout/gui-items/widgets/jive_Label.h>
#include <jive_layouts/layout/gui-items/widgets/jive_ProgressBar.h>
#include <jive_layouts/layout/gui-items/widgets/jive_Slider.h>
#include <jive_layouts/layout/gui-items/widgets/jive_Spinner.h>
#include <jive_layouts/utilities/jive_Display.h>

namespace jive
{
    GuiItemDecorator::GuiItemDecorator(std::unique_ptr<GuiItem> itemToDecorate)
        : GuiItem{ *itemToDecorate }
        , item{ std::move(itemToDecorate) }
    {
        if (auto* decorator = dynamic_cast<GuiItemDecorator*>(item.get()))
            decorator->owner = this;
    }

    void GuiItemDecorator::insertChild(std::unique_ptr<GuiItem> child, int index)
    {
        item->insertChild(std::move(child), index);
        childrenChanged();
    }

    void GuiItemDecorator::setChildren(std::vector<std::unique_ptr<GuiItem>>&& newChildren)
    {
        item->setChildren(std::move(newChildren));
        childrenChanged();
    }

    void GuiItemDecorator::removeChild(GuiItem& child)
    {
        item->removeChild(child);
    }

    juce::Array<GuiItem*> GuiItemDecorator::getChildren()
    {
        if (item == nullptr)
            return {};

        return item->getChildren();
    }

    juce::Array<const GuiItem*> GuiItemDecorator::getChildren() const
    {
        return juce::Array<const GuiItem*>{ item->getChildren().getRawDataPointer(), item->getChildren().size() };
    }

    const GuiItem* GuiItemDecorator::getParent() const
    {
        return const_cast<GuiItemDecorator*>(this)->getParent();
    }

    GuiItem* GuiItemDecorator::getParent()
    {
        if (auto* parentItem = item->getParent())
        {
            if (auto* decoratedParent = dynamic_cast<GuiItemDecorator*>(parentItem))
                return &decoratedParent->getTopLevelDecorator();
        }

        return item->getParent();
    }

    bool GuiItemDecorator::isContainer() const
    {
        return item->isContainer();
    }

    bool GuiItemDecorator::isContent() const
    {
        return item->isContent();
    }

#if JIVE_IS_PLUGIN_PROJECT
    void GuiItemDecorator::attachToParameter(juce::RangedAudioParameter* parameter, juce::UndoManager* undoManager)
    {
        item->attachToParameter(parameter, undoManager);
    }
#endif

    GuiItemDecorator& GuiItemDecorator::getTopLevelDecorator()
    {
        if (owner != nullptr)
            return owner->getTopLevelDecorator();

        return *this;
    }

    const GuiItemDecorator& GuiItemDecorator::getTopLevelDecorator() const
    {
        if (owner != nullptr)
            return owner->getTopLevelDecorator();

        return *this;
    }

    std::unique_ptr<GuiItem> GuiItemDecorator::stripAllDecorators()
    {
        auto result = std::move(item);

        if (auto* decorator = dynamic_cast<GuiItemDecorator*>(result.get()))
            return decorator->stripAllDecorators();

        return result;
    }

    void GuiItemDecorator::layOutChildren()
    {
        item->layOutChildren();
    }

    [[nodiscard]] static std::unique_ptr<GuiItem> decorateWithDisplayBehaviour(std::unique_ptr<GuiItem> item)
    {
        Property<Display> display{ item->state, "display" };

        switch (display.getOr(Display::flex))
        {
        case Display::flex:
            return std::make_unique<FlexContainer>(std::move(item));
        case Display::grid:
            return std::make_unique<GridContainer>(std::move(item));
        case Display::block:
            return std::make_unique<BlockContainer>(std::move(item));
        }

        // Unhandled display type!
        jassertfalse;
        return nullptr;
    }

    [[nodiscard]] static std::unique_ptr<GuiItem> decorateWithHereditaryBehaviour(std::unique_ptr<GuiItem> item)
    {
        if (item->getParent() == nullptr)
            return item;

        Property<Display> display{ item->state.getParent(), "display" };

        switch (display.getOr(Display::flex))
        {
        case Display::flex:
            return std::make_unique<FlexItem>(std::move(item));
        case Display::grid:
            return std::make_unique<GridItem>(std::move(item));
        case Display::block:
            return std::make_unique<BlockItem>(std::move(item));
        }

        // Unhandled display type!
        jassertfalse;
        return nullptr;
    }

    [[nodiscard]] static std::unique_ptr<GuiItem> decorateWithWidgetBehaviour(std::unique_ptr<GuiItem> item)
    {
        const auto name = item->state.getType().toString();

        if (name == "Button" || name == "Checkbox")
            return std::make_unique<Button>(std::move(item));
        if (name == "ComboBox")
            return std::make_unique<ComboBox>(std::move(item));
        if (name == "Hyperlink")
            return std::make_unique<Hyperlink>(std::move(item));
        if (name == "Image" || name.compareIgnoreCase("svg") == 0)
            return std::make_unique<Image>(std::move(item));
        if (name == "Knob")
            return std::make_unique<Knob>(std::move(item));
        if (name == "Label")
            return std::make_unique<Label>(std::move(item));
        if (name == "ProgressBar")
            return std::make_unique<ProgressBar>(std::move(item));
        if (name == "Slider")
            return std::make_unique<Slider>(std::move(item));
        if (name == "Spinner")
            return std::make_unique<Spinner>(std::move(item));
        if (name == "Text")
            return std::make_unique<Text>(std::move(item));

        return item;
    }

    using DecoratorCreator = std::function<std::unique_ptr<GuiItemDecorator>(std::unique_ptr<GuiItem>)>;

    [[nodiscard]] static std::vector<const DecoratorCreator*> collectDecoratorCreators(const juce::Identifier& itemType,
                                                                                       const std::vector<std::pair<juce::Identifier, DecoratorCreator>>& decorators)
    {
        std::vector<const DecoratorCreator*> creators;

        for (const auto& decorator : decorators)
        {
            if (decorator.first == itemType)
                creators.push_back(&decorator.second);
        }

        return creators;
    }

    std::unique_ptr<GuiItem> decorate(std::unique_ptr<GuiItem> item,
                                      const std::vector<std::pair<juce::Identifier, DecoratorCreator>>& customDecorators)
    {
        item = std::make_unique<CommonGuiItem>(std::move(item));
        item = decorateWithHereditaryBehaviour(std::move(item));
        item = decorateWithWidgetBehaviour(std::move(item));

        if (!item->isContent())
            item = decorateWithDisplayBehaviour(std::move(item));

        for (const auto* decorateWithCustomDecorations : collectDecoratorCreators(item->state.getType(), customDecorators))
            item = (*decorateWithCustomDecorations)(std::move(item));

        return item;
    }
} // namespace jive
