#pragma once

#include "jive_Shadow.h"
#include "jive_StyleSelector.h"
#include "jive_Styles.h"

#include <jive_core/interface/jive_InteractionState.h>

#include <juce_gui_basics/juce_gui_basics.h>

namespace jive
{
    class Caret;

    enum class Theme
    {
        juce,
        steel,
    };

    // Allows for custom styles and renderers to be specified for chosen
    // components using predicates.
    //
    // Use this class like any other LookAndFeel class - you can even use it
    // has the base class for your existing custom LookAndFeel. Use the various
    // `add_` methods to specify how different components should be styled.
    class LookAndFeel : public juce::LookAndFeel_V4
    {
    public:
        // Should draw the current state of the given component to the graphics
        // context provided, using the set of styles provided.
        using ComponentPainter = std::function<void(const juce::Component&,
                                                    const Styles&,
                                                    juce::Graphics&)>;

        using juce::LookAndFeel_V4::LookAndFeel_V4;

        // Automatically attaches this look-and-feel to the given component,
        // and detaches just before this object is destroyed.
        explicit LookAndFeel(juce::Component&);

        ~LookAndFeel() override;

        // Adds a painter that will be used to paint any component that matches
        // the given selector
        juce::Uuid addPainter(const StyleSelector& selector,
                              const ComponentPainter&);

        // Removes the specified painter
        void removePainter(const juce::Uuid& uuid);

        // Removes all the custom painters from this object.
        void clearPainters();

        // Adds a set of styles that will be used to style any component that
        // matches the given selector.
        juce::Uuid addStyles(const StyleSelector& selector,
                             const Styles&);

        // Returns a pointer to any styles with the given ID
        [[nodiscard]] Styles* findStyles(const juce::Uuid& uuid);
        [[nodiscard]] const Styles* findStyles(const juce::Uuid& uuid) const;

        // Removes the specified styles
        void removeStyles(const juce::Uuid& uuid);

        // Removes all the custom styles from this object.
        void clearStyles();

        // Updates the shadow component (if any) associated with the given
        // component using the styles provided.
        template <typename ComponentType>
        void updateShadow(std::remove_const_t<ComponentType>& component, const Styles& styles) const
        {
            static constexpr auto property = "jive::attached-shadow-component";

            if (!styles.shadow.hasValue())
            {
                component.getProperties().remove(property);
                component.getProperties().remove("jive::shadow-bounds");
                component.getProperties().remove("jive::shadow-path");
                return;
            }

            if (!component.getProperties().contains(property))
            {
                component
                    .getProperties()
                    .set(property,
                         new ReferenceCountedShadowComponent{
                             std::make_unique<ShadowComponent>(component),
                         });
            }

            auto& shadowComp = *dynamic_cast<ReferenceCountedShadowComponent*>(component
                                                                                   .getProperties()[property]
                                                                                   .getObject())
                                    ->shadow;
            shadowComp.setShadow(styles.find<Shadow>("shadow").value());
        }

        // Updates the shadow component (if any) associated with the given
        // component.
        template <typename ComponentType>
        void updateShadow(std::remove_const_t<ComponentType>& component) const
        {
            updateShadow<ComponentType>(component, findMostApplicableStyles(component));
        }

        // Removes all existing styles and painters and sets up predefined ones
        // according to the given Theme.
        void applyTheme(Theme theme);

        // Uses all the styles and predicates given to this object using
        // `addStyles()` to find the most applicable set.
        // This includes the "cascading" feature of CLAF, where all applicable
        // styles for this component will be merged such that the returned
        // object contains the final set of properties that should be used to
        // render the given component.
        [[nodiscard]] Styles findMostApplicableStyles(const juce::Component&) const;

        // Searches all the painters given to this object to find the one
        // (if any) that's most applicable to the given component.
        [[nodiscard]] std::optional<ComponentPainter> findMostApplicablePainter(const juce::Component&) const;

        // Can be overridden to change where the actual check-box for the given
        // toggle button should be positioned relative to the button's bounds.
        // Positioned center-left by default.
        virtual juce::Rectangle<int> getToggleButtonCheckboxBounds(const juce::ToggleButton&, const Styles&) const;

        // A generic component drawer - useful for drawing simple components
        // that just have a background, border, corner radius, etc.
        virtual void drawComponent(juce::Graphics&, const juce::Component&) const;

        // Draws a popup menu's background that's associated with the given
        // combo box (as opposed to a standalone menu, or one associated with a
        // different sort of component).
        virtual void drawComboBoxPopupBackground(juce::Graphics&,
                                                 const juce::ComboBox&,
                                                 const juce::PopupMenu::Options&,
                                                 const juce::Rectangle<float>&) const;

