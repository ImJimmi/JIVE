#pragma once

namespace jive
{
    class View : public juce::ReferenceCountedObject
    {
    public:
        using ReferenceCountedPointer = juce::ReferenceCountedObjectPtr<View>;

        virtual ~View() override = default;

        [[nodiscard]] juce::ValueTree toValueTree();
        [[nodiscard]] operator juce::ValueTree();

    protected:
        [[nodiscard]] virtual juce::ValueTree initialise() = 0;

        /** Called during interpretation to construct a custom component for a
            given element.
        */
        virtual std::unique_ptr<juce::Component> createComponent(const juce::ValueTree&)
        {
            return nullptr;
        }

        /** Called after the view is interpreted and wrapped with a GuiItem.

            The GuiItem passed to this callback is the item that now owns this
            view. You can do any additional setup behaviour here such as
            attaching listeners to components, or pretty much anything that
            couldn't be done declaratively through the ValueTree.
        */
        virtual void setup([[maybe_unused]] jive::GuiItem& item) {}

    private:
        friend class Interpreter;

        JUCE_LEAK_DETECTOR(View)
    };

    template <typename ViewType, typename... Args>
    [[nodiscard]] typename ViewType::ReferenceCountedPointer makeView(Args&&... args)
    {
        return new ViewType{ std::forward<Args>(args)... };
    }
} // namespace jive
