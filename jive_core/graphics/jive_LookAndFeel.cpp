#include <jive_core/jive_core.h>

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    bool LookAndFeel::areScrollbarButtonsVisible()
    {
        return false;
    }

    void LookAndFeel::drawScrollbarButton(juce::Graphics& /* g */,
                                          juce::ScrollBar& /* scrollbar */,
                                          int /* width */,
                                          int /* height */,
                                          int /* buttonDirection */,
                                          bool /* isScrollbarVertical */,
                                          bool /* isMouseOverButton */,
                                          bool /* isButtonDown */)
    {
    }

    void LookAndFeel::drawScrollbar(juce::Graphics& /* g */,
                                    juce::ScrollBar& /* scrollbar */,
                                    int /* x */,
                                    int /* y */,
                                    int /* width */,
                                    int /* height */,
                                    bool /* isScrollbarVertical */,
                                    int /* thumbStartPosition */,
                                    int /* thumbSize */,
                                    bool /* isMouseOver */,
                                    bool /* isMouseDown */)
    {
    }

    juce::ImageEffectFilter* LookAndFeel::getScrollbarEffect()
    {
        return nullptr;
    }

    int LookAndFeel::getMinimumScrollbarThumbSize(juce::ScrollBar&)
    {
        return 0;
    }

    int LookAndFeel::getDefaultScrollbarWidth()
    {
        return 0;
    }

    int LookAndFeel::getScrollbarButtonSize(juce::ScrollBar&)
    {
        return 0;
    }

    //==================================================================================================================
    void LookAndFeel::drawButtonBackground(juce::Graphics&,
                                           juce::Button&,
                                           const juce::Colour& /* backgroundColour */,
                                           bool /* shouldDrawButtonAsHighlighted */,
                                           bool /* shouldDrawButtonAsDown */)
    {
    }

    juce::Font LookAndFeel::getTextButtonFont(juce::TextButton&,
                                              int /* buttonHeight */)
    {
        return {};
    }

    int LookAndFeel::getTextButtonWidthToFitText(juce::TextButton&,
                                                 int /* buttonHeight */)
    {
        return 0;
    }

    void LookAndFeel::drawButtonText(juce::Graphics&,
                                     juce::TextButton&,
                                     bool /* shouldDrawButtonAsHighlighted */,
                                     bool /* shouldDrawButtonAsDown */)
    {
    }

    void LookAndFeel::drawToggleButton(juce::Graphics&,
                                       juce::ToggleButton&,
                                       bool /* shouldDrawButtonAsHighlighted */,
                                       bool /* shouldDrawButtonAsDown */)
    {
    }

    void LookAndFeel::changeToggleButtonWidthToFitText(juce::ToggleButton&)
    {
    }

    void LookAndFeel::drawTickBox(juce::Graphics&,
                                  juce::Component&,
                                  float /* x */,
                                  float /* y */,
                                  float /* w */,
                                  float /* h */,
                                  bool /* ticked */,
                                  bool /* isEnabled */,
                                  bool /* shouldDrawButtonAsHighlighted */,
                                  bool /* shouldDrawButtonAsDown */)
    {
    }

    void LookAndFeel::drawDrawableButton(juce::Graphics&,
                                         juce::DrawableButton&,
                                         bool /* shouldDrawButtonAsHighlighted */,
                                         bool /* shouldDrawButtonAsDown */)
    {
    }

    //==================================================================================================================
    void LookAndFeel::fillTextEditorBackground(juce::Graphics&,
                                               int /* width */,
                                               int /* height */,
                                               juce::TextEditor&)
    {
    }

    void LookAndFeel::drawTextEditorOutline(juce::Graphics&,
                                            int /* width */,
                                            int /* height */,
                                            juce::TextEditor&)
    {
    }

    juce::CaretComponent* LookAndFeel::createCaretComponent(juce::Component* /* keyFocusOwner */)
    {
        return nullptr;
    }

    //==================================================================================================================
    void LookAndFeel::drawComboBox(juce::Graphics&,
                                   int /* width */,
                                   int /* height */,
                                   bool /* isButtonDown */,
                                   int /* buttonX */,
                                   int /* buttonY */,
                                   int /* buttonW */,
                                   int /* buttonH */,
                                   juce::ComboBox&)
    {
    }

    juce::Font LookAndFeel::getComboBoxFont(juce::ComboBox&)
    {
        return {};
    }

    void LookAndFeel::positionComboBoxText(juce::ComboBox&,
                                           juce::Label& /* labelToPosition */)
    {
    }

    juce::PopupMenu::Options LookAndFeel::getOptionsForComboBoxPopupMenu(juce::ComboBox&,
                                                                         juce::Label&)
    {
        return {};
    }

    void LookAndFeel::drawComboBoxTextWhenNothingSelected(juce::Graphics&,
                                                          juce::ComboBox&,
                                                          juce::Label&)
    {
    }

    //==================================================================================================================
    void LookAndFeel::drawCornerResizer(juce::Graphics&,
                                        int /* w */,
                                        int /* h */,
                                        bool /* isMouseOver */,
                                        bool /* isMouseDragging */)
    {
    }

    void LookAndFeel::drawResizableFrame(juce::Graphics&,
                                         int /* w */,
                                         int /* h */,
                                         const juce::BorderSize<int>&)
    {
    }

    void LookAndFeel::fillResizableWindowBackground(juce::Graphics&,
                                                    int /* w */,
                                                    int /* h */,
                                                    const juce::BorderSize<int>&,
                                                    juce::ResizableWindow&)
    {
    }

    void LookAndFeel::drawResizableWindowBorder(juce::Graphics&,
                                                int /* w */,
                                                int /* h */,
                                                const juce::BorderSize<int>& /* border */,
                                                juce::ResizableWindow&)
    {
    }
} // namespace jive
