#include <jive_layouts/jive_layouts.h>

namespace jive
{
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
        , remover{ std::make_unique<Remover>(*this) }
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

    const std::shared_ptr<const juce::Component> GuiItem::getComponent() const
    {
        return component;
    }

    const std::shared_ptr<juce::Component> GuiItem::getComponent()
    {
        return component;
    }

    void GuiItem::insertChild(std::unique_ptr<GuiItem> child, int index)
    {
        auto* newlyAddedChild = children.insert(index, std::move(child));
        component->addChildComponent(*newlyAddedChild->getComponent());
    }

    void GuiItem::removeChild(GuiItem& childToRemove)
    {
        children.removeObject(&childToRemove);
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

    GuiItem::Remover::Remover(GuiItem& guiItem)
        : item{ guiItem }
        , parent{ item.getParent() }
    {
        if (parent != nullptr)
            parent->state.addListener(this);
    }

    GuiItem::Remover::~Remover()
    {
        if (parent != nullptr)
            parent->state.removeListener(this);
    }

    void GuiItem::Remover::valueTreeChildRemoved(juce::ValueTree&,
                                                 juce::ValueTree& childWhichHasBeenRemoved,
                                                 int)
    {
        if (childWhichHasBeenRemoved == item.state && parent != nullptr)
            parent->removeChild(item);
    }
} // namespace jive

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
        expectEquals(item->state.getNumChildren(), 0);

        item->state.appendChild(juce::ValueTree{ "Component" }, nullptr);
        item->insertChild(std::make_unique<jive::GuiItem>(std::make_unique<juce::Component>(),
                                                          item->state.getChild(0),
                                                          item.get()),
                          -1);
        expectEquals(item->getChildren().size(), 1);
        expectEquals(item->getComponent()->getNumChildComponents(), item->getChildren().size());

        item->state.appendChild(juce::ValueTree{ "Component" }, nullptr);
        item->insertChild(std::make_unique<jive::GuiItem>(std::make_unique<juce::Component>(),
                                                          item->state.getChild(1),
                                                          item.get()),
                          -1);
        item->state.appendChild(juce::ValueTree{ "Component" }, nullptr);
        item->insertChild(std::make_unique<jive::GuiItem>(std::make_unique<juce::Component>(),
                                                          item->state.getChild(2),
                                                          item.get()),
                          -1);
        expectEquals(item->getChildren().size(), 3);
        expectEquals(item->getComponent()->getNumChildComponents(), item->getChildren().size());

        auto* child0 = item->getChildren()[0];
        expect(child0 != nullptr);
        item->removeChild(*child0);
        expectEquals(item->getChildren().size(), 2);
        expectEquals(item->getComponent()->getNumChildComponents(), item->getChildren().size());

        item->state.removeChild(0, nullptr);
        expectEquals(item->getChildren().size(), 2);
        expectEquals(item->getComponent()->getNumChildComponents(), item->getChildren().size());

        item->state.removeChild(0, nullptr);
        expectEquals(item->getChildren().size(), 1);
        expectEquals(item->getComponent()->getNumChildComponents(), item->getChildren().size());
    }
};

static GuiItemUnitTest guiItemUnitTest;
#endif
