#include "jive_ProgressBar.h"

namespace jive
{
    ProgressBar::ProgressBar(std::unique_ptr<GuiItem> itemToDecorate)
        : GuiItemDecorator{ std::move(itemToDecorate) }
        , value{ state, "value" }
        , width{ state, "width" }
        , height{ state, "height" }
        , focusable{ state, "focusable" }
    {
        const BoxModel::ScopedCallbackLock boxModelLock{ boxModel(*this) };

        if (!focusable.exists())
            focusable = true;

        value.onValueChange = [this]() {
            getProgressBar().setValue(juce::jlimit(0.0, 1.0, value.get()));
        };
        getProgressBar().setValue(juce::jlimit(0.0, 1.0, value.get()));

        getProgressBar().setPercentageDisplay(false);

        if (width.isAuto())
            width = "135";
        if (height.isAuto())
            height = "20";
    }

    bool ProgressBar::isContainer() const
    {
        return false;
    }

    NormalisedProgressBar& ProgressBar::getProgressBar()
    {
        return *dynamic_cast<NormalisedProgressBar*>(getComponent().get());
    }

    const NormalisedProgressBar& ProgressBar::getProgressBar() const
    {
        return *dynamic_cast<const NormalisedProgressBar*>(getComponent().get());
    }
} // namespace jive

#if JIVE_UNIT_TESTS
    #include <jive_layouts/layout/jive_Interpreter.h>

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
        testDefaultSize();
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
            juce::ValueTree tree{
                "ProgressBar",
                {
                    { "width", 222 },
                    { "height", 333 },
                },
            };
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
                    { "width", 222 },
                    { "height", 333 },
                    { "value", 0.463 },
                },
            };
            auto item = createProgressBar(tree);
            expectEquals(item->getProgressBar().getValue(), 0.463);
        }
    }

    void testDefaultSize()
    {
        beginTest("default size");

        juce::ValueTree parentState{
            "Component",
            {
                { "width", 999 },
                { "height", 999 },
            },
            {
                juce::ValueTree{ "ProgressBar" },
            },
        };
        jive::Interpreter interpreter;
        auto parent = interpreter.interpret(parentState);
        auto& progressBar = *dynamic_cast<jive::GuiItemDecorator&>(*parent->getChildren()[0])
                                 .toType<jive::ProgressBar>();
        expect(!progressBar.isContainer());
        const auto& boxModel = jive::boxModel(progressBar);
        expectEquals(boxModel.getWidth(), 135.0f);
        expectEquals(boxModel.getHeight(), 20.0f);
    }
};

static ProgressBarTest progressBarTest;
#endif
