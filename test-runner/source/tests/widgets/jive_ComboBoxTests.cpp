#include <utilities/test_ComponentUtilities.h>

#include <catch2/catch_test_macros.hpp>
#include <jive/jive.h>

//======================================================================================================================
SCENARIO("combo boxes have an initial height")
{
    GIVEN("a combo box GUI item")
    {
        juce::ValueTree tree{ "ComboBox" };
        jive::ComboBox item{ std::make_unique<jive::GuiItem>(std::make_unique<juce::ComboBox>(), tree) };

        THEN("the item has a height of 25")
        {
            REQUIRE(item.getHeight() == 25.f);
        }
    }
}

//======================================================================================================================
SCENARIO("combo boxes can have some text content")
{
    GIVEN("a combo box GUI item")
    {
        juce::ValueTree tree{ "ComboBox" };
        jive::ComboBox item{ std::make_unique<jive::GuiItem>(std::make_unique<juce::ComboBox>(), tree) };

        THEN("the item's combo box has no text")
        {
            REQUIRE(item.getComboBox().getText() == "");
        }

        WHEN("the item's text is changed")
        {
            tree.setProperty("text", "Some Text", nullptr);

            THEN("the text of the item's combo box matches")
            {
                REQUIRE(item.getComboBox().getText() == "Some Text");
            }
        }
    }

    GIVEN("a value-tree with its 'text' property set to a non-empty string")
    {
        juce::ValueTree tree{ "ComboBox", { { "text", "Not empty" } } };

        WHEN("a combo box GUI item is created from the tree")
        {
            jive::ComboBox item{ std::make_unique<jive::GuiItem>(std::make_unique<juce::ComboBox>(), tree) };

            THEN("the item's combo box has the same text")
            {
                REQUIRE(item.getComboBox().getText() == "Not empty");
            }
        }
    }
}

//======================================================================================================================
SCENARIO("combo boxes can have some text when nothing is selected")
{
    GIVEN("a combo box GUI item")
    {
        juce::ValueTree tree{ "ComboBox" };
        jive::ComboBox item{ std::make_unique<jive::GuiItem>(std::make_unique<juce::ComboBox>(), tree) };

        THEN("the text of the item's combo box when nothing is selected is empty")
        {
            auto* box = dynamic_cast<juce::ComboBox*>(&item.getComponent());
            REQUIRE(box != nullptr);

            REQUIRE(box->getTextWhenNothingSelected() == "");
        }

        WHEN("the text of the item when nothing is selected is changed")
        {
            tree.setProperty("text-nothing-selected", "Select Something", nullptr);

            THEN("the text of the item's combo box when nothing is selected matches")
            {
                auto* box = dynamic_cast<juce::ComboBox*>(&item.getComponent());
                REQUIRE(box != nullptr);

                REQUIRE(box->getTextWhenNothingSelected() == "Select Something");
            }
        }
    }

    GIVEN("a value-tree with its 'nothingSelectedText' property set to a non-empty string")
    {
        juce::ValueTree tree{ "ComboBox", { { "text-nothing-selected", "Apple pie" } } };

        WHEN("a combo box GUI item is created from the tree")
        {
            jive::ComboBox item{ std::make_unique<jive::GuiItem>(std::make_unique<juce::ComboBox>(), tree) };

            THEN("the item's combo box has the same text")
            {
                REQUIRE(item.getComboBox().getTextWhenNothingSelected() == "Apple pie");
            }
        }
    }
}

//======================================================================================================================
SCENARIO("combo boxes can have some text when there are no options")
{
    GIVEN("a combo box GUI item")
    {
        juce::ValueTree tree{ "ComboBox" };
        jive::ComboBox item{ std::make_unique<jive::GuiItem>(std::make_unique<juce::ComboBox>(), tree) };

        THEN("the text of the item's combo box when there are no options is empty")
        {
            auto* box = dynamic_cast<juce::ComboBox*>(&item.getComponent());
            REQUIRE(box != nullptr);

            REQUIRE(box->getTextWhenNoChoicesAvailable() == "");
        }

        WHEN("the text of the item when there are no options is changed")
        {
            tree.setProperty("text-no-options", "No Options", nullptr);

            THEN("the text of the item's combo box when there are no options matches")
            {
                auto* box = dynamic_cast<juce::ComboBox*>(&item.getComponent());
                REQUIRE(box != nullptr);

                REQUIRE(box->getTextWhenNoChoicesAvailable() == "No Options");
            }
        }
    }
}

//======================================================================================================================
SCENARIO("combo boxes' text can be edited")
{
    GIVEN("a combo box GUI item")
    {
        juce::ValueTree tree{ "ComboBox" };
        jive::ComboBox item{ std::make_unique<jive::GuiItem>(std::make_unique<juce::ComboBox>(), tree) };

        THEN("the text of the item's component is not editable")
        {
            auto* box = dynamic_cast<juce::ComboBox*>(&item.getComponent());
            REQUIRE(box != nullptr);

            REQUIRE_FALSE(box->isTextEditable());
        }

        WHEN("the box's text is set to be editable")
        {
            tree.setProperty("editable", true, nullptr);

            THEN("the text of the item's component is editable")
            {
                auto* box = dynamic_cast<juce::ComboBox*>(&item.getComponent());
                REQUIRE(box != nullptr);

                REQUIRE(box->isTextEditable());
            }
        }
    }
}

