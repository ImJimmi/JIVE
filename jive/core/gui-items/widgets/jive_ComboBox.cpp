#include "jive_ComboBox.h"

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    ComboBox::ComboBox(std::unique_ptr<GuiItem> itemToDecorate)
        : GuiItem{ *itemToDecorate }
        , item{ std::move(itemToDecorate) }
        , text{ tree, "text" }
        , nothingSelectedText{ tree, "text-nothing-selected" }
        , noOptionsText{ tree, "text-no-options" }
        , editable{ tree, "editable" }
        , justification{ tree, "justification", juce::Justification::centredLeft }
    {
        text.onValueChange = [this]() {
            getComboBox().setText(text);
        };
        nothingSelectedText.onValueChange = [this]() {
            getComboBox().setTextWhenNothingSelected(nothingSelectedText);
        };
        noOptionsText.onValueChange = [this]() {
            getComboBox().setTextWhenNoChoicesAvailable(noOptionsText);
        };
        editable.onValueChange = [this]() {
            getComboBox().setEditableText(editable);
        };
        justification.onValueChange = [this]() {
            getComboBox().setJustificationType(justification);
        };

        resetComboBoxOptions();

        getComboBox().setText(text);
        getComboBox().setTextWhenNothingSelected(nothingSelectedText);
        getComboBox().setTextWhenNoChoicesAvailable(noOptionsText);
        getComboBox().setEditableText(editable);
        getComboBox().setJustificationType(justification);

        if (!tree.hasProperty("height"))
            tree.setProperty("height", 25, nullptr);
    }

    //==================================================================================================================
    bool ComboBox::isContainer() const
    {
        return false;
    }

    ComboBox::operator juce::FlexBox()
    {
        return { *item };
    }

    ComboBox::operator juce::FlexItem()
    {
        return { *item };
    }

    //==================================================================================================================
    juce::ComboBox& ComboBox::getComboBox()
    {
        auto* box = dynamic_cast<juce::ComboBox*>(&getComponent());
        jassert(box != nullptr);

        return *box;
    }

    //==================================================================================================================
    void ComboBox::valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyChanged,
                                            const juce::Identifier& propertyID)
    {
        item->valueTreePropertyChanged(treeWhosePropertyChanged, propertyID);

        if (treeWhosePropertyChanged.isAChildOf(tree))
            resetComboBoxOptions();
    }

    void ComboBox::valueTreeChildAdded(juce::ValueTree& parent, juce::ValueTree& child)
    {
        jassert(parent == tree);

        item->valueTreeChildAdded(parent, child);

        resetComboBoxOptions();
    }

    void ComboBox::valueTreeChildRemoved(juce::ValueTree& parent, juce::ValueTree& child, int childIndex)
    {
        jassert(parent == tree);

        item->valueTreeChildRemoved(parent, child, childIndex);

        resetComboBoxOptions();
    }

    //==================================================================================================================
    void addOption(ComboBox& boxGuiItem, juce::ValueTree tree)
    {
        // <Option> elements must have a 'text' property!
        jassert(tree.hasProperty("text"));

        auto& box = boxGuiItem.getComboBox();
        const auto itemID = box.getNumItems() + 1;

        box.addItem(tree["text"], itemID);

        const auto enabled = tree.getProperty("enabled", true);
        box.setItemEnabled(itemID, enabled);

        if (tree.getProperty("selected", false))
        {
            // Multiple options were selected!
            jassert(box.getSelectedId() == 0);
            box.setSelectedId(itemID);
        }
    }

    void addSeparator(ComboBox& boxGuiItem)
    {
        auto& box = boxGuiItem.getComboBox();

        // Can't have a separator as the first item!
        jassert(box.getRootMenu()->getNumItems() > 0);
        box.addSeparator();
    }

    void addHeader(ComboBox& boxGuiItem, juce::ValueTree tree)
    {
        auto& box = boxGuiItem.getComboBox();
        box.addSectionHeading(tree["text"]);
    }

    void ComboBox::resetComboBoxOptions()
    {
        getComboBox().clear();

        for (const auto& childTree : tree)
        {
            if (childTree.hasType("Option"))
                addOption(*this, childTree);
            else if (childTree.hasType("Separator"))
                addSeparator(*this);
            else if (childTree.hasType("Header"))
                addHeader(*this, childTree);
        }
    }
} // namespace jive
