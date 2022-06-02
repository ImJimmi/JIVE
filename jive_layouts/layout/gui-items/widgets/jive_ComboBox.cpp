#include <jive_layouts/jive_layouts.h>

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    ComboBox::Option::Option(juce::ValueTree sourceTree, int itemIndex, juce::ComboBox& box)
        : tree{ sourceTree }
        , comboBox{ box }
        , index{ itemIndex }
        , id{ index + 1 }
        , text{ tree, "text" }
        , enabled{ tree, "enabled", true }
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

    //==================================================================================================================
    ComboBox::ComboBox(std::unique_ptr<GuiItem> itemToDecorate)
        : GuiItemDecorator(std::move(itemToDecorate))
        , TextWidget{ tree }
        , editable{ tree, "editable" }
        , tooltip{ tree, "tooltip" }
        , selected{ tree, "selected" }
    {
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

            auto currentlySelectedOption = tree.getChildWithProperty("selected", true);

            if (currentlySelectedOption.isValid())
                currentlySelectedOption.setProperty("selected", false, nullptr);

            if (selected < options.size())
                options[selected]->setSelected(true);
        };

        onJustificationChanged = [this]() {
            getComboBox().setJustificationType(getTextJustification());
        };
        getComboBox().setJustificationType(getTextJustification());

        onTextChanged = [this]() {
            getComboBox().setText(getText());
        };
        getComboBox().setText(getText());

        onFontChanged = [this]() {
            getComponent().getProperties().set("font", juce::VariantConverter<juce::Font>::toVar(getFont()));
        };
        getComponent().getProperties().set("font", juce::VariantConverter<juce::Font>::toVar(getFont()));

        updateItems();
        getComboBox().setSelectedItemIndex(selected);
        getComboBox().addListener(this);
    }

    //==================================================================================================================
    float ComboBox::getWidth() const
    {
        if (!hasAutoWidth())
            return GuiItemDecorator::getWidth();

        const auto textWidth = getFont().getStringWidthFloat(getText());
        const auto borderWidth = getBoxModel().getBorder().getLeftAndRight();

        return juce::jmax(textWidth + borderWidth, 20.f);
    }

    float ComboBox::getHeight() const
    {
        if (!hasAutoWidth())
            return GuiItemDecorator::getHeight();

        const auto textHeight = getFont().getHeight();
        const auto borderHeight = getBoxModel().getBorder().getTopAndBottom();

        return juce::jmax(textHeight + borderHeight, 20.f);
    }

    //==================================================================================================================
    juce::ComboBox& ComboBox::getComboBox()
    {
        return *dynamic_cast<juce::ComboBox*>(&getComponent());
    }

    const juce::ComboBox& ComboBox::getComboBox() const
    {
        return *dynamic_cast<const juce::ComboBox*>(&getComponent());
    }

    //==================================================================================================================
    void ComboBox::updateItems()
    {
        options.clear();
        getComboBox().clear();

        for (auto child : tree)
        {
            if (child.hasType("Option"))
                options.add(std::make_unique<Option>(child, options.size(), getComboBox()));
            else if (child.hasType("Header"))
                headers.add(std::make_unique<Header>(child, *this));
            else if (child.hasType("Separator"))
                getComboBox().addSeparator();
        }
    }

    //==================================================================================================================
    void ComboBox::valueTreeChildAdded(juce::ValueTree& parent, juce::ValueTree& child)
    {
        GuiItemDecorator::valueTreeChildAdded(parent, child);

        if (parent != tree)
            return;

        updateItems();
    }

    void ComboBox::valueTreeChildRemoved(juce::ValueTree& parent, juce::ValueTree& child, int index)
    {
        GuiItemDecorator::valueTreeChildRemoved(parent, child, index);

        if (parent != tree)
            return;

        updateItems();
    }

    //==================================================================================================================
    void ComboBox::comboBoxChanged(juce::ComboBox* box)
    {
        jassert(box == &getComboBox());
        selected = getComboBox().getSelectedItemIndex();
    }
} // namespace jive

//======================================================================================================================
#if JIVE_UNIT_TESTS
class ComboBoxTest : public juce::UnitTest
{
public:
    ComboBoxTest()
        : juce::UnitTest{ "jive::ComboBox", "jive" }
    {
    }

    void runTest() override
    {
        testEditable();
        testJustification();
        testFont();
        testTooltip();
        testOptions();
        testSelected();
        testAutoSize();
    }

private:
    std::unique_ptr<jive::ComboBox> createComboBox(juce::ValueTree tree)
    {
        return std::make_unique<jive::ComboBox>(std::make_unique<jive::GuiItem>(std::make_unique<juce::ComboBox>(),
                                                                                tree));
    }

