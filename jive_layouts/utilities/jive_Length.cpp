#include "jive_Length.h"

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    void Length::setCorrespondingGuiItem(const GuiItem& correspondingItem)
    {
        item = &correspondingItem;
    }

    void Length::setPixels(float pixels)
    {
        unit = Unit::pixels;
        magnitude = pixels;
    }

    float Length::getPixels() const
    {
        jassert(isPixels());
        return magnitude;
    }

    bool Length::isPixels() const
    {
        return unit == Unit::pixels;
    }

    void Length::setPercent(float percent)
    {
        unit = Unit::percent;
        magnitude = percent;
    }

    float Length::getPercent() const
    {
        jassert(isPercent());
        jassert(item != nullptr);
        jassert(item->getParent() != nullptr);

        const auto scale = magnitude * 0.01f;
        return scale * getRelativeParentLength();
    }

    bool Length::isPercent() const
    {
        return unit == Unit::percent;
    }

    //==================================================================================================================
    Length::operator float() const
    {
        if (isPixels())
            return getPixels();

        if (isPercent())
            return getPercent();

        jassertfalse;
        return 0.f;
    }

    //==================================================================================================================
    Width Width::fromPixels(float pixels)
    {
        return createFromPixels<Width>(pixels);
    }

    Width Width::fromPercent(float percent, const GuiItem& correspondingItem)
    {
        return createFromPercent<Width>(percent, correspondingItem);
    }

    float Width::getRelativeParentLength() const
    {
        return item->getParent()->getViewport().getWidth();
    }

    //==================================================================================================================
    Height Height::fromPixels(float pixels)
    {
        return createFromPixels<Height>(pixels);
    }

    Height Height::fromPercent(float percent, const GuiItem& correspondingItem)
    {
        return createFromPercent<Height>(percent, correspondingItem);
    }

    float Height::getRelativeParentLength() const
    {
        return item->getParent()->getViewport().getHeight();
    }
} // namespace jive

//======================================================================================================================
#if JIVE_UNIT_TESTS
class LengthUnitTest : public juce::UnitTest
{
public:
    LengthUnitTest()
        : juce::UnitTest{ "jive::Length", "jive" }
    {
    }

    void runTest() final
    {
        testPixels();
        testPercent();
    }

private:
    void testPixels()
    {
        beginTest("pixels");

        auto item = std::make_unique<jive::GuiItem>(std::make_unique<juce::Component>(),
                                                    juce::ValueTree{ "Component" });
        jive::Width width;
        width.setCorrespondingGuiItem(*item);

        width.setPixels(10.f);
        expectEquals<float>(width, 10.f);

        width.setPixels(-312.65f);
        expectEquals<float>(width, -312.65f);
    }

    void testPercent()
    {
        beginTest("percent");

        {
            auto item = std::make_unique<jive::GuiItem>(std::make_unique<juce::Component>(),
                                                        juce::ValueTree{
                                                            "Component",
                                                            {
                                                                { "width", 40.f },
                                                                { "height", 20.f },
                                                            },
                                                        });
            item->addChild(std::make_unique<jive::GuiItem>(std::make_unique<juce::Component>(),
                                                           juce::ValueTree{ "Component" },
                                                           item.get()));

            jive::Width width;
            width.setCorrespondingGuiItem(item->getChild(0));

            width.setPercent(50.f);
            expectEquals<float>(width, 20.f);

            jive::Height height;
            height.setCorrespondingGuiItem(item->getChild(0));

            height.setPercent(20.f);
            expectWithinAbsoluteError<float>(height,
                                             4.f,
                                             0.000001f);
        }
        {
            jive::Interpreter interpreter;
            juce::ValueTree tree{
                "Component",
                {
                    { "display", "flex" },
                    { "width", 40.f },
                    { "height", 60.f },
                },
                {
                    juce::ValueTree{
                        "Component",
                        {
                            { "height", 20.f },
                        },
                        {
                            juce::ValueTree{ "Component" },
                        },
                    },
                },
            };
            auto item = interpreter.interpret(tree);

            jive::Width width;
            width.setCorrespondingGuiItem(item->getChild(0).getChild(0));

            width.setPercent(25.f);
            expectEquals<float>(width, 10.f);
        }
    }
};

static LengthUnitTest lengthUnitTest;
#endif
