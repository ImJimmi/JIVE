#pragma once

//======================================================================================================================
namespace jive
{
    //==================================================================================================================
    class LookAndFeel : public juce::LookAndFeel_V4
    {
    public:
        //==============================================================================================================
        bool areScrollbarButtonsVisible() override;
        void drawScrollbarButton(juce::Graphics& g,
                                 juce::ScrollBar& scrollbar,
                                 int width,
                                 int height,
                                 int buttonDirection,
                                 bool isScrollbarVertical,
                                 bool isMouseOverButton,
                                 bool isButtonDown) override;
        void drawScrollbar(juce::Graphics& g,
                           juce::ScrollBar& scrollbar,
                           int x,
                           int y,
                           int width,
                           int height,
                           bool isScrollbarVertical,
                           int thumbStartPosition,
                           int thumbSize,
                           bool isMouseOver,
                           bool isMouseDown) override;
        juce::ImageEffectFilter* getScrollbarEffect() override;
        int getMinimumScrollbarThumbSize(juce::ScrollBar&) override;
        int getDefaultScrollbarWidth() override;
        int getScrollbarButtonSize(juce::ScrollBar&) override;

        //==============================================================================================================
        void drawButtonBackground(juce::Graphics&,
                                  juce::Button&,
                                  const juce::Colour& backgroundColour,
                                  bool shouldDrawButtonAsHighlighted,
                                  bool shouldDrawButtonAsDown) override;
        juce::Font getTextButtonFont(juce::TextButton&,
                                     int buttonHeight) override;
        int getTextButtonWidthToFitText(juce::TextButton&,
                                        int buttonHeight) override;
        void drawButtonText(juce::Graphics&,
                            juce::TextButton&,
                            bool shouldDrawButtonAsHighlighted,
                            bool shouldDrawButtonAsDown) override;
        void drawToggleButton(juce::Graphics&,
                              juce::ToggleButton&,
                              bool shouldDrawButtonAsHighlighted,
                              bool shouldDrawButtonAsDown) override;
        void changeToggleButtonWidthToFitText(juce::ToggleButton&) override;
        void drawTickBox(juce::Graphics&,
                         juce::Component&,
                         float x,
                         float y,
                         float w,
                         float h,
                         bool ticked,
                         bool isEnabled,
                         bool shouldDrawButtonAsHighlighted,
                         bool shouldDrawButtonAsDown) override;
        void drawDrawableButton(juce::Graphics&,
                                juce::DrawableButton&,
                                bool shouldDrawButtonAsHighlighted,
                                bool shouldDrawButtonAsDown) override;

        //==============================================================================================================
        void fillTextEditorBackground(juce::Graphics&,
                                      int width,
                                      int height,
                                      juce::TextEditor&) override;
        void drawTextEditorOutline(juce::Graphics&,
                                   int width,
                                   int height,
                                   juce::TextEditor&) override;
        juce::CaretComponent* createCaretComponent(juce::Component* keyFocusOwner) override;

        //==============================================================================================================
        void drawComboBox(juce::Graphics&,
                          int width,
                          int height,
                          bool isButtonDown,
                          int buttonX,
                          int buttonY,
                          int buttonW,
                          int buttonH,
                          juce::ComboBox&) override;
        juce::Font getComboBoxFont(juce::ComboBox&) override;
        void positionComboBoxText(juce::ComboBox&,
                                  juce::Label& labelToPosition) override;
        juce::PopupMenu::Options getOptionsForComboBoxPopupMenu(juce::ComboBox&,
                                                                juce::Label&) override;
        void drawComboBoxTextWhenNothingSelected(juce::Graphics&,
                                                 juce::ComboBox&,
                                                 juce::Label&) override;

        //==============================================================================================================
        void drawCornerResizer(juce::Graphics&,
                               int w,
                               int h,
                               bool isMouseOver,
                               bool isMouseDragging) override;
        void drawResizableFrame(juce::Graphics&,
                                int w,
                                int h,
                                const juce::BorderSize<int>&) override;
        void drawResizableWindowBorder(juce::Graphics&,
                                       int w,
                                       int h,
                                       const juce::BorderSize<int>& border,
                                       juce::ResizableWindow&) override;

    private:
        //==============================================================================================================
    };
} // namespace jive
