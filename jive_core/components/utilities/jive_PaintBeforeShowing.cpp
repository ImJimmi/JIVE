#include "jive_PaintBeforeShowing.h"

#include <jive_core/graphics/jive_LookAndFeel.h>

namespace jive
{
    PaintBeforeShowing::PaintBeforeShowing(juce::Component& componentToWatch)
        : juce::ComponentMovementWatcher{ &componentToWatch }
    {
    }

    void PaintBeforeShowing::componentMovedOrResized(bool, bool)
    {
    }

    void PaintBeforeShowing::componentPeerChanged()
    {
    }

    [[nodiscard]] static juce::Array<LookAndFeel*> findLookAndFeels(const juce::Component& component)
    {
        juce::Array<LookAndFeel*> lookAndFeels;

        if (auto* lookAndFeel = dynamic_cast<LookAndFeel*>(&component.getLookAndFeel()))
            lookAndFeels.add(lookAndFeel);

        for (const auto* child : component.getChildren())
        {
            for (auto* lookAndFeel : findLookAndFeels(*child))
                lookAndFeels.addIfNotAlreadyThere(lookAndFeel);
        }

        return lookAndFeels;
    }

    // JUCE notifies visibility listeners before it shows the native window.
    void PaintBeforeShowing::componentVisibilityChanged()
    {
        auto* watchedComponent = getComponent();

        if (watchedComponent == nullptr || !watchedComponent->isShowing())
            return;

        if (auto* peer = watchedComponent->getPeer())
        {
            // Painting asks for some components, such as shadows, which are
            // only added on the next frame. Painting off-screen and adding
            // them first means they're all in the window's first frame.
            auto& window = peer->getComponent();
            juce::ignoreUnused(window.createComponentSnapshot(window.getLocalBounds()));

            for (auto* lookAndFeel : findLookAndFeels(window))
                lookAndFeel->addAndRemoveShadows();

            peer->performAnyPendingRepaintsNow();
        }
    }
} // namespace jive

#if JIVE_UNIT_TESTS
class PaintBeforeShowingUnitTest : public juce::UnitTest
{
public:
    PaintBeforeShowingUnitTest()
        : juce::UnitTest{ "jive::PaintBeforeShowing", "jive" }
    {
    }

    void runTest() final
    {
        beginTest("showing the component's window paints it straight away");
        {
            Window window;
            jive::PaintBeforeShowing paintBeforeShowing{ window.content };

            window.setVisible(true);
            expectGreaterThan(window.content.numPaints, 0);
        }

        beginTest("the first frame includes the shadows that painting asks for");
        {
            Window window;
            jive::LookAndFeel lookAndFeel{ window };
            lookAndFeel.applyTheme(jive::Theme::juce);
            window.content.setComponentID("content");
            lookAndFeel.addStyles("#content", jive::Styles{}.withBackground(juce::Colours::black));
            lookAndFeel.addStyles("#button", jive::Styles{}.withShadow(jive::Shadow{}.setColour(juce::Colours::black)));

            juce::TextButton button;
            button.setComponentID("button");
            window.content.addAndMakeVisible(button);
            button.setBounds(50, 50, 20, 20);
            jive::PaintBeforeShowing paintBeforeShowing{ button };

            window.setVisible(true);
            expectEquals(window.content.getNumChildComponents(), 2);
        }
    }

private:
    struct PaintCounter : public juce::Component
    {
        void paint(juce::Graphics&) final
        {
            numPaints++;
        }

        int numPaints = 0;
    };

    struct Window : public juce::Component
    {
        Window()
        {
            addAndMakeVisible(content);
            setBounds(100, 100, 200, 200);
            content.setBounds(getLocalBounds());
            setOpaque(true);
            addToDesktop(0);
        }

        void paint(juce::Graphics& g) final
        {
            g.fillAll(juce::Colours::black);
        }

        PaintCounter content;
    };
};

static PaintBeforeShowingUnitTest paintBeforeShowingUnitTest;
#endif
