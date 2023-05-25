#pragma once

namespace jive
{
    class DocumentWindow : public juce::DocumentWindow
    {
    public:
        DocumentWindow();

        void closeButtonPressed() override;

        std::function<void(void)> onCloseButtonPressed;

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DocumentWindow)
    };
} // namespace jive
