#pragma once

#include "jive_StyleIdentifier.h"

#include <bitset>

namespace jive
{
    struct StyleSelectors
    {
    public:
        explicit StyleSelectors(const juce::ValueTree& sourceState)
            : state{ sourceState }
            , id{ state, "id" }
            , classes{ state, "class" }
            , enabled{ state, "enabled" }
            , mouse{ state, "mouse" }
            , keyboard{ state, "keyboard" }
            , toggled{ state, "toggled" }
        {
            const auto informListeners = [this]() {
                if (onChange != nullptr)
                    onChange();
            };
            id.onValueChange = informListeners;
            enabled.onValueChange = informListeners;
            mouse.onValueChange = informListeners;
            keyboard.onValueChange = informListeners;
            classes.onValueChange = informListeners;
            toggled.onValueChange = informListeners;
        }

        template <typename PropertyType>
        [[nodiscard]] const PropertyType* findStyle(const std::unordered_map<StyleIdentifier, PropertyType>& styles) const
        {
            if (std::empty(styles))
                return nullptr;

            auto entries = collectIterators(styles);

            entries.erase(std::remove_if(std::begin(entries),
                                         std::end(entries),
                                         [this](auto&& entry) {
                                             return entry->first.id.isNotEmpty() && entry->first.id != id.toString();
                                         }),
                          std::end(entries));
            entries.erase(std::remove_if(std::begin(entries),
                                         std::end(entries),
                                         [this](auto&& entry) {
                                             return entry->first.className.isNotEmpty() && !classes.get().contains(entry->first.className);
                                         }),
                          std::end(entries));
            entries.erase(std::remove_if(std::begin(entries),
                                         std::end(entries),
                                         [this](auto&& entry) {
                                             return entry->first.type.isNotEmpty() && entry->first.type != state.getType().toString();
                                         }),
                          std::end(entries));
            entries.erase(std::remove_if(std::begin(entries),
                                         std::end(entries),
                                         [this](auto&& entry) {
                                             return !entry->first.enabled && enabled.getOr(true);
                                         }),
                          std::end(entries));
            entries.erase(std::remove_if(std::begin(entries),
                                         std::end(entries),
                                         [this](auto&& entry) {
                                             return entry->first.keyboard != ComponentInteractionState::Keyboard::dissociate
                                                 && entry->first.keyboard != keyboard.get();
                                         }),
                          std::end(entries));
            entries.erase(std::remove_if(std::begin(entries),
                                         std::end(entries),
                                         [this](auto&& entry) {
                                             return entry->first.mouse != ComponentInteractionState::Mouse::dissociate
                                                 && entry->first.mouse != mouse.get();
                                         }),
                          std::end(entries));
            entries.erase(std::remove_if(std::begin(entries),
                                         std::end(entries),
                                         [this](auto&& entry) {
                                             return entry->first.toggled && !toggled.get();
                                         }),
                          std::end(entries));

            if (std::size(entries) == 0)
                return nullptr;

            std::sort(std::begin(entries),
                      std::end(entries),
                      [this](auto&& first, auto&& second) {
                          return measureApplicability(first->first) < measureApplicability(second->first);
                      });
            return &entries.back()->second;
        }

        const juce::ValueTree state;
        const Property<juce::String> id;
        const Property<juce::StringArray> classes;
        const Property<bool> enabled;
        const Property<ComponentInteractionState::Mouse> mouse;
        const Property<ComponentInteractionState::Keyboard> keyboard;
        const Property<bool> toggled;
        std::function<void()> onChange = nullptr;

    private:
        template <typename Container>
        [[nodiscard]] static auto collectIterators(const Container& container)
        {
            std::vector<decltype(&(*std::begin(container)))> result;
            result.reserve(std::size(container));

            auto current = std::begin(container);

            for (std::size_t i = 0; i < std::size(container); i++)
                result.push_back(&(*current++));

            return result;
        }

        [[nodiscard]] unsigned long measureApplicability(const StyleIdentifier& styleID) const
        {
            std::size_t bit = 0;

            return std::bitset<7>{}
                .set(bit++, styleID.toggled == toggled.get())
                .set(bit++, styleID.mouse == mouse.get())
                .set(bit++, styleID.keyboard == keyboard.get())
                .set(bit++, !styleID.enabled && !enabled.getOr(true))
                .set(bit++, styleID.type == state.getType().toString())
                .set(bit++, classes.get().contains(styleID.className))
                .set(bit++, styleID.id == id.toString())
                .to_ulong();
        }
    };
} // namespace jive