//======================================================================================================================
SCENARIO("combo boxes can justify their text")
{
    GIVEN("a combo box GUI item")
    {
        juce::ValueTree tree{ "ComboBox" };
        jive::ComboBox item{ std::make_unique<jive::GuiItem>(std::make_unique<juce::ComboBox>(), tree) };

        THEN("the box's text is justified to the centre-left")
        {
            REQUIRE(item.getComboBox().getJustificationType() == juce::Justification::centredLeft);
        }

        WHEN("the box's text is set to be justified in the top-left")
        {
            tree.setProperty("justification",
                             juce::VariantConverter<juce::Justification>::toVar(juce::Justification::topLeft),
                             nullptr);

            THEN("the box's text is justified in the top-left")
            {
                REQUIRE(item.getComboBox().getJustificationType() == juce::Justification::topLeft);
            }
        }
        WHEN("the box's text is set to be justified to the centred-top")
        {
            tree.setProperty("justification",
                             juce::VariantConverter<juce::Justification>::toVar(juce::Justification::centredTop),
                             nullptr);

            THEN("the box's text is justified in the centred-top")
            {
                REQUIRE(item.getComboBox().getJustificationType() == juce::Justification::centredTop);
            }
        }
        WHEN("the box's text is set to be justified to the top-right")
        {
            tree.setProperty("justification",
                             juce::VariantConverter<juce::Justification>::toVar(juce::Justification::topRight),
                             nullptr);

            THEN("the box's text is justified in the top-right")
            {
                REQUIRE(item.getComboBox().getJustificationType() == juce::Justification::topRight);
            }
        }
        WHEN("the box's text is set to be justified to the centre-right")
        {
            tree.setProperty("justification",
                             juce::VariantConverter<juce::Justification>::toVar(juce::Justification::centredRight),
                             nullptr);

            THEN("the box's text is justified in the top-right")
            {
                REQUIRE(item.getComboBox().getJustificationType() == juce::Justification::centredRight);
            }
        }
        WHEN("the box's text is set to be justified to the bottom-right")
        {
            tree.setProperty("justification",
                             juce::VariantConverter<juce::Justification>::toVar(juce::Justification::bottomRight),
                             nullptr);

            THEN("the box's text is justified in the bottom-right")
            {
                REQUIRE(item.getComboBox().getJustificationType() == juce::Justification::bottomRight);
            }
        }
        WHEN("the box's text is set to be justified to the centre-bottom")
        {
            tree.setProperty("justification",
                             juce::VariantConverter<juce::Justification>::toVar(juce::Justification::centredBottom),
                             nullptr);

            THEN("the box's text is justified in the centre-bottom")
            {
                REQUIRE(item.getComboBox().getJustificationType() == juce::Justification::centredBottom);
            }
        }
        WHEN("the box's text is set to be justified to the bottom-left")
        {
            tree.setProperty("justification",
                             juce::VariantConverter<juce::Justification>::toVar(juce::Justification::bottomLeft),
                             nullptr);

            THEN("the box's text is justified in the bottom-left")
            {
                REQUIRE(item.getComboBox().getJustificationType() == juce::Justification::bottomLeft);
            }
        }
        WHEN("the box's text is set to be justified to the centred")
        {
            tree.setProperty("justification",
                             juce::VariantConverter<juce::Justification>::toVar(juce::Justification::centred),
                             nullptr);

            THEN("the box's text is justified in the centre")
            {
                REQUIRE(item.getComboBox().getJustificationType() == juce::Justification::centred);
            }
        }
    }

    GIVEN("a value-tree with a specified 'justification' property")
    {
        juce::ValueTree tree{
            "ComboBox",
            {
                { "justification", juce::VariantConverter<juce::Justification>::toVar(juce::Justification::bottomRight) }
            }
        };

        WHEN("a combo box GUI item is created from the tree")
        {
            jive::ComboBox item{ std::make_unique<jive::GuiItem>(std::make_unique<juce::ComboBox>(), tree) };

            THEN("the text of the item's combo box has the same justification")
            {
                REQUIRE(item.getComboBox().getJustificationType() == juce::Justification::bottomRight);
            }
        }
    }
}