    void testEditable()
    {
        beginTest("editable");

        {
            juce::ValueTree tree{ "ComboBox" };
            auto item = createComboBox(tree);
            expect(!item->getComboBox().isTextEditable());

            tree.setProperty("editable", true, nullptr);
            expect(item->getComboBox().isTextEditable());
        }
        {
            juce::ValueTree tree{
                "ComboBox",
                {
                    { "editable", true },
                },
            };
            auto item = createComboBox(tree);
            expect(item->getComboBox().isTextEditable());
        }
    }

    void testJustification()
    {
        beginTest("justification");

        {
            juce::ValueTree tree{ "ComboBox" };
            auto item = createComboBox(tree);
            expect(item->getComboBox().getJustificationType() == juce::Justification::centredLeft);

            tree.setProperty("justification", "bottom-right", nullptr);
            expect(item->getComboBox().getJustificationType() == juce::Justification::bottomRight);
        }
        {
            juce::ValueTree tree{
                "ComboBox",
                {
                    { "justification", "bottom-left" },
                },
            };
            auto item = createComboBox(tree);
            expect(item->getComboBox().getJustificationType() == juce::Justification::bottomLeft);
        }
    }

    void testText()
    {
        beginTest("text");

        {
            juce::ValueTree tree{ "ComboBox" };
            auto item = createComboBox(tree);
            expect(item->getComboBox().getText().isEmpty());

            tree.setProperty("text", "123", nullptr);
            expectEquals(item->getComboBox().getText(), juce::String{ "123" });
        }
        {
            juce::ValueTree tree{
                "ComboBox",
                {
                    { "text", "456" },
                },
            };
            auto item = createComboBox(tree);
            expectEquals(item->getComboBox().getText(), juce::String{ "456" });
        }
    }

    void testFont()
    {
        beginTest("font");

        {
            juce::ValueTree tree{ "ComboBox" };
            auto item = createComboBox(tree);

            expect(item->getComponent().getProperties().contains("font"));

            juce::Font font{ "Helvetica", 16.f, 0 };
            tree.setProperty("typeface-name", font.getTypefaceName(), nullptr);
            tree.setProperty("font-weight", font.getTypefaceStyle(), nullptr);
            tree.setProperty("font-height", font.getHeightInPoints(), nullptr);
            expectEquals(item->getComponent().getProperties()["font"].toString(), font.toString());
        }
        {
            juce::Font font{ "Arial", 48.f, 0 };
            juce::ValueTree tree{
                "Button",
                {
                    { "typeface-name", font.getTypefaceName() },
                    { "font-weight", font.getTypefaceStyle() },
                    { "font-height", font.getHeightInPoints() },
                },
            };
            auto item = createComboBox(tree);

            expectEquals(item->getComponent().getProperties()["font"].toString(), font.toString());
        }
    }

    void testTooltip()
    {
        beginTest("tooltip");

        {
            juce::ValueTree tree{ "ComboBox" };
            auto item = createComboBox(tree);
            expect(item->getComboBox().getTooltip().isEmpty());

            tree.setProperty("tooltip", "246", nullptr);
            expectEquals(item->getComboBox().getTooltip(), juce::String{ "246" });
        }
        {
            juce::ValueTree tree{
                "Button",
                {
                    { "tooltip", "369" },
                },
            };
            auto item = createComboBox(tree);
            expectEquals(item->getComboBox().getTooltip(), juce::String{ "369" });
        }
    }

