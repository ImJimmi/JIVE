#include <jive_layouts/jive_layouts.h>

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    ProgressBar::ProgressBar(std::unique_ptr<GuiItem> itemToDecorate)
        : GuiItemDecorator{ std::move(itemToDecorate) }
        , TextWidget{ tree }
        , value{ tree, "value" }
        , showPercentage{ tree, "show-percentage", true }
    {
        value.onValueChange = [this]() {
            getProgressBar().setValue(juce::jlimit(0.0, 1.0, value.get()));
        };
        getProgressBar().setValue(juce::jlimit(0.0, 1.0, value.get()));

        showPercentage.onValueChange = [this]() {
            getProgressBar().setPercentageDisplay(showPercentage);
        };

        onTextChanged = [this]() {
            getProgressBar().setTextToDisplay(getText());
            getProgressBar().setPercentageDisplay(getText().isEmpty() && showPercentage);
        };
        getProgressBar().setTextToDisplay(getText());
        getProgressBar().setPercentageDisplay(getText().isEmpty() && showPercentage);

        onFontChanged = [this]() {
            getComponent().getProperties().set("font", juce::VariantConverter<juce::Font>::toVar(getFont()));
        };
        getComponent().getProperties().set("font", juce::VariantConverter<juce::Font>::toVar(getFont()));

        onJustificationChanged = [this]() {
            getComponent().getProperties().set("justification", juce::VariantConverter<juce::Justification>::toVar(getTextJustification()));
        };
        getComponent().getProperties().set("justification", juce::VariantConverter<juce::Justification>::toVar(getTextJustification()));
    }

    //==================================================================================================================
    NormalisedProgressBar& ProgressBar::getProgressBar()
    {
        return *dynamic_cast<NormalisedProgressBar*>(&getComponent());
    }

    const NormalisedProgressBar& ProgressBar::getProgressBar() const
    {
        return *dynamic_cast<const NormalisedProgressBar*>(&getComponent());
    }
} // namespace jive

//======================================================================================================================
#if JIVE_UNIT_TESTS
class ProgressBarTest : public juce::UnitTest
{
public:
    ProgressBarTest()
        : juce::UnitTest{ "jive::ProgressBar", "jive" }
    {
    }

    void runTest() final
    {
        testValue();
    }

private:
    std::unique_ptr<jive::ProgressBar> createProgressBar(juce::ValueTree tree)
    {
        jive::Interpreter interpreter;

        return std::make_unique<jive::ProgressBar>(interpreter.interpret(tree));
    }

    void testValue()
    {
        beginTest("value");

        {
            juce::ValueTree tree{ "ProgressBar" };
            auto item = createProgressBar(tree);
            expectEquals(item->getProgressBar().getValue(), 0.0);

            tree.setProperty("value", 0.7481, nullptr);
            expectEquals(item->getProgressBar().getValue(), 0.7481);

            tree.setProperty("value", -123.456, nullptr);
            expectEquals(item->getProgressBar().getValue(), 0.0);

            tree.setProperty("value", 9876.54321, nullptr);
            expectEquals(item->getProgressBar().getValue(), 1.0);
        }
        {
            juce::ValueTree tree{
                "ProgressBar",
                {
                    { "value", 0.463 },
                },
            };
            auto item = createProgressBar(tree);
            expectEquals(item->getProgressBar().getValue(), 0.463);
        }
    }
};

static ProgressBarTest progressBarTest;
#endif
