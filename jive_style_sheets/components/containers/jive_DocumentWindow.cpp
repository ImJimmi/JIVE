#include "jive_DocumentWindow.h"

#include <jive_style_sheets/graphics/jive_Fill.h>

namespace jive
{
    DocumentWindow::DocumentWindow()
        : juce::DocumentWindow{
            "",
            detail::getV4Colour(detail::V4Colour::windowBackground),
            DocumentWindow::allButtons,
        }
        , onCloseButtonPressed{
            [] {
                juce::JUCEApplication::getInstance()->systemRequestedQuit();
            },
        }
    {
    }

    void DocumentWindow::closeButtonPressed()
    {
        if (onCloseButtonPressed != nullptr)
            onCloseButtonPressed();
    }
} // namespace jive
