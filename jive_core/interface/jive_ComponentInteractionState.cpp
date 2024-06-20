#include "jive_ComponentInteractionState.h"

namespace jive
{
    ComponentInteractionState::ComponentInteractionState(const juce::Component& comp, juce::ValueTree tree)
        : component{ comp }
        , mouse{ tree, "mouse" }
        , keyboard{ tree, "keyboard" }
    {
        const_cast<juce::Component&>(component).addMouseListener(this, true);
        juce::Desktop::getInstance().addFocusChangeListener(this);

        mouse = getCurrentMouseState();
        keyboard = getCurrentKeyboardState();
    }

    ComponentInteractionState::~ComponentInteractionState()
    {
        juce::Desktop::getInstance().removeFocusChangeListener(this);
        const_cast<juce::Component&>(component).removeMouseListener(this);
    }

    void ComponentInteractionState::mouseEnter(const juce::MouseEvent&)
    {
        mouse = getCurrentMouseState();
    }

    void ComponentInteractionState::mouseExit(const juce::MouseEvent&)
    {
        mouse = getCurrentMouseState();
    }

    void ComponentInteractionState::mouseDown(const juce::MouseEvent&)
    {
        mouse = getCurrentMouseState();
    }

    void ComponentInteractionState::mouseUp(const juce::MouseEvent&)
    {
        mouse = getCurrentMouseState();
    }

    void ComponentInteractionState::globalFocusChanged(juce::Component*)
    {
        keyboard = getCurrentKeyboardState();
    }

    ComponentInteractionState::Mouse ComponentInteractionState::getCurrentMouseState() const
    {
        if (component.isMouseOverOrDragging(true))
        {
            if (component.isMouseButtonDownAnywhere())
                return Mouse::active;

            return Mouse::hover;
        }

        return Mouse::dissociate;
    }

    ComponentInteractionState::Keyboard ComponentInteractionState::getCurrentKeyboardState() const
    {
        if (component.hasKeyboardFocus(false))
            return Keyboard::focus;

        return Keyboard::dissociate;
    }
} // namespace jive

namespace juce
{
    const std::unordered_map<jive::ComponentInteractionState::Mouse, juce::String> VariantConverter<jive::ComponentInteractionState::Mouse>::map{
        { jive::ComponentInteractionState::Mouse::dissociate, "dissociate" },
        { jive::ComponentInteractionState::Mouse::hover, "hover" },
        { jive::ComponentInteractionState::Mouse::active, "active" },
    };

    var VariantConverter<jive::ComponentInteractionState::Mouse>::toVar(const jive::ComponentInteractionState::Mouse& mouse)
    {
        return map.find(mouse)->second;
    }

    jive::ComponentInteractionState::Mouse VariantConverter<jive::ComponentInteractionState::Mouse>::fromVar(const var& value)
    {
        if (value.isVoid())
            return jive::ComponentInteractionState::Mouse::dissociate;

        return std::find_if(std::begin(map),
                            std::end(map),
                            [value](auto&& pair) {
                                return value.toString() == pair.second;
                            })
            ->first;
    }

    var VariantConverter<jive::ComponentInteractionState::Keyboard>::toVar(const jive::ComponentInteractionState::Keyboard& mouse)
    {
        return map.find(mouse)->second;
    }

    jive::ComponentInteractionState::Keyboard VariantConverter<jive::ComponentInteractionState::Keyboard>::fromVar(const var& value)
    {
        if (value.isVoid())
            return jive::ComponentInteractionState::Keyboard::dissociate;

        return std::find_if(std::begin(map),
                            std::end(map),
                            [value](auto&& pair) {
                                return value.toString() == pair.second;
                            })
            ->first;
    }

    const std::unordered_map<jive::ComponentInteractionState::Keyboard, juce::String> VariantConverter<jive::ComponentInteractionState::Keyboard>::map{
        { jive::ComponentInteractionState::Keyboard::dissociate, "dissociate" },
        { jive::ComponentInteractionState::Keyboard::focus, "focus" },
    };
} // namespace juce
