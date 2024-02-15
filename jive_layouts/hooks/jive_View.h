#pragma once

namespace jive
{
    class View : public juce::ReferenceCountedObject
    {
    public:
        using ReferenceCountedPointer = juce::ReferenceCountedObjectPtr<View>;

        View() = default;
        virtual ~View() override = default;

        [[nodiscard]] juce::ValueTree getState();

    protected:
        /** Called to instantiate the state of this View. */
        [[nodiscard]] virtual juce::ValueTree initialise() = 0;

        /** Called during interpretation to construct a custom component for a
            given element.

            This will be called not only for the ValueTree returned by
            initialise(), but also by all of its children.
            If you return nullptr, the interpreter will choose a sensible
            default.
        */
        [[nodiscard]] virtual std::unique_ptr<juce::Component> createComponent(const juce::ValueTree&)
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
        juce::ValueTree state;

        friend class Interpreter;
        JUCE_LEAK_DETECTOR(View)
    };

    /** Constructs the given ViewType as a ValueTree to be used either directly
        by jive::Interpreter, or to be added as a child to a ValueTree being
        interpreted.

        CAUTION - this function creates a circular dependency between the View
        object and the ValueTree that's returned as both hold a reference to
        each other! Using this with jive::Interpreter is safe because the
        circular dependency will be broken, however using this in any other way
        will require you to manually remove the "view-object" property of the
        returned tree before it goes out of scope.
    */
    template <typename ViewType, typename... Args>
    [[nodiscard]] auto makeView(Args&&... args)
    {
        juce::ReferenceCountedObjectPtr<ViewType> view = new ViewType{ std::forward<Args>(args)... };
        return view->getState()
            .setProperty("view-object", view.get(), nullptr);
    }
} // namespace jive
