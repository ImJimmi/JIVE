#pragma once

#include <juce_core/juce_core.h>

namespace jive
{
    class FileObserver
    {
    public:
        explicit FileObserver(const juce::File& f)
            : file{ f }
            , lastModificationTime{ file.getLastModificationTime() }
        {
            observers.emplace_back(this);
            fileCheckCallback.startTimerHz(10);
        }

        JUCE_DECLARE_NON_COPYABLE(FileObserver)
        JUCE_DECLARE_NON_MOVEABLE(FileObserver)

        ~FileObserver()
        {
            observers.erase(std::remove(std::begin(observers),
                                        std::end(observers),
                                        this),
                            std::end(observers));
        }

        static void triggerAllTimerCallbacks()
        {
            for (auto* observer : observers)
                observer->checkFile();
        }

        const juce::File file;
        std::function<void()> onFileModified = nullptr;

    private:
        void checkFile()
        {
            if (file.getLastModificationTime() == lastModificationTime)
                return;

            lastModificationTime = file.getLastModificationTime();

            if (onFileModified != nullptr)
                onFileModified();
        }

        juce::TimedCallback fileCheckCallback{
            [this] {
                checkFile();
            },
        };

        juce::Time lastModificationTime;

        // NOLINTNEXTLINE(juce-plugin-global-state)
        static inline std::vector<FileObserver*> observers;
    };
} // namespace jive
