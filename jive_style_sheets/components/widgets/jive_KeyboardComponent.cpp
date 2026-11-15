#include "jive_KeyboardComponent.h"

#if JUCE_MODULE_AVAILABLE_juce_audio_utils
    #include <jive_style_sheets/graphics/default-styles/jive_DefaultKeyboardStyles.h>
    #include <jive_style_sheets/graphics/jive_LookAndFeel.h>

namespace jive
{
    KeyboardComponent::Key::Key(int keyNote)
        : note{ keyNote }
    {
        setAccessible(false);
        setInterceptsMouseClicks(false, false);
        getProperties().set(ids::type(), isBlack() ? "BlackKey" : "WhiteKey");
    }

    int KeyboardComponent::Key::getNote() const noexcept
    {
        return note;
    }

    bool KeyboardComponent::Key::isBlack() const noexcept
    {
        return juce::MidiMessage::isMidiNoteBlack(note);
    }

    void KeyboardComponent::Key::setInteraction(bool isOver, bool isDown)
    {
        getProperties().set(ids::highlighted(), isOver);
        getProperties().set(ids::pressed(), isDown);
    }

    KeyboardComponent::KeyboardComponent()
        : KeyboardStateHolder{}
        , juce::MidiKeyboardComponent{ ownedState, horizontalKeyboard }
        , state{ ownedState }
    {
        initialise();
    }

    KeyboardComponent::KeyboardComponent(juce::MidiKeyboardState& sharedState)
        : juce::MidiKeyboardComponent{ sharedState, horizontalKeyboard }
        , state{ sharedState }
    {
        initialise();
    }

    void KeyboardComponent::initialise()
    {
        setScrollButtonsVisible(false);
        setOctaveForMiddleC(4);
        setBlackNoteLengthProportion(0.6f);
        setColour(whiteNoteColourId, juce::Colours::transparentBlack);
        setColour(shadowColourId, juce::Colours::transparentBlack);
        getProperties().set(ids::type(), "Keyboard");
        positionKeys();
    }

    juce::MidiKeyboardState& KeyboardComponent::getState() noexcept
    {
        return state;
    }

    void KeyboardComponent::setNoteRange(int lowestNote, int highestNote)
    {
        setAvailableRange(lowestNote, highestNote);
    }

    KeyboardComponent::Key* KeyboardComponent::findKey(int note) const
    {
        return keys[note - getRangeStart()];
    }

    void KeyboardComponent::paint(juce::Graphics& g)
    {
        auto* lookAndFeel = dynamic_cast<LookAndFeel*>(&getLookAndFeel());

        if (lookAndFeel == nullptr)
        {
            juce::MidiKeyboardComponent::paint(g);
            return;
        }

        if (const auto painter = lookAndFeel->findMostApplicablePainter(*this);
            painter.has_value())
        {
            (*painter)(*this, lookAndFeel->findMostApplicableStyles(*this), g);
            lookAndFeel->updateShadow<KeyboardComponent>(*this);
            return;
        }

        lookAndFeel->drawComponent(g, *this);
        juce::MidiKeyboardComponent::paint(g);
    }

    void KeyboardComponent::resized()
    {
        fitKeysToWidth();
        juce::MidiKeyboardComponent::resized();
        positionKeys();
    }

    void KeyboardComponent::fitKeysToWidth()
    {
        auto numWhiteKeys = 0;

        for (auto note = getRangeStart(); note <= getRangeEnd(); note++)
            numWhiteKeys += juce::MidiMessage::isMidiNoteBlack(note) ? 0 : 1;

        if (getWidth() > 0 && numWhiteKeys > 0)
            setKeyWidth(static_cast<float>(getWidth()) / static_cast<float>(numWhiteKeys));
    }

    // Changing the range resizes the keyboard before it returns, so the keys
    // are brought up to date while they're positioned.
    void KeyboardComponent::createKeys()
    {
        const auto numKeys = getRangeEnd() - getRangeStart() + 1;

        if (keys.size() == numKeys && keys.getFirst()->getNote() == getRangeStart())
            return;

        keys.clear();

        for (auto note = getRangeStart(); note <= getRangeEnd(); note++)
            addAndMakeVisible(keys.add(std::make_unique<Key>(note)));
    }

    // Keys repaint the part of the keyboard they cover, such as while their
    // styles transition.
    void KeyboardComponent::positionKeys()
    {
        createKeys();

        for (auto* key : keys)
            key->setBounds(getRectangleForKey(key->getNote()).getSmallestIntegerContainer());
    }

