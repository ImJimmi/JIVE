#include <utilities/test_ComponentUtilities.h>

#include <catch2/catch_test_macros.hpp>
#include <jive_layouts/jive_layouts.h>

//======================================================================================================================
namespace juce
{
    std::ostream& operator<<(std::ostream& stream, const juce::Font& font)
    {
        stream << font.toString();
        return stream;
    }
} // namespace juce

//======================================================================================================================
SCENARIO("label GUI items can have some text content")
{
    GIVEN("a label GUI item")
    {
        juce::ValueTree tree{ "Label" };
        jive::Label item{ std::make_unique<jive::GuiItem>(std::make_unique<juce::Label>(), tree) };

        THEN("the item's label has no text")
        {
            REQUIRE(item.getLabel().getText() == "");
        }

        WHEN("the item's text is changed")
        {
            tree.setProperty("text", "Some Text", nullptr);

            THEN("the text of the item's label matches")
            {
                REQUIRE(item.getLabel().getText() == "Some Text");
            }
        }
    }

    GIVEN("a value-tree with its 'text' property set to a non-empty string")
    {
        juce::ValueTree tree{ "Label", { { "text", "Not empty" } } };

        WHEN("a label GUI item is created from the tree")
        {
            jive::Label item{ std::make_unique<jive::GuiItem>(std::make_unique<juce::Label>(), tree) };

            THEN("the item's label has the same text")
            {
                REQUIRE(item.getLabel().getText() == "Not empty");
            }
        }
    }
}

//======================================================================================================================
SCENARIO("label GUI items can have different fonts")
{
    GIVEN("a label GUI item")
    {
        juce::ValueTree tree{ "Label" };
        jive::Label item{ std::make_unique<jive::GuiItem>(std::make_unique<juce::Label>(), tree) };

        THEN("the font of the item's label is the default juce::Label font with a point height of 12")
        {
            REQUIRE(item.getFont() == juce::Font{ 0.f }.withPointHeight(12.f));
        }

        WHEN("the item's typeface name is changed")
        {
            tree.setProperty("typeface-name", "Helvetica", nullptr);

            THEN("the font of the item's label has the same typeface name")
            {
                REQUIRE(item.getFont().getTypefaceName() == "Helvetica");
            }
        }
        WHEN("the item's font weight is changed")
        {
            tree.setProperty("font-weight", "Thin", nullptr);

            THEN("the font of the item's label has the same weight")
            {
                REQUIRE(item.getFont().getTypefaceStyle() == "Thin");
            }
        }
        WHEN("the item's font height is changed")
        {
            tree.setProperty("font-height", 35.f, nullptr);

            THEN("the font of the item's label has a point height of 35.f")
            {
                REQUIRE(item.getLabel().getFont() == item.getLabel().getFont().withPointHeight(35.f));
            }
        }
        WHEN("the item's font style flags are changed")
        {
            tree.setProperty("font-style", "bold italic underlined", nullptr);

            THEN("the font of the item's label has the style styles")
            {
                REQUIRE(item.getFont().isBold());
                REQUIRE(item.getFont().isItalic());
                REQUIRE(item.getFont().isUnderlined());
            }
        }
    }
    GIVEN("A value-tree with font properties already specified")
    {
        juce::ValueTree tree{
            "Label",
            { { "typeface-name", "Georgia" },
              { "font-height", "23.4" },
              { "font-style", "bold" } }
        };

        WHEN("a label GUI item is created from the value-tree")
        {
            jive::Label item{ std::make_unique<jive::GuiItem>(std::make_unique<juce::Label>(), tree) };

            THEN("the item's label has the correct font")
            {
                const auto expectedFont = juce::Font{ "Georgia", 0.f, juce::Font::bold }.withPointHeight(23.4f);
                REQUIRE(item.getFont() == expectedFont);
            }
        }
    }
}

