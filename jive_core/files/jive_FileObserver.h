#pragma once

#include <juce_core/juce_core.h>

namespace jive
{
    class FileObserver : private juce::Timer
    {
    public:
        explicit FileObserver(const juce::File& f)
            : file{ f }
            , lastModificationTime{ file.getLastModificationTime() }
        {
            startTimerHz(10);
        }

        std::function<void()> onFileModified = nullptr;

    private:
        void timerCallback() final
        {
            if (file.getLastModificationTime() != lastModificationTime)
            {
                lastModificationTime = file.getLastModificationTime();

                if (onFileModified)
                    onFileModified();
            }
        }

        juce::File file;
        juce::Time lastModificationTime;
    };
} // namespace jive
