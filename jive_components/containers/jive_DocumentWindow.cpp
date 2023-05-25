#include <jive_layouts/jive_layouts.h>

namespace jive
{
    DocumentWindow::DocumentWindow()
        : juce::DocumentWindow{ "",
                                juce::LookAndFeel::getDefaultLookAndFeel()
                                    .findColour(ResizableWindow::backgroundColourId),
                                DocumentWindow::allButtons }
        , onCloseButtonPressed{ []() {
            juce::JUCEApplication::getInstance()->systemRequestedQuit();
        } }
    {
        setVisible(true);
    }

    void DocumentWindow::closeButtonPressed()
    {
        if (onCloseButtonPressed != nullptr)
            onCloseButtonPressed();
    }
} // namespace jive
