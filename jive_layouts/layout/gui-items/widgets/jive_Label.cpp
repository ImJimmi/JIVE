#include "jive_Label.h"

namespace jive
{
    static juce::BorderSize<int> toNearestInt(juce::BorderSize<float> border)
    {
        return juce::BorderSize<int>{
            juce::roundToInt(border.getTop()),
            juce::roundToInt(border.getLeft()),
            juce::roundToInt(border.getBottom()),
            juce::roundToInt(border.getRight()),
        };
    }

    Label::Label(std::unique_ptr<GuiItem> itemToDecorate)
        : GuiItemDecorator{ std::move(itemToDecorate) }
        , border{ state, "border-width" }
    {
        border.onValueChange = [this]() {
            getLabel().setBorderSize(toNearestInt(border));
        };
        getLabel().setBorderSize(toNearestInt(border));
    }

    bool Label::isContainer() const
    {
        return false;
    }

    juce::Label& Label::getLabel()
    {
        auto* label = dynamic_cast<juce::Label*>(getComponent().get());
        jassert(label != nullptr);

        return *label;
    }

    const juce::Label& Label::getLabel() const
    {
        const auto* label = dynamic_cast<const juce::Label*>(getComponent().get());
        jassert(label != nullptr);

        return *label;
    }
} // namespace jive

#if JIVE_UNIT_TESTS
    #include <jive_layouts/layout/interpreter/jive_Interpreter.h>

class LabelUnitTest : public juce::UnitTest
{
public:
    LabelUnitTest()
        : juce::UnitTest{ "jive::Label", "jive" }
    {
    }

    void runTest() final
    {
        testGuiItem();
        testBorder();
    }

private:
    jive::Interpreter interpreter;

    [[nodiscard]] auto& getLabel(jive::GuiItem& item)
    {
        return dynamic_cast<jive::GuiItemDecorator&>(item).toType<jive::Label>()->getLabel();
    }

    void testGuiItem()
    {
        beginTest("gui-item");

        auto item = interpreter.interpret(juce::ValueTree{
            "Label",
            {
                { "width", 222 },
                { "height", 333 },
            },
        });
        expect(!item->isContainer());
    }

    void testBorder()
    {
        beginTest("border");

        juce::ValueTree tree{
            "Label",
            {
                { "width", 222 },
                { "height", 333 },
            },
        };
        auto item = interpreter.interpret(tree);
        auto& label = getLabel(*item);

        expect(label.getBorderSize().getTop() == 0);
        expect(label.getBorderSize().getRight() == 0);
        expect(label.getBorderSize().getBottom() == 0);
        expect(label.getBorderSize().getLeft() == 0);

        tree.setProperty("border-width", "5 10 20 40", nullptr);

        expect(label.getBorderSize().getTop() == 5);
        expect(label.getBorderSize().getRight() == 10);
        expect(label.getBorderSize().getBottom() == 20);
        expect(label.getBorderSize().getLeft() == 40);
    }
};

static LabelUnitTest labelUnitTest;
#endif
