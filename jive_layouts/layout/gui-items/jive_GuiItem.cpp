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

        if (state.hasProperty(ids::viewObject()))
        {
            view = dynamic_cast<View*>(state[ids::viewObject()].getObject());
        }
        else if (state.hasProperty(ids::makeView()))
        {
            const auto buildView = state[ids::makeView()].getNativeFunction();
            const auto viewObjectProperty = buildView({ juce::var{}, nullptr, 0 });

            if (auto* wrappedState = dynamic_cast<ReferenceCountedValueTreeWrapper*>(viewObjectProperty.getObject()))
            {
                if (auto* viewObject = dynamic_cast<View*>(wrappedState->state[ids::viewObject()].getObject()))
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

        state.removeProperty(ids::viewObject(), nullptr);
        view->state = state;

        return view;
    }

    GuiItem::GuiItem(std::shared_ptr<juce::Component> comp,
                     GuiItem* parentItem,
                     View::ReferenceCountedPointer sourceView)
        : state{ sourceView->getState() }
        , component{ comp }
        , parent{ parentItem }
        , view{ sourceView }
    {
        jassert(component != nullptr);
    }

    GuiItem::GuiItem(std::unique_ptr<juce::Component> comp,
                     const juce::ValueTree& sourceState,
                     GuiItem* parentItem)
        : GuiItem{
            std::shared_ptr<juce::Component>{ std::move(comp) },
            parentItem,
            getOrCreateView(sourceState),
        }
    {
    }

    GuiItem::GuiItem(std::shared_ptr<juce::Component> comp,
                     const juce::ValueTree& sourceState,
                     GuiItem* parentItem)
        : GuiItem{
            comp,
            parentItem,
            getOrCreateView(sourceState),
        }
    {
    }

    GuiItem::GuiItem(std::unique_ptr<juce::Component> comp,
                     View::ReferenceCountedPointer sourceView,
                     GuiItem* parentItem)
        : GuiItem{
            std::shared_ptr<juce::Component>{ std::move(comp) },
            parentItem,
            sourceView,
        }
    {
    }

    GuiItem::GuiItem(const GuiItem& other)
        : GuiItem{
            other.component,
            other.parent,
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

    const juce::LocalisedStrings* GuiItem::findLocalisedStrings() const
    {
        for (const auto* ancestor = this; ancestor != nullptr; ancestor = ancestor->getParent())
        {
            if (const auto* strings = ancestor->getLocalisedStrings())
                return strings;
        }

        return nullptr;
    }

    juce::String GuiItem::translate(const juce::String& text) const
    {
        if (const auto* strings = findLocalisedStrings())
            return strings->translate(text);

        return juce::translate(text);
    }

    const juce::LocalisedStrings* GuiItem::getLocalisedStrings() const
    {
        return localisedStrings.has_value() ? &*localisedStrings : nullptr;
    }

    // Children are notified first so that a nested Text has its new text by the
    // time its parent appends it. Children with their own strings are skipped
    // because they don't translate with this item's.
    void GuiItem::notifyLocalisedStringsChanged()
    {
        for (auto* child : getChildren())
        {
            if (child->getLocalisedStrings() == nullptr)
                child->notifyLocalisedStringsChanged();
        }

        localisedStringsChanged();
    }

    bool GuiItem::isContainer() const
    {
        return true;
    }

    bool GuiItem::isContent() const
    {
        return false;
    }

    bool GuiItem::stretchesByDefault() const
    {
        return true;
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
        if (root.state[ids::id()].toString() == id.toString())
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
    #include <jive_layouts/layout/interpreter/jive_Interpreter.h>

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

        item->insertChild(std::make_unique<jive::GuiItem>(std::make_unique<juce::Component>(),
                                                          juce::ValueTree{ "Component" },
                                                          item.get()),
                          -1);
        expectEquals(item->getChildren().size(), 1);
        expectEquals(item->getComponent()->getNumChildComponents(), item->getChildren().size());

        item->insertChild(std::make_unique<jive::GuiItem>(std::make_unique<juce::Component>(),
                                                          juce::ValueTree{ "Component" },
                                                          item.get()),
                          -1);
        item->insertChild(std::make_unique<jive::GuiItem>(std::make_unique<juce::Component>(),
                                                          juce::ValueTree{ "Component" },
                                                          item.get()),
                          -1);
        expectEquals(item->getChildren().size(), 3);
        expectEquals(item->getComponent()->getNumChildComponents(), item->getChildren().size());

        auto* child0 = item->getChildren()[0];
        expect(child0 != nullptr);
        item->removeChild(*child0);
        expectEquals(item->getChildren().size(), 2);
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
