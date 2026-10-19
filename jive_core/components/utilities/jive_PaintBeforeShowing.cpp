#include "jive_PaintBeforeShowing.h"

namespace jive
{
    PaintBeforeShowing::PaintBeforeShowing(juce::Component& component)
        : juce::ComponentMovementWatcher{ &component }
    {
    }

    void PaintBeforeShowing::componentMovedOrResized(bool, bool)
    {
    }

    void PaintBeforeShowing::componentPeerChanged()
    {
    }

    // JUCE notifies visibility listeners before it shows the native window.
    void PaintBeforeShowing::componentVisibilityChanged()
    {
        auto* component = getComponent();

        if (component == nullptr || !component->isShowing())
            return;

        if (auto* peer = component->getPeer())
        {
            // Painting creates some components, such as shadows, which are
            // added behind components that have already been painted. Painting
            // off-screen first means they're all in the window's first frame.
            auto& window = peer->getComponent();
            juce::ignoreUnused(window.createComponentSnapshot(window.getLocalBounds()));
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

        beginTest("the first frame includes components that painting creates, such as shadows");
        {
            Window window;
            ShadowCreator creator;
            window.addAndMakeVisible(creator);
            creator.setBounds(50, 50, 20, 20);
            jive::PaintBeforeShowing paintBeforeShowing{ creator };

            window.setVisible(true);
            expectGreaterThan(creator.shadow.numPaints, 0);
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

    // Like a jive::ShadowComponent, adds a component behind itself the first
    // time it's painted.
    struct ShadowCreator : public juce::Component
    {
        void paint(juce::Graphics&) final
        {
            if (shadow.getParentComponent() != nullptr)
                return;

            getParentComponent()->addAndMakeVisible(shadow, 0);
            shadow.setBounds(getBounds().expanded(5));
        }

        PaintCounter shadow;
    };
};

static PaintBeforeShowingUnitTest paintBeforeShowingUnitTest;
#endif
