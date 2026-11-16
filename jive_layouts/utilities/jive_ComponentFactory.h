#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace std
{
    template <>
    class hash<juce::Identifier>
    {
    public:
        std::size_t operator()(const juce::Identifier& id) const
        {
            return id.toString().hash();
        }
    };
} // namespace std

namespace jive
{
    class ComponentFactory
    {
    public:
        using ComponentCreator = std::function<std::shared_ptr<juce::Component>(void)>;

        ComponentFactory();

        std::shared_ptr<juce::Component> create(juce::Identifier name) const;
        void set(juce::Identifier name, ComponentCreator creator);

    private:
        std::unordered_map<juce::Identifier, ComponentCreator> creators;
    };
} // namespace jive
