#include <jive_layouts/jive_layouts.h>

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    GuiItem::GuiItem(std::shared_ptr<juce::Component> comp,
                     GuiItem* parentItem,
#if JIVE_GUI_ITEMS_HAVE_STYLE_SHEETS
                     StyleSheet::ReferenceCountedPointer sheet,
#endif
                     const juce::ValueTree& stateSource)
        : state{ stateSource }
        , component{ comp }
        , parent{ parentItem }
#if JIVE_GUI_ITEMS_HAVE_STYLE_SHEETS
        , styleSheet{ sheet }
#endif
    {
        jassert(component != nullptr);
    }

    GuiItem::GuiItem(std::unique_ptr<juce::Component> comp,
                     const juce::ValueTree& sourceState,
#if JIVE_GUI_ITEMS_HAVE_STYLE_SHEETS
                     StyleSheet::ReferenceCountedPointer sheet,
#endif
                     GuiItem* parentItem)
        : GuiItem
    {
        std::shared_ptr<juce::Component>{ std::move(comp) },
            parentItem,
#if JIVE_GUI_ITEMS_HAVE_STYLE_SHEETS
            std::move(sheet),
#endif
            sourceState,
    }
    {
    }

    GuiItem::GuiItem(const GuiItem& other)
        : GuiItem
    {
        other.component,
            other.parent,
#if JIVE_GUI_ITEMS_HAVE_STYLE_SHEETS
            nullptr,
#endif
            other.state,
    }
    {
    }

    //==================================================================================================================
    const std::shared_ptr<const juce::Component> GuiItem::getComponent() const
    {
        return component;
    }

    const std::shared_ptr<juce::Component> GuiItem::getComponent()
    {
        return component;
    }

    void GuiItem::addChild(std::unique_ptr<GuiItem> child)
    {
        auto* newlyAddedChild = children.add(std::move(child));
        component->addChildComponent(*newlyAddedChild->getComponent());
    }

    juce::Array<GuiItem*> GuiItem::getChildren()
    {
        return { children.getRawDataPointer(), children.size() };
    }

    juce::Array<const GuiItem*> GuiItem::getChildren() const
    {
        return { const_cast<GuiItem*>(this)->children.getRawDataPointer(), children.size() };
    }

    const GuiItem* GuiItem::getParent() const
    {
        return parent;
    }

    GuiItem* GuiItem::getParent()
    {
        return parent;
    }

    bool GuiItem::isTopLevel() const
    {
        return getParent() == nullptr;
    }

    bool GuiItem::isContainer() const
    {
        return true;
    }

    bool GuiItem::isContent() const
    {
        return false;
    }
} // namespace jive

//======================================================================================================================
#if JIVE_UNIT_TESTS
class GuiItemUnitTest : public juce::UnitTest
{
public:
    GuiItemUnitTest()
        : juce::UnitTest{ "jive::GuiItem", "jive" }
    {
    }

    void runTest() final
    {
        testChildren();
    }

private:
    void testChildren()
    {
        beginTest("children");

        jive::Interpreter interpreter;
        auto item = interpreter.interpret(juce::ValueTree{
            "Component",
            {
                { "width", 100 },
                { "height", 100 },
            },
        });
        expectEquals(item->getChildren().size(), 0);
        expectEquals(item->getComponent()->getNumChildComponents(), item->getChildren().size());

        item->addChild(std::make_unique<jive::GuiItem>(std::make_unique<juce::Component>(),
                                                       juce::ValueTree{ "Component" },
                                                       item.get()));
        expectEquals(item->getChildren().size(), 1);
        expectEquals(item->getComponent()->getNumChildComponents(), item->getChildren().size());

        item->addChild(std::make_unique<jive::GuiItem>(std::make_unique<juce::Component>(),
                                                       juce::ValueTree{ "Component" },
                                                       item.get()));
        item->addChild(std::make_unique<jive::GuiItem>(std::make_unique<juce::Component>(),
                                                       juce::ValueTree{ "Component" },
                                                       item.get()));
        expectEquals(item->getChildren().size(), 3);
        expectEquals(item->getComponent()->getNumChildComponents(), item->getChildren().size());
    }
};

static GuiItemUnitTest guiItemUnitTest;
#endif
