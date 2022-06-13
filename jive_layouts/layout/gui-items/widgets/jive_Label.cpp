#include <jive_layouts/jive_layouts.h>

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    juce::BorderSize<int> toNearestInt(juce::BorderSize<float> border)
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
        , TextWidget{ tree }
        , border{ tree, "border-width" }
    {
        onTextChanged = [this]() {
            getLabel().setText(getText(), juce::sendNotification);
        };
        getLabel().setText(getText(), juce::sendNotification);

        onFontChanged = [this]() {
            getLabel().setFont(getFont());
        };
        getLabel().setFont(getFont());

        onJustificationChanged = [this]() {
            getLabel().setJustificationType(getTextJustification());
        };
        getLabel().setJustificationType(getTextJustification());

        border.onValueChange = [this]() {
            getLabel().setBorderSize(toNearestInt(border));
        };
        getLabel().setBorderSize(toNearestInt(border));
    }

    //==================================================================================================================
    bool Label::isContainer() const
    {
        return false;
    }

    float Label::getWidth() const
    {
        if (hasAutoWidth())
        {
            const auto textWidth = getFont().getStringWidthFloat(getText());
            const auto borderWidth = getBoxModel().getBorder().getLeftAndRight();

            return textWidth + borderWidth;
        }

        return GuiItemDecorator::getWidth();
    }

    float Label::getHeight() const
    {
        if (hasAutoHeight())
        {
            const auto textHeight = getFont().getHeight();
            const auto borderHeight = getLabel().getBorderSize().getTopAndBottom();

            return textHeight + borderHeight;
        }

        return GuiItemDecorator::getHeight();
    }

    //==================================================================================================================
    juce::Label& Label::getLabel()
    {
        auto* label = dynamic_cast<juce::Label*>(&getComponent());
        jassert(label != nullptr);

        return *label;
    }

    const juce::Label& Label::getLabel() const
    {
        const auto* label = dynamic_cast<const juce::Label*>(&getComponent());
        jassert(label != nullptr);

        return *label;
    }
} // namespace jive

//======================================================================================================================
#if JIVE_UNIT_TESTS
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
        testAutoSize();
    }

private:
    std::unique_ptr<jive::Label> createLabel(juce::ValueTree tree)
    {
        jive::Interpreter interpreter;

        return std::make_unique<jive::Label>(interpreter.interpret(tree));
    }

    void testGuiItem()
    {
        beginTest("gui-item");

        auto item = createLabel(juce::ValueTree{ "Label" });
        expect(!item->isContainer());
    }

    void testBorder()
    {
        beginTest("border");

        juce::ValueTree tree{ "Label" };
        auto label = createLabel(tree);

        expect(label->getLabel().getBorderSize().getTop() == 0);
        expect(label->getLabel().getBorderSize().getRight() == 0);
        expect(label->getLabel().getBorderSize().getBottom() == 0);
        expect(label->getLabel().getBorderSize().getLeft() == 0);

        tree.setProperty("border-width", "5 10 20 40", nullptr);

        expect(label->getLabel().getBorderSize().getTop() == 5);
        expect(label->getLabel().getBorderSize().getRight() == 10);
        expect(label->getLabel().getBorderSize().getBottom() == 20);
        expect(label->getLabel().getBorderSize().getLeft() == 40);
    }

    void testAutoSize()
    {
        beginTest("auto size");

        juce::ValueTree tree{
            "Label",
            { { "text", "Some text" },
              { "border-width", 30 },
              { "padding", 5 } }
        };
        auto label = createLabel(tree);

        const auto boxModel = label->getBoxModel();
        const auto borderWidth = boxModel.getBorder().getLeftAndRight();
        const auto textWidth = label->getFont().getStringWidthFloat(label->getText());
        const auto expectedWidth = borderWidth + textWidth;
        expect(label->getWidth() == expectedWidth);

        const auto borderHeight = boxModel.getBorder().getTopAndBottom();
        const auto textHeight = label->getFont().getHeight();
        const auto expectedHeight = borderHeight + textHeight;
        expect(label->getHeight() == expectedHeight);
    }
};

static LabelUnitTest labelUnitTest;
#endif
