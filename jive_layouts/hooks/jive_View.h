#pragma once

#include <jive_core/jive_core.h>

namespace jive
{
    class GuiItem;

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

        friend class GuiItem;
        friend class Interpreter;
        friend class ViewBuilder;
        JUCE_LEAK_DETECTOR(View)
    };

    class ViewBuilder
    {
    public:
        template <typename ViewType, typename... Args>
        [[nodiscard]] static auto makeView(Args&&... args)
        {
            const auto buildView = makeViewBuilder<ViewType, Args...>(std::forward<Args>(args)...);
            const auto wrappedState = buildView({ juce::var{}, nullptr, 0 });
            auto state = dynamic_cast<ReferenceCountedValueTreeWrapper*>(wrappedState.getObject())->state;
            state.setProperty("make-view", buildView, nullptr);

            return state;
        }

    private:
        template <typename ViewType, typename... Args>
        [[nodiscard]] static juce::var::NativeFunction makeViewBuilder(Args&&... args)
        {
            return [args = std::make_tuple(std::forward<Args>(args)...)](const auto&) {
                static constexpr auto createRawView = [](auto&&... theArgs) {
                    return new ViewType{ std::forward<Args>(theArgs)... };
                };
                const View::ReferenceCountedPointer view{ std::apply(createRawView, std::move(args)) };
                auto state = view->initialise();
                state.setProperty("view-object", view.get(), nullptr);

                return juce::var{
                    new ReferenceCountedValueTreeWrapper{ state },
                };
            };
        }
    };

    /** Constructs the given ViewType as a ValueTree to be used either directly
        by jive::Interpreter, or to be added as a child to a ValueTree being
        interpreted.
    */
    template <typename ViewType, typename... Args>
    [[nodiscard]] auto makeView(Args&&... args)
    {
        return ViewBuilder::makeView<ViewType, Args...>(std::forward<Args>(args)...);
    }
} // namespace jive
