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
        , border{ state, "border-width" }
    {
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

    //==================================================================================================================
    juce::Label& Label::getLabel()
    {
        auto* label = dynamic_cast<juce::Label*>(component.get());
        jassert(label != nullptr);

        return *label;
    }

    const juce::Label& Label::getLabel() const
    {
        const auto* label = dynamic_cast<const juce::Label*>(component.get());
        jassert(label != nullptr);

        return *label;
    }

    //==================================================================================================================
    void Label::contentChanged()
    {
        GuiItemDecorator::contentChanged();

        if (auto* text = findFirstTextContent(*this))
        {
            getLabel().setText(text->getTextComponent().getText(),
                               juce::sendNotification);
        }
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
        testContentChanged();
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

    void testContentChanged()
    {
        beginTest("content-changed");

        juce::ValueTree tree{
            "Label",
            {},
            {
                juce::ValueTree{
                    "Text",
                    {
                        { "text", "Some text..." },
                    },
                },
            },
        };
        auto item = createLabel(tree);
        expectEquals<juce::String>(item->getLabel().getText(), "Some text...");

        tree.getChild(0).setProperty("text", "Some different text!", nullptr);
        expectEquals<juce::String>(item->getLabel().getText(), "Some different text!");
    }
};

static LabelUnitTest labelUnitTest;
#endif
