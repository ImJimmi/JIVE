#pragma once

#include "Fonts.h"

#include <jive_layouts/jive_layouts.h>

namespace jive_examples
{
    class BasePresenter
    {
    public:
        BasePresenter() = default;
        virtual ~BasePresenter() = default;

        JUCE_DECLARE_NON_COPYABLE(BasePresenter)
        JUCE_DECLARE_NON_MOVEABLE(BasePresenter)

    protected:
        void onToggled(std::vector<std::unique_ptr<jive::Property<bool>>>& bindings,
                       const juce::String& buttonID,
                       std::function<void(bool)> callback)
        {
            onToggled(bindings, jive::findElementWithID(window->state, buttonID), std::move(callback));
        }

        static void onToggled(std::vector<std::unique_ptr<jive::Property<bool>>>& bindings,
                              const juce::ValueTree& button,
                              std::function<void(bool)> callback)
        {
            if (!button.isValid())
                return;

            auto& toggled = *bindings.emplace_back(std::make_unique<jive::Property<bool>>(button, jive::ids::toggled()));
            toggled.onValueChange = [&toggled, callback] {
                callback(toggled.get());
            };

            callback(toggled.get());
        }

        void onValueChange(std::vector<std::unique_ptr<jive::Property<int>>>& bindings,
                           const juce::String& controlID,
                           const juce::Identifier& property,
                           std::function<void(int)> callback)
        {
            const auto control = jive::findElementWithID(window->state, controlID);

            if (!control.isValid())
                return;

            auto& value = *bindings.emplace_back(std::make_unique<jive::Property<int>>(control, property));
            value.onValueChange = [&value, callback] {
                callback(value.get());
            };

            callback(value.get());
        }

        void setProperty(const juce::String& id, const juce::Identifier& property, const juce::var& value)
        {
            jive::findElementWithID(window->state, id).setProperty(property, value, nullptr);
        }

        [[nodiscard]] int indexOfOption(const juce::String& comboBoxID, const juce::var& text) const
        {
            const auto comboBox = jive::findElementWithID(window->state, comboBoxID);

            for (auto i = 0; i < comboBox.getNumChildren(); i++)
            {
                if (comboBox.getChild(i)[jive::ids::text()] == text)
                    return i;
            }

            return -1;
        }

    private:
        // Declared before anything that gets interpreted.
        const std::array<juce::Typeface::Ptr, 3> fonts = registerFonts();

    protected:
        jive::Interpreter interpreter;
        std::unique_ptr<jive::GuiItem> window;
    };
} // namespace jive_examples