    void testOptions()
    {
        beginTest("options");

        {
            juce::ValueTree tree{ "ComboBox" };
            auto item = createComboBox(tree);
            expectEquals(item->getComboBox().getNumItems(), 0);

            tree.appendChild(juce::ValueTree{
                                 "Option",
                                 {
                                     { "text", "Choose me!" },
                                 },
                             },
                             nullptr);
            expectEquals(item->getComboBox().getNumItems(), 1);
            expectEquals(item->getComboBox().getItemText(0),
                         juce::String{ "Choose me!" });
            expectEquals(item->getComboBox().getSelectedId(), 0);

            tree.getChild(0).setProperty("selected", true, nullptr);
            expectEquals(item->getComboBox().getSelectedId(), 1);

            tree.appendChild(juce::ValueTree{
                                 "Option",
                                 {
                                     { "text", "Two" },
                                 },
                             },
                             nullptr);
            expectEquals(item->getComboBox().getNumItems(), 2);
            expectEquals(item->getComboBox().getItemText(1),
                         juce::String{ "Two" });
            expectEquals(item->getComboBox().getSelectedId(), 1);

            tree.removeChild(0, nullptr);
            expectEquals(item->getComboBox().getNumItems(), 1);
            expectEquals(item->getComboBox().getSelectedId(), 0);

            tree.getChild(0).setProperty("enabled", false, nullptr);
            expect(!item->getComboBox().isItemEnabled(1));

            tree.getChild(0).setProperty("text", "new text", nullptr);
            expectEquals(item->getComboBox().getItemText(0), juce::String{ "new text" });

            tree.getChild(0).setProperty("selected", true, nullptr);
            expectEquals(item->getComboBox().getSelectedId(), 1);

            tree.getChild(0).setProperty("selected", false, nullptr);
            expectEquals(item->getComboBox().getSelectedId(), 0);
        }
        {
            juce::ValueTree tree{
                "ComboBox",
                {},
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
            auto item = createComboBox(tree);
            expectEquals(item->getComboBox().getNumItems(), 2);
            expectEquals(item->getComboBox().getItemText(0), juce::String{ "One" });
            expectEquals(item->getComboBox().getItemText(1), juce::String{ "Two" });
            expectEquals(item->getComboBox().getSelectedId(), 1);
            expect(!item->getComboBox().isItemEnabled(2));
        }
    }

    void testSelected()
    {
        beginTest("selected");

        {
            juce::ValueTree tree{ "ComboBox" };
            auto item = createComboBox(tree);
            expectEquals(item->getComboBox().getSelectedId(), 0);

            tree.appendChild(juce::ValueTree{ "Option", { { "text", "1" } } }, nullptr);
            tree.appendChild(juce::ValueTree{ "Option", { { "text", "2" } } }, nullptr);
            tree.appendChild(juce::ValueTree{ "Separator" }, nullptr);
            tree.appendChild(juce::ValueTree{ "Header", { { "text", "Header" } } }, nullptr);
            tree.appendChild(juce::ValueTree{ "Option", { { "text", "3" } } }, nullptr);
            expectEquals(item->getComboBox().getSelectedId(), 0);

            tree.setProperty("selected", 0, nullptr);
            expectEquals(item->getComboBox().getSelectedItemIndex(), 0);
            expect(tree.getChild(0)["selected"]);
            expect(!tree.getChild(1)["selected"]);
            expect(!tree.getChild(4)["selected"]);

            tree.setProperty("selected", 2, nullptr);
            expectEquals(item->getComboBox().getSelectedItemIndex(), 2);
            expect(!tree.getChild(0)["selected"]);
            expect(!tree.getChild(1)["selected"]);
            expect(tree.getChild(4)["selected"]);

            tree.setProperty("selected", 100, nullptr);
            expectEquals(item->getComboBox().getSelectedId(), 0);
            expect(!tree.getChild(0)["selected"]);
            expect(!tree.getChild(1)["selected"]);
            expect(!tree.getChild(4)["selected"]);

            item->getComboBox().setSelectedItemIndex(1, juce::NotificationType::sendNotificationSync);
            expectEquals(static_cast<int>(tree["selected"]), 1);
            expect(!tree.getChild(0)["selected"]);
            expect(tree.getChild(1)["selected"]);
            expect(!tree.getChild(4)["selected"]);
        }
        {
            juce::ValueTree tree{
                "ComboBox",
                {
                    { "selected", 1 },
                },
                {
                    juce::ValueTree{ "Option", { { "text", "One" } } },
                    juce::ValueTree{ "Option", { { "text", "Two" } } },
                },
            };
            auto item = createComboBox(tree);
            expectEquals(item->getComboBox().getSelectedItemIndex(), 1);
        }
    }

    void testAutoSize()
    {
        beginTest("auto size");

        juce::ValueTree tree{ "ComboBox" };
        auto item = createComboBox(tree);

        expectEquals(item->getWidth(), 20.f);
        expectEquals(item->getHeight(), 20.f);

        tree.setProperty("text", "Some text", nullptr);
        tree.setProperty("border-width", 20, nullptr);
        tree.setProperty("padding", 17, nullptr);

        const auto boxModel = item->getBoxModel();
        const auto borderWidth = boxModel.getBorder().getLeftAndRight();
        const auto textWidth = item->getFont().getStringWidthFloat(item->getText());
        const auto expectedWidth = borderWidth + textWidth;
        expect(item->getWidth() == expectedWidth);

        const auto borderHeight = boxModel.getBorder().getTopAndBottom();
        const auto textHeight = item->getFont().getHeight();
        const auto expectedHeight = borderHeight + textHeight;
        expect(item->getHeight() == expectedHeight);
    }
};

static ComboBoxTest comboBoxTest;
#endif
