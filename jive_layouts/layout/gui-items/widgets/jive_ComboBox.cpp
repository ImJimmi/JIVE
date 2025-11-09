#include "jive_ComboBox.h"

namespace jive
{
    ComboBox::Option::Option(juce::ValueTree sourceTree, int itemIndex, juce::ComboBox& box)
        : tree{ sourceTree }
        , comboBox{ box }
        , index{ itemIndex }
        , id{ index + 1 }
        , text{ tree, "text" }
        , enabled{ tree, "enabled" }
        , selected{ tree, "selected" }
    {
        comboBox.addItem(text, id);

        text.onValueChange = [this]() {
            comboBox.changeItemText(id, text);
        };

        selected.onValueChange = [this]() {
            if (selected)
            {
                comboBox.setSelectedItemIndex(index);
                wasSelected = true;
            }
            else if (wasSelected)
            {
                comboBox.setSelectedId(0);
                wasSelected = false;
            }
        };

        if (selected)
        {
            comboBox.setSelectedItemIndex(index);
            wasSelected = true;
        }

        enabled.onValueChange = [this]() {
            comboBox.setItemEnabled(id, enabled);
        };
        comboBox.setItemEnabled(id, enabled);
    }

    void ComboBox::Option::setSelected(bool shouldBeSelected)
    {
        selected = shouldBeSelected;
    }

    ComboBox::Header::Header(juce::ValueTree sourceTree, ComboBox& box)
        : comboBox{ box }
        , text{ sourceTree, "text" }
    {
        box.getComboBox().addSectionHeading(text);

        text.onValueChange = [this]() {
            comboBox.updateItems();
        };
    }

    ComboBox::ComboBox(std::unique_ptr<GuiItem> itemToDecorate)
        : GuiItemDecorator(std::move(itemToDecorate))
        , editable{ state, "editable" }
        , tooltip{ state, "tooltip" }
        , selected{ state, "selected" }
        , width{ state, "width" }
        , height{ state, "height" }
        , focusable{ state, "focusable" }
        , onChange{ state, "on-change" }
    {
        const BoxModel::ScopedCallbackLock boxModelLock{ boxModel(*this) };

        if (!focusable.exists())
            focusable = true;

        editable.onValueChange = [this]() {
            getComboBox().setEditableText(editable);
        };
        getComboBox().setEditableText(editable);

        tooltip.onValueChange = [this]() {
            getComboBox().setTooltip(tooltip);
        };
        getComboBox().setTooltip(tooltip);

        selected.onValueChange = [this]() {
            getComboBox().setSelectedItemIndex(selected);

            auto currentlySelectedOption = state.getChildWithProperty("selected", true);

            if (currentlySelectedOption.isValid())
                currentlySelectedOption.setProperty("selected", false, nullptr);

            if (selected < options.size())
            {
                if (auto* selectedOption = options[selected])
                    selectedOption->setSelected(true);
            }
        };

        updateItems();
        getComboBox().setSelectedItemIndex(selected);
        getComboBox().addListener(this);

        state.addListener(this);

        if (width.isAuto())
            width = "50px";
        if (height.isAuto())
            height = "20px";
    }

    bool ComboBox::isContainer() const
    {
        return false;
    }

#if JIVE_IS_PLUGIN_PROJECT
    void ComboBox::attachToParameter(juce::RangedAudioParameter* parameter, juce::UndoManager* undoManager)
    {
        if (parameter != nullptr)
            parameterAttachment = std::make_unique<juce::ComboBoxParameterAttachment>(*parameter, getComboBox(), undoManager);
        else
            parameterAttachment = nullptr;
    }
#endif

    juce::ComboBox& ComboBox::getComboBox()
    {
        return *dynamic_cast<juce::ComboBox*>(getComponent().get());
    }

    const juce::ComboBox& ComboBox::getComboBox() const
    {
        return *dynamic_cast<const juce::ComboBox*>(getComponent().get());
    }

    void ComboBox::updateItems()
    {
        options.clear();
        getComboBox().clear();

        for (auto childState : state)
        {
            if (childState.hasType("Option"))
                options.add(std::make_unique<Option>(childState, options.size(), getComboBox()));
            else if (childState.hasType("Header"))
                headers.add(std::make_unique<Header>(childState, *this));
            else if (childState.hasType("Separator"))
                getComboBox().addSeparator();
        }
    }

    void ComboBox::valueTreeChildAdded(juce::ValueTree& parentState, juce::ValueTree& /* child */)
    {
        if (parentState != state)
            return;

        updateItems();
    }

    void ComboBox::valueTreeChildRemoved(juce::ValueTree& parentState, juce::ValueTree& /* child */, int /* index */)
    {
        if (parentState != state)
            return;

        updateItems();
    }

    void ComboBox::comboBoxChanged(juce::ComboBox* box)
    {
        jassertquiet(box == &getComboBox());

        selected = getComboBox().getSelectedItemIndex();
        onChange.triggerWithoutSelfCallback();
    }
} // namespace jive