//======================================================================================================================
SCENARIO("combo boxes can have multiple options in their drop-down menus")
{
    GIVEN("a combo box GUI item")
    {
        juce::ValueTree tree{ "ComboBox" };
        jive::ComboBox item{ std::make_unique<jive::GuiItem>(std::make_unique<juce::ComboBox>(), tree) };

        THEN("the item's combo box should have no options")
        {
            REQUIRE(item.getComboBox().getNumItems() == 0);
        }

        WHEN("the box is given some <Option> children with different 'text' properties")
        {
            tree.appendChild(juce::ValueTree{ "Option", { { "text", "One" } } }, nullptr);
            tree.appendChild(juce::ValueTree{ "Option", { { "text", "Two" } } }, nullptr);

            THEN("the item has the same number of options")
            {
                REQUIRE(item.getComboBox().getNumItems() == 2);
            }

            AND_WHEN("some of the options are removed")
            {
                tree.removeChild(0, nullptr);

                THEN("the item's combo box has the same number of remaining options")
                {
                    REQUIRE(item.getComboBox().getNumItems() == 1);
                }
            }
        }
    }

    GIVEN("a value-tree with some <Option> children")
    {
        juce::ValueTree tree{
            "ComboBox",
            {
            },
            {
                juce::ValueTree{ "Option", { { "text", "One" } } },
                juce::ValueTree{ "Option", { { "text", "Two" } } },
                juce::ValueTree{ "Option", { { "text", "Three" } } }
            }
        };

        WHEN("a combo box GUI item is created from the value-tree")
        {
            jive::ComboBox item{ std::make_unique<jive::GuiItem>(std::make_unique<juce::ComboBox>(), tree) };

            THEN("the item's combo box has the same number of options as the value-tree")
            {
                REQUIRE(item.getComboBox().getNumItems() == 3);
            }
        }
    }
}

//======================================================================================================================
SCENARIO("a combo box's options can be selected")
{
    GIVEN("a combo box GUI item with some options")
    {
        juce::ValueTree option1{ "Option", { { "text", "one" } } };
        juce::ValueTree option2{ "Option", { { "text", "two" } } };
        juce::ValueTree tree { "ComboBox", {}, { option1, option2 } };
        jive::ComboBox item{ std::make_unique<jive::GuiItem>(std::make_unique<juce::ComboBox>(), tree) };

        THEN("the box has no options selected")
        {
            REQUIRE(item.getComboBox().getSelectedId() == 0);
        }

        WHEN("one of the options is selected")
        {
            option1.setProperty("selected", true, nullptr);

            THEN("the same option in the box is selected")
            {
                REQUIRE(item.getComboBox().getSelectedId() == 1);
            }
        }
    }
}

//======================================================================================================================
SCENARIO("a combo box's options can be disabled")
{
    GIVEN("a combo box GUI item with some options")
    {
        juce::ValueTree tree{
            "ComboBox",
            {
            },
            {
                juce::ValueTree{ "Option", { { "text", "One" } } },
                juce::ValueTree{ "Option", { { "text", "Two" } } }
            }
        };
        jive::ComboBox item{ std::make_unique<jive::GuiItem>(std::make_unique<juce::ComboBox>(), tree) };

        THEN("all of the options in the item's combo box are enabled")
        {
            REQUIRE(item.getComboBox().isItemEnabled(1));
            REQUIRE(item.getComboBox().isItemEnabled(2));
        }

        WHEN("some of the item's options are disabled")
        {
            tree.getChild(0).setProperty("enabled", false, nullptr);

            THEN("the option in the item's combo box is not enabled")
            {
                REQUIRE_FALSE(item.getComboBox().isItemEnabled(0));
            }
        }
    }
}

//======================================================================================================================
SCENARIO("combo box GUI items can have separators")
{
    GIVEN("a combo box GUI item")
    {
        juce::ValueTree tree{ "ComboBox" };
        jive::ComboBox item{ std::make_unique<jive::GuiItem>(std::make_unique<juce::ComboBox>(), tree) };

        THEN("the item's combo box has no separators")
        {
            REQUIRE(test::getPopupMenuNumSeparators(*item.getComboBox().getRootMenu()) == 0);
        }

        WHEN("some separator items are added to the box")
        {
            tree.appendChild(juce::ValueTree{ "Option", { { "text", "Dummy" } } }, nullptr);
            tree.appendChild(juce::ValueTree{ "Separator" }, nullptr);
            tree.appendChild(juce::ValueTree{ "Option", { { "text", "Another dummy" } } }, nullptr);
            tree.appendChild(juce::ValueTree{ "Separator" }, nullptr);

            THEN("the item's combo box has the same number of separators")
            {
                REQUIRE(test::getPopupMenuNumSeparators(*item.getComboBox().getRootMenu()) == 2);
            }
        }
    }
}

//======================================================================================================================
SCENARIO("combo box GUI items can have headers")
{
    GIVEN("a combo box GUI item")
    {
        juce::ValueTree tree{ "ComboBox" };
        jive::ComboBox item{ std::make_unique<jive::GuiItem>(std::make_unique<juce::ComboBox>(), tree) };

        THEN("the item's combo box has no headers")
        {
            REQUIRE(test::getPopupMenuNumHeaders(*item.getComboBox().getRootMenu()) == 0);
        }

        WHEN("some headers are added to the box")
        {
            tree.appendChild(juce::ValueTree{ "Header", { { "text", "Section One" } } }, nullptr);
            tree.appendChild(juce::ValueTree{ "Header", { { "text", "Section Two" } } }, nullptr);

            THEN("the item's combo box has the same number of headers")
            {
                REQUIRE(test::getPopupMenuNumHeaders(*item.getComboBox().getRootMenu()) == 2);
            }
        }
    }
}