//======================================================================================================================
SCENARIO("label GUI items can justify their text differently")
{
    GIVEN("a label GUI item")
    {
        juce::ValueTree tree{ "Label" };
        jive::Label item{ std::make_unique<jive::GuiItem>(std::make_unique<juce::Label>(), tree) };

        THEN("the label's text is justified to the centre-left")
        {
            REQUIRE(item.getLabel().getJustificationType() == juce::Justification::centredLeft);
        }

        WHEN("the label's text is set to be justified in the top-left")
        {
            tree.setProperty("justification",
                             juce::VariantConverter<juce::Justification>::toVar(juce::Justification::topLeft),
                             nullptr);

            THEN("the label's text is justified in the top-left")
            {
                REQUIRE(item.getLabel().getJustificationType() == juce::Justification::topLeft);
            }
        }
        WHEN("the label's text is set to be justified to the centred-top")
        {
            tree.setProperty("justification",
                             juce::VariantConverter<juce::Justification>::toVar(juce::Justification::centredTop),
                             nullptr);

            THEN("the label's text is justified in the centred-top")
            {
                REQUIRE(item.getLabel().getJustificationType() == juce::Justification::centredTop);
            }
        }
        WHEN("the label's text is set to be justified to the top-right")
        {
            tree.setProperty("justification",
                             juce::VariantConverter<juce::Justification>::toVar(juce::Justification::topRight),
                             nullptr);

            THEN("the label's text is justified in the top-right")
            {
                REQUIRE(item.getLabel().getJustificationType() == juce::Justification::topRight);
            }
        }
        WHEN("the label's text is set to be justified to the centre-right")
        {
            tree.setProperty("justification",
                             juce::VariantConverter<juce::Justification>::toVar(juce::Justification::centredRight),
                             nullptr);

            THEN("the label's text is justified in the top-right")
            {
                REQUIRE(item.getLabel().getJustificationType() == juce::Justification::centredRight);
            }
        }
        WHEN("the label's text is set to be justified to the bottom-right")
        {
            tree.setProperty("justification",
                             juce::VariantConverter<juce::Justification>::toVar(juce::Justification::bottomRight),
                             nullptr);

            THEN("the label's text is justified in the bottom-right")
            {
                REQUIRE(item.getLabel().getJustificationType() == juce::Justification::bottomRight);
            }
        }
        WHEN("the label's text is set to be justified to the centre-bottom")
        {
            tree.setProperty("justification",
                             juce::VariantConverter<juce::Justification>::toVar(juce::Justification::centredBottom),
                             nullptr);

            THEN("the label's text is justified in the centre-bottom")
            {
                REQUIRE(item.getLabel().getJustificationType() == juce::Justification::centredBottom);
            }
        }
        WHEN("the label's text is set to be justified to the bottom-left")
        {
            tree.setProperty("justification",
                             juce::VariantConverter<juce::Justification>::toVar(juce::Justification::bottomLeft),
                             nullptr);

            THEN("the label's text is justified in the bottom-left")
            {
                REQUIRE(item.getLabel().getJustificationType() == juce::Justification::bottomLeft);
            }
        }
        WHEN("the label's text is set to be justified to the centred")
        {
            tree.setProperty("justification",
                             juce::VariantConverter<juce::Justification>::toVar(juce::Justification::centred),
                             nullptr);

            THEN("the label's text is justified in the centre")
            {
                REQUIRE(item.getLabel().getJustificationType() == juce::Justification::centred);
            }
        }
    }

    GIVEN("a value-tree with a specified 'justification' property")
    {
        juce::ValueTree tree{
            "Label",
            { { "justification", juce::VariantConverter<juce::Justification>::toVar(juce::Justification::bottomRight) } }
        };

        WHEN("a label GUI item is created from the tree")
        {
            jive::Label item{ std::make_unique<jive::GuiItem>(std::make_unique<juce::Label>(), tree) };

            THEN("the text of the item's label has the same justification")
            {
                REQUIRE(item.getLabel().getJustificationType() == juce::Justification::bottomRight);
            }
        }
    }
}

//======================================================================================================================
SCENARIO("unless specified, a label GUI item's size matches the size of its text")
{
    GIVEN("a label GUI item")
    {
        juce::ValueTree tree{
            "Label",
            { { "text", "Some text" } }
        };
        jive::Label item{ std::make_unique<jive::GuiItem>(std::make_unique<juce::Label>(), tree) };

        THEN("the item's width matches the width of the its text, plus the border size of the item's label")
        {
            const auto expectedWidth = item.getFont().getStringWidthFloat(item.getText()) + item.getLabel().getBorderSize().getLeftAndRight();
            REQUIRE(item.getWidth() == expectedWidth);
        }
        THEN("the item's height matches the height of its font, plus the border size of the item's label")
        {
            const auto expectedHeight = item.getFont().getHeight() + item.getLabel().getBorderSize().getTopAndBottom();
            REQUIRE(item.getHeight() == expectedHeight);
        }

        WHEN("the item's size is changed")
        {
            tree.setProperty("width", 246, nullptr);
            tree.setProperty("height", 120, nullptr);

            THEN("the item's width matches the specified width")
            {
                REQUIRE(item.getWidth() == 246.f);
            }
            THEN("the item's height matches the specified height")
            {
                REQUIRE(item.getHeight() == 120.f);
            }
        }
    }
}