#if JIVE_UNIT_TESTS
    #include <jive_layouts/layout/jive_Interpreter.h>

class ComboBoxTest : public juce::UnitTest
{
public:
    ComboBoxTest()
        : juce::UnitTest{ "jive::ComboBox", "jive" }
    {
    }

    void runTest() override
    {
        testGuiItem();
        testEditable();
        testTooltip();
        testOptions();
        testSelected();
        testEvents();
    }

private:
    void testGuiItem()
    {
        beginTest("gui-item");

        jive::Interpreter interpreter;
        auto item = interpreter.interpret(juce::ValueTree{
            "ComboBox",
            {
                { "width", 222 },
                { "height", 333 },
            },
        });
        expect(!item->isContainer());
    }

    void testEditable()
    {
        beginTest("editable");

        {
            juce::ValueTree tree{
                "ComboBox",
                {
                    { "width", 222 },
                    { "height", 333 },
                },
            };
            jive::Interpreter interpreter;
            auto item = interpreter.interpret(tree);
            auto& comboBox = dynamic_cast<jive::GuiItemDecorator&>(*item)
                                 .toType<jive::ComboBox>()
                                 ->getComboBox();
            expect(!comboBox.isTextEditable());

            tree.setProperty("editable", true, nullptr);
            expect(comboBox.isTextEditable());
        }
        {
            juce::ValueTree tree{
                "ComboBox",
                {
                    { "width", 222 },
                    { "height", 333 },
                    { "editable", true },
                },
            };
            jive::Interpreter interpreter;
            auto item = interpreter.interpret(tree);
            auto& comboBox = dynamic_cast<jive::GuiItemDecorator&>(*item)
                                 .toType<jive::ComboBox>()
                                 ->getComboBox();
            expect(comboBox.isTextEditable());
        }
    }

    void testTooltip()
    {
        beginTest("tooltip");

        {
            juce::ValueTree tree{
                "ComboBox",
                {
                    { "width", 222 },
                    { "height", 333 },
                },
            };
            jive::Interpreter interpreter;
            auto item = interpreter.interpret(tree);
            auto& comboBox = dynamic_cast<jive::GuiItemDecorator&>(*item)
                                 .toType<jive::ComboBox>()
                                 ->getComboBox();
            expect(comboBox.getTooltip().isEmpty());

            tree.setProperty("tooltip", "246", nullptr);
            expectEquals(comboBox.getTooltip(), juce::String{ "246" });
        }
        {
            juce::ValueTree tree{
                "ComboBox",
                {
                    { "width", 222 },
                    { "height", 333 },
                    { "tooltip", "369" },
                },
            };
            jive::Interpreter interpreter;
            auto item = interpreter.interpret(tree);
            auto& comboBox = dynamic_cast<jive::GuiItemDecorator&>(*item)
                                 .toType<jive::ComboBox>()
                                 ->getComboBox();
            expectEquals(comboBox.getTooltip(), juce::String{ "369" });
        }
    }

    void testOptions()
    {
        beginTest("options");

        {
            juce::ValueTree tree{
                "ComboBox",
                {
                    { "width", 222 },
                    { "height", 333 },
                },
            };
            jive::Interpreter interpreter;
            auto item = interpreter.interpret(tree);
            auto& comboBox = dynamic_cast<jive::GuiItemDecorator&>(*item)
                                 .toType<jive::ComboBox>()
                                 ->getComboBox();
            expectEquals(comboBox.getNumItems(), 0);

            tree.appendChild(juce::ValueTree{
                                 "Option",
                                 {
                                     { "text", "Choose me!" },
                                 },
                             },
                             nullptr);
            expectEquals(comboBox.getNumItems(), 1);
            expectEquals(comboBox.getItemText(0),
                         juce::String{ "Choose me!" });
            expectEquals(comboBox.getSelectedId(), 0);

            tree.getChild(0).setProperty("selected", true, nullptr);
            expectEquals(comboBox.getSelectedId(), 1);

            tree.appendChild(juce::ValueTree{
                                 "Option",
                                 {
                                     { "text", "Two" },
                                 },
                             },
                             nullptr);
            expectEquals(comboBox.getNumItems(), 2);
            expectEquals(comboBox.getItemText(1),
                         juce::String{ "Two" });
            expectEquals(comboBox.getSelectedId(), 1);

            tree.removeChild(0, nullptr);
            expectEquals(comboBox.getNumItems(), 1);
            expectEquals(comboBox.getSelectedId(), 0);

            tree.getChild(0).setProperty("enabled", false, nullptr);
            expect(!comboBox.isItemEnabled(1));

            tree.getChild(0).setProperty("text", "new text", nullptr);
            expectEquals(comboBox.getItemText(0), juce::String{ "new text" });

            tree.getChild(0).setProperty("selected", true, nullptr);
            expectEquals(comboBox.getSelectedId(), 1);

            tree.getChild(0).setProperty("selected", false, nullptr);
            expectEquals(comboBox.getSelectedId(), 0);
        }
        {
            juce::ValueTree tree{
                "ComboBox",
                {
                    { "width", 222 },
                    { "height", 333 },
                },
                {
                    juce::ValueTree{
                        "Option",
                        {
                            { "text", "One" },
                            { "selected", true },
                        },
                    },
                    juce::ValueTree{
                        "Option",
                        {
                            { "text", "Two" },
                            { "enabled", false },
                        },
                    },
                },
            };
            jive::Interpreter interpreter;
            auto item = interpreter.interpret(tree);
            auto& comboBox = dynamic_cast<jive::GuiItemDecorator&>(*item)
                                 .toType<jive::ComboBox>()
                                 ->getComboBox();
            expectEquals(comboBox.getNumItems(), 2);
            expectEquals(comboBox.getItemText(0), juce::String{ "One" });
            expectEquals(comboBox.getItemText(1), juce::String{ "Two" });
            expectEquals(comboBox.getSelectedId(), 1);
            expect(!comboBox.isItemEnabled(2));
        }
    }

