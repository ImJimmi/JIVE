#include "jive_GuiItem.h"

#include "jive_CommonGuiItem.h"
#include "jive_GuiItemDecorator.h"

namespace jive
{
    class PassiveView : public View
    {
    public:
        explicit PassiveView(const juce::ValueTree& sourceState)
            : view{ sourceState }
        {
        }

    protected:
        [[nodiscard]] juce::ValueTree initialise() final
        {
            return view;
        }

    private:
        juce::ValueTree view;
    };

    View::ReferenceCountedPointer GuiItem::getOrCreateView(juce::ValueTree state)
    {
        View::ReferenceCountedPointer view;

        if (state.hasProperty("view-object"))
        {
            view = dynamic_cast<View*>(state["view-object"].getObject());
        }
        else if (state.hasProperty("make-view"))
        {
            const auto buildView = state["make-view"].getNativeFunction();
            const auto viewObjectProperty = buildView({ juce::var{}, nullptr, 0 });

            if (auto* wrappedState = dynamic_cast<ReferenceCountedValueTreeWrapper*>(viewObjectProperty.getObject()))
            {
                if (auto* viewObject = dynamic_cast<View*>(wrappedState->state["view-object"].getObject()))
                    view = viewObject;
                else
                    jassertfalse;
            }
            else
            {
                jassertfalse;
            }
        }
        else
        {
            view = new PassiveView{ state };
        }

        state.removeProperty("view-object", nullptr);
        view->state = state;

        return view;
    }

    GuiItem::GuiItem(std::shared_ptr<juce::Component> comp,
                     GuiItem* parentItem,
#if JIVE_GUI_ITEMS_HAVE_STYLE_SHEETS
                     StyleSheet::ReferenceCountedPointer sheet,
#endif
                     View::ReferenceCountedPointer sourceView)
        : state{ sourceView->getState() }
#if JIVE_GUI_ITEMS_HAVE_STYLE_SHEETS
        , styleSheet{ sheet }
#endif
        , component{ comp }
        , parent{ parentItem }
        , remover{ std::make_unique<Remover>(*this) }
        , view{ sourceView }
    {
        jassert(component != nullptr);
    }

    GuiItem::GuiItem(std::unique_ptr<juce::Component> comp,
                     const juce::ValueTree& sourceState,
#if JIVE_GUI_ITEMS_HAVE_STYLE_SHEETS
                     StyleSheet::ReferenceCountedPointer sheet,
#endif
                     GuiItem* parentItem)
        : GuiItem{
            std::shared_ptr<juce::Component>{ std::move(comp) },
            parentItem,
#if JIVE_GUI_ITEMS_HAVE_STYLE_SHEETS
            std::move(sheet),
#endif
            getOrCreateView(sourceState),
        }
    {
    }

    GuiItem::GuiItem(std::unique_ptr<juce::Component> comp,
                     View::ReferenceCountedPointer sourceView,
#if JIVE_GUI_ITEMS_HAVE_STYLE_SHEETS
                     StyleSheet::ReferenceCountedPointer sheet,
#endif
                     GuiItem* parentItem)
        : GuiItem{
            std::shared_ptr<juce::Component>{ std::move(comp) },
            parentItem,
#if JIVE_GUI_ITEMS_HAVE_STYLE_SHEETS
            std::move(sheet),
#endif
            sourceView,
        }
    {
    }

    GuiItem::GuiItem(const GuiItem& other)
        : GuiItem{
            other.component,
            other.parent,
#if JIVE_GUI_ITEMS_HAVE_STYLE_SHEETS
            nullptr,
#endif
            other.view,
        }
    {
    }

    GuiItem::~GuiItem()
    {
        masterReference.clear();
    }

    const std::shared_ptr<const juce::Component> GuiItem::getComponent() const
    {
        return component;
    }

    std::shared_ptr<juce::Component> GuiItem::getComponent()
    {
        return component;
    }

    const View::ReferenceCountedPointer GuiItem::getView() const
    {
        return view;
    }

    View::ReferenceCountedPointer GuiItem::getView()
    {
        return view;
    }

    void GuiItem::insertChild(std::unique_ptr<GuiItem> child, int index)
    {
        insertChild(std::move(child), index, true);
    }

