#include "jive_ComponentFactory.h"

#include <jive_core/components/accessibility/jive_IgnoredComponent.h>
#include <jive_core/components/widgets/jive_ImageComponent.h>
#include <jive_core/components/widgets/jive_NormalisedProgressBar.h>
#include <jive_core/components/widgets/jive_TextComponent.h>

namespace jive
{
    ComponentFactory::ComponentFactory()
    {
        creators.insert({
            "Button",
            []() {
                return std::make_shared<juce::TextButton>();
            },
        });
        creators.insert({
            "Checkbox",
            []() {
                return std::make_shared<juce::ToggleButton>();
            },
        });
        creators.insert({
            "ComboBox",
            []() {
                return std::make_shared<juce::ComboBox>();
            },
        });
        creators.insert({
            "Component",
            []() {
                return std::make_shared<IgnoredComponent>();
            },
        });
        creators.insert({
            "Editor",
            []() {
                return std::make_shared<IgnoredComponent>();
            },
        });
        creators.insert({
            "Hyperlink",
            []() {
                return std::make_shared<juce::HyperlinkButton>();
            },
        });
        creators.insert({
            "Image",
            []() {
                return std::make_shared<ImageComponent>();
            },
        });
        creators.insert({
            "Knob",
            []() {
                return std::make_shared<juce::Slider>();
            },
        });
        creators.insert({
            "Label",
            []() {
                return std::make_shared<juce::Label>();
            },
        });
        creators.insert({
            "ProgressBar",
            []() {
                return std::make_shared<NormalisedProgressBar>();
            },
        });
        creators.insert({
            "Slider",
            []() {
                return std::make_shared<juce::Slider>();
            },
        });
        creators.insert({
            "Spinner",
            []() {
                return std::make_shared<juce::Slider>();
            },
        });
        creators.insert({
            "svg",
            []() {
                return std::make_shared<ImageComponent>();
            },
        });
        creators.insert({
            "Text",
            []() {
                return std::make_shared<TextComponent>();
            },
        });
        creators.insert({
            "Window",
            []() {
                return std::make_shared<IgnoredComponent>();
            },
        });
    }

    std::shared_ptr<juce::Component> ComponentFactory::create(juce::Identifier name) const
    {
        auto nameFactoryPair = creators.find(name);

        if (nameFactoryPair == std::end(creators))
            return nullptr;

        return nameFactoryPair->second();
    }

    void ComponentFactory::set(juce::Identifier name, ComponentCreator creator)
    {
        creators.insert({ name, creator });
    }
} // namespace jive

#if JIVE_UNIT_TESTS
class ComponentFactoryTest : public juce::UnitTest
{
public:
    ComponentFactoryTest()
        : juce::UnitTest{ "jive::ComponentFactory", "jive" }
    {
    }

    void runTest() final
    {
        testDefaultFactory();
        testCustomCreators();
    }

private:
    void testDefaultFactory()
    {
        beginTest("default-factory");

        const jive::ComponentFactory factory;

        expect(factory.create("FakeComponent") == nullptr);
        expect(dynamic_cast<juce::TextButton*>(factory.create("Button").get()) != nullptr);
    }

    void testCustomCreators()
    {
        beginTest("custom-creators");

        jive::ComponentFactory factory;
        expect(factory.create("Card") == nullptr);

        struct Card : public juce::Component
        {
        };

        factory.set("Card", []() {
            return std::make_unique<Card>();
        });
        expect(dynamic_cast<Card*>(factory.create("Card").get()) != nullptr);
    }
};

static ComponentFactoryTest componentFactoryTest;
#endif