    void testSelected()
    {
        beginTest("selected");

        {
            juce::ValueTree tree{
                "ComboBox",
                {
                    { "width", 222 },
                    { "height", 333 },
                },
            };
            jive::Interpreter interpreter;
            auto item = interpreter.interpret(tree);
            auto& comboBox = dynamic_cast<jive::GuiItemDecorator&>(*item)
                                 .toType<jive::ComboBox>()
                                 ->getComboBox();
            expectEquals(comboBox.getSelectedId(), 0);

            tree.appendChild(juce::ValueTree{ "Option", { { "text", "1" } } }, nullptr);
            tree.appendChild(juce::ValueTree{ "Option", { { "text", "2" } } }, nullptr);
            tree.appendChild(juce::ValueTree{ "Separator" }, nullptr);
            tree.appendChild(juce::ValueTree{ "Header", { { "text", "Header" } } }, nullptr);
            tree.appendChild(juce::ValueTree{ "Option", { { "text", "3" } } }, nullptr);
            expectEquals(comboBox.getSelectedId(), 0);

            tree.setProperty("selected", 0, nullptr);
            expectEquals(comboBox.getSelectedItemIndex(), 0);
            expect(tree.getChild(0)["selected"]);
            expect(!tree.getChild(1)["selected"]);
            expect(!tree.getChild(4)["selected"]);

            tree.setProperty("selected", 2, nullptr);
            expectEquals(comboBox.getSelectedItemIndex(), 2);
            expect(!tree.getChild(0)["selected"]);
            expect(!tree.getChild(1)["selected"]);
            expect(tree.getChild(4)["selected"]);

            tree.setProperty("selected", 100, nullptr);
            expectEquals(comboBox.getSelectedId(), 0);
            expect(!tree.getChild(0)["selected"]);
            expect(!tree.getChild(1)["selected"]);
            expect(!tree.getChild(4)["selected"]);

            comboBox.setSelectedItemIndex(1, juce::NotificationType::sendNotificationSync);
            expectEquals(static_cast<int>(tree["selected"]), 1);
            expect(!tree.getChild(0)["selected"]);
            expect(tree.getChild(1)["selected"]);
            expect(!tree.getChild(4)["selected"]);
        }
        {
            juce::ValueTree tree{
                "ComboBox",
                {
                    { "width", 222 },
                    { "height", 333 },
                    { "selected", 1 },
                },
                {
                    juce::ValueTree{ "Option", { { "text", "One" } } },
                    juce::ValueTree{ "Option", { { "text", "Two" } } },
                },
            };
            jive::Interpreter interpreter;
            auto item = interpreter.interpret(tree);
            auto& comboBox = dynamic_cast<jive::GuiItemDecorator&>(*item)
                                 .toType<jive::ComboBox>()
                                 ->getComboBox();
            expectEquals(comboBox.getSelectedItemIndex(), 1);
        }
    }

    void testEvents()
    {
        beginTest("events");

        juce::ValueTree tree{
            "ComboBox",
            {
                { "width", 222 },
                { "height", 333 },
            },
            {
                juce::ValueTree{
                    "Option",
                    {
                        { "text", "One" },
                        { "selected", true },
                    },
                },
                juce::ValueTree{
                    "Option",
                    {
                        { "text", "Two" },
                        { "enabled", false },
                    },
                },
            },
        };
        jive::Interpreter interpreter;
        auto item = interpreter.interpret(tree);
        auto& comboBox = dynamic_cast<jive::GuiItemDecorator&>(*item)
                             .toType<jive::ComboBox>()
                             ->getComboBox();
        jive::Event onChange{ tree, "on-change" };
        expectEquals(onChange.getAssumedTriggerCount(), 0);

        comboBox.setSelectedId(2, juce::sendNotificationSync);
        expectEquals(onChange.getAssumedTriggerCount(), 1);
    }
};

static ComboBoxTest comboBoxTest;
#endif