        // Draws an item in a popup menu that's associated with the given
        // combo box.
        virtual void drawComboBoxPopupItem(juce::Graphics&,
                                           const juce::PopupMenu::Item&,
                                           const juce::ComboBox&,
                                           const juce::PopupMenu::Options&,
                                           const InteractionState&,
                                           const juce::Rectangle<float>&) const;

        // =============================================================================================================
        // Overridden JUCE methods - see JUCE documentation for details.

        // juce::ScrollBar
        void drawScrollbarButton(juce::Graphics&, juce::ScrollBar&, int, int, int, bool, bool, bool) override;
        void drawScrollbar(juce::Graphics&, juce::ScrollBar&, int, int, int, int, bool, int, int, bool, bool) override;

        // juce::Button
        void drawButtonBackground(juce::Graphics&, juce::Button&, const juce::Colour&, bool, bool) override;
        void drawButtonText(juce::Graphics&, juce::TextButton&, bool, bool) override;
        void drawToggleButton(juce::Graphics&, juce::ToggleButton&, bool, bool) override;

        // juce::TextEditor
        void fillTextEditorBackground(juce::Graphics&, int, int, juce::TextEditor&) override;
        void drawTextEditorOutline(juce::Graphics&, int, int, juce::TextEditor&) override;
        juce::CaretComponent* createCaretComponent(juce::Component* keyFocusOwner) override;

        // juce::PopupMenu
        void drawPopupMenuBackgroundWithOptions(juce::Graphics&, int, int, const juce::PopupMenu::Options&) override;
        void drawPopupMenuItemWithOptions(juce::Graphics&, const juce::Rectangle<int>&, bool, const juce::PopupMenu::Item&, const juce::PopupMenu::Options&) override;
        void getIdealPopupMenuItemSizeWithOptions(const juce::String& text, bool isSeparator, int standardMenuItemHeight, int& idealWidth, int& idealHeight, const juce::PopupMenu::Options&) override;

        // juce::ComboBox
        void drawComboBox(juce::Graphics&, int, int, bool, int, int, int, int, juce::ComboBox&) override;
        juce::Label* createComboBoxTextBox(juce::ComboBox&) override;
        void positionComboBoxText(juce::ComboBox&, juce::Label&) override;

        // juce::Label
        void drawLabel(juce::Graphics&, juce::Label&) override;

        // juce::Slider
        void drawLinearSlider(juce::Graphics&, int, int, int, int, float, float, float, juce::Slider::SliderStyle, juce::Slider&) override;
        void drawRotarySlider(juce::Graphics&, int, int, int, int, float, float, float, juce::Slider&) override;

        // juce::ResizableWindow
        void fillResizableWindowBackground(juce::Graphics&, int, int, const juce::BorderSize<int>&, juce::ResizableWindow&) override;

        // juce::GroupComponent
        void drawGroupComponentOutline(juce::Graphics&, int, int, const juce::String& text, const juce::Justification&, juce::GroupComponent&) override;

        // juce::ProgressBar
        void drawProgressBar(juce::Graphics&, juce::ProgressBar&, int, int, double, const juce::String&) override;

        // =============================================================================================================

    private:
        template <class PaintFunction>
        struct Painter
        {
            juce::Uuid id;
            PaintFunction paint;
            StyleSelector selector;
        };

        struct Styler
        {
            juce::Uuid id;
            StyleSelector selector;
            Styles styles;
        };

        struct ComponentStylesCache
        {
            juce::Component::SafePointer<juce::Component> component;
            Styles styles;
        };

        template <typename Component, typename DefaultStyleDrawer>
        void draw(juce::Graphics& g,
                  const Component& component,
                  DefaultStyleDrawer drawDefaultStyle) const
        {
            const auto styles = findMostApplicableStyles(component);

            if (const auto painter = findMostApplicablePainter(component);
                painter.has_value())
            {
                (*painter)(component, styles, g);
                return;
            }

            drawDefaultStyle(styles);
            updateShadow<std::remove_const_t<Component>>(*const_cast<Component*>(&component));
        }

        void onVBlankUpdate();

        juce::Component::SafePointer<juce::Component> attachedComponent;

        std::vector<Painter<ComponentPainter>> painters;
        std::vector<Styler> stylers;
        mutable std::vector<ComponentStylesCache> stylesCache;

        juce::VBlankAttachment vBlank;
        mutable juce::Array<juce::Component::SafePointer<juce::Component>> transitioningComponents;
    };

    // A custom Caret component which is drawn by its look-and-feel, rather
    // than the hard-coded drawing of the base class.
    class Caret : public juce::CaretComponent
    {
    public:
        using juce::CaretComponent::CaretComponent;
        void paint(juce::Graphics&) override;
    };
} // namespace jive