    void GuiItem::insertChild(std::unique_ptr<GuiItem> child, int index, bool invokeCallback)
    {
        if (child == nullptr)
        {
            // Trying to add a nonexistant child!
            jassertfalse;
            return;
        }

        auto* newlyAddedChild = children.insert(index, std::move(child));
        component->addChildComponent(*newlyAddedChild->getComponent());

        if (invokeCallback)
            childrenChanged();
    }

    void GuiItem::setChildren(std::vector<std::unique_ptr<GuiItem>>&& newChildren)
    {
        children.clearQuick(true);

        for (auto& child : newChildren)
            insertChild(std::move(child), children.size(), false);

        childrenChanged();
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

    void GuiItem::callLayoutChildrenWithRecursionLock()
    {
        if (isLayingOutChildren() || std::size(getChildren()) == 0)
            return;

        const BoxModel::ScopedCallbackLock boxModelLock{ boxModel(*this) };
        const juce::ScopedValueSetter svs{ layoutRecursionLock, true };
        layOutChildren();
    }

    bool GuiItem::isLayingOutChildren() const
    {
        return layoutRecursionLock;
    }

#if JIVE_IS_PLUGIN_PROJECT
    void GuiItem::attachToParameter(juce::RangedAudioParameter*, juce::UndoManager*)
    {
        // It doesn't make sense to call this on an item that isn't a widget!
        // Did you mean to use jive::findItemWithID() to attach to a specific item?
        jassertfalse;
    }
#endif

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

    BoxModel& boxModel(GuiItem& item)
    {
        // This is a convenience function that only works if the given GUI item
        // is decorated as a CommonGuiItem!
        auto* decorated = dynamic_cast<jive::GuiItemDecorator*>(&item);
        jassert(decorated != nullptr);
        auto* common = decorated->getTopLevelDecorator().toType<jive::CommonGuiItem>();
        jassert(common != nullptr);

        return common->boxModel;
    }

    const BoxModel& boxModel(const GuiItem& item)
    {
        return boxModel(*const_cast<GuiItem*>(&item));
    }

    GuiItem* findItemWithID(GuiItem& root, const juce::Identifier& id)
    {
        if (root.state["id"].toString() == id.toString())
            return &root;

        for (auto* child : root.getChildren())
        {
            if (auto* item = findItemWithID(*child, id))
                return item;
        }

        return nullptr;
    }
} // namespace jive

#if JIVE_UNIT_TESTS
    #include <jive_layouts/layout/jive_Interpreter.h>

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
        testAddingMultipleChildrenAtOnce();
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

    void testAddingMultipleChildrenAtOnce()
    {
        beginTest("Adding multiple children at once");

        jive::GuiItem item{
            std::make_unique<juce::Component>(),
            juce::ValueTree{ "Component" },
        };
        expectEquals(item.getChildren().size(), 0);

        item.setChildren({});
        expectEquals(item.getChildren().size(), 0);

        {
            std::vector<std::unique_ptr<jive::GuiItem>> children;
            children.push_back(std::make_unique<jive::GuiItem>(std::make_unique<juce::Component>(),
                                                               juce::ValueTree{ "Component" }));
            children.push_back(std::make_unique<jive::GuiItem>(std::make_unique<juce::Component>(),
                                                               juce::ValueTree{ "Component" }));

            item.setChildren(std::move(children));
            expectEquals(item.getChildren().size(), 2);
        }

        {
            std::vector<std::unique_ptr<jive::GuiItem>> children;

            for (auto i = 0; i < 1000; i++)
                children.push_back(std::make_unique<jive::GuiItem>(std::make_unique<juce::Component>(),
                                                                   juce::ValueTree{ "Component" }));

            item.setChildren(std::move(children));
            expectEquals(item.getChildren().size(), 1000);
        }
    }
};

struct BoxModelFreeFunctionTest : juce::UnitTest
{
    BoxModelFreeFunctionTest()
        : juce::UnitTest{ "jive::boxModel()", "jive" }
    {
    }

    void runTest() final
    {
        beginTest("Common GUI item");

        const jive::CommonGuiItem item{
            std::make_unique<jive::GuiItem>(std::make_unique<juce::Component>(),
                                            juce::ValueTree{ "Component" }),
        };
        const auto& box = jive::boxModel(item);
        expect(&box == &item.boxModel);
    }
};

static GuiItemUnitTest guiItemUnitTest;
#endif