    void KeyboardComponent::paintKey(juce::Graphics& g, int note, juce::Rectangle<float> area, bool isDown, bool isOver)
    {
        auto* key = findKey(note);

        if (key == nullptr)
            return;

        key->setInteraction(isOver, isDown);

        auto* lookAndFeel = dynamic_cast<LookAndFeel*>(&getLookAndFeel());
        const auto styles = lookAndFeel == nullptr
                              ? Styles{}
                              : lookAndFeel->findMostApplicableStyles(*key);

        if (lookAndFeel != nullptr)
        {
            if (const auto painter = lookAndFeel->findMostApplicablePainter(*key);
                painter.has_value())
            {
                const juce::Graphics::ScopedSaveState savedState{ g };
                g.setOrigin(key->getPosition());
                (*painter)(*key, styles, g);
                return;
            }
        }

        const auto label = !key->isBlack() && note % 12 == 0
                             ? getWhiteNoteText(note)
                             : juce::String{};
        default_styles::paintKeyboardKey(g, *key, area, label, isDown, isOver, styles);
    }

    void KeyboardComponent::drawWhiteNote(int note,
                                          juce::Graphics& g,
                                          juce::Rectangle<float> area,
                                          bool isDown,
                                          bool isOver,
                                          juce::Colour,
                                          juce::Colour)
    {
        paintKey(g, note, area, isDown, isOver);
    }

    void KeyboardComponent::drawBlackNote(int note,
                                          juce::Graphics& g,
                                          juce::Rectangle<float> area,
                                          bool isDown,
                                          bool isOver,
                                          juce::Colour)
    {
        paintKey(g, note, area, isDown, isOver);
    }
} // namespace jive
#endif

#if JIVE_UNIT_TESTS && JUCE_MODULE_AVAILABLE_juce_audio_utils
class KeyboardComponentTest : public juce::UnitTest
{
public:
    KeyboardComponentTest()
        : juce::UnitTest{ "jive::KeyboardComponent", "jive" }
    {
    }

    void runTest() final
    {
        testKeyStandIns();
        testKeySelectors();
        testPressedWhileHovered();
    }

    void testPressedWhileHovered()
    {
        beginTest("a key pressed under the mouse is active");

        juce::Component parent;
        jive::LookAndFeel lookAndFeel{ parent };
        lookAndFeel.applyTheme(jive::Theme::juce);
        lookAndFeel.addStyles("Keyboard > WhiteKey:hover", jive::Styles{}.withBackground(juce::Colours::red));
        lookAndFeel.addStyles("Keyboard > WhiteKey:active", jive::Styles{}.withBackground(juce::Colours::lime));

        jive::KeyboardComponent keyboard;
        parent.addAndMakeVisible(keyboard);
        keyboard.setNoteRange(60, 71);
        keyboard.setBounds(0, 0, 140, 100);

        auto& key = *keyboard.findKey(60);
        key.setInteraction(true, true);
        const auto styles = lookAndFeel.findMostApplicableStyles(key);
        expect(styles.find<jive::Fill>("background") == jive::Fill{ juce::Colours::lime });
    }

private:
    void testKeyStandIns()
    {
        beginTest("key stand-ins");

        jive::KeyboardComponent keyboard;
        keyboard.setNoteRange(60, 72);
        expect(keyboard.findKey(72) != nullptr);
        expect(keyboard.findKey(73) == nullptr);
        expectEquals(keyboard.findKey(60)->getProperties()["jive::type"].toString(), juce::String{ "WhiteKey" });
        expectEquals(keyboard.findKey(61)->getProperties()["jive::type"].toString(), juce::String{ "BlackKey" });
        expect(keyboard.findKey(59) == nullptr);

        keyboard.setBounds(0, 0, 160, 100);
        expect(keyboard.findKey(60)->isVisible());
        expect(keyboard.findKey(60)->getBounds() == keyboard.getRectangleForKey(60).getSmallestIntegerContainer());

        keyboard.setNoteRange(48, 50);
        expect(keyboard.findKey(50) != nullptr);
        expect(keyboard.findKey(60) == nullptr);
    }

    void testKeySelectors()
    {
        beginTest("Keyboard > WhiteKey and Keyboard > BlackKey");

        juce::Component parent;
        jive::LookAndFeel lookAndFeel{ parent };
        lookAndFeel.applyTheme(jive::Theme::juce);
        lookAndFeel.addStyles("Keyboard > WhiteKey", jive::Styles{}.withBackground(juce::Colours::red).withBorderWidth(juce::BorderSize<float>{ 0.0f }));
        lookAndFeel.addStyles("Keyboard > WhiteKey:active", jive::Styles{}.withBackground(juce::Colours::lime));
        lookAndFeel.addStyles("Keyboard > BlackKey", jive::Styles{}.withBackground(juce::Colours::blue));

        jive::KeyboardComponent keyboard;
        parent.addAndMakeVisible(keyboard);
        keyboard.setNoteRange(60, 71);
        keyboard.setBounds(0, 0, 140, 100);
        keyboard.getState().noteOn(1, 64, 1.0f);

        const auto snapshot = keyboard.createComponentSnapshot(keyboard.getLocalBounds());
        expect(snapshot.getPixelAt(10, 90) == juce::Colours::red);
        expect(snapshot.getPixelAt(20, 20) == juce::Colours::blue);
        expect(snapshot.getPixelAt(50, 90) == juce::Colours::lime);
    }
};

static KeyboardComponentTest keyboardComponentTest;
#endif
