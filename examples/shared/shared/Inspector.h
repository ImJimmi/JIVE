#pragma once

#include <jive_layouts/jive_layouts.h>

#if JIVE_ENABLE_MELATONIN_INSPECTOR
    #include <melatonin_inspector/melatonin_inspector.h>
#endif

namespace jive_examples
{
    // Shows the melatonin inspector when JIVE_ENABLE_MELATONIN_INSPECTOR is
    // on, and does nothing otherwise.
    class Inspector
    {
    public:
        void inspect(juce::Component& root)
        {
#if JIVE_ENABLE_MELATONIN_INSPECTOR
            if (inspector != nullptr)
            {
                inspector->setRoot(root);
                return;
            }

            inspector = std::make_unique<melatonin::Inspector>(root, false);
            inspector->setVisible(true);
#else
            juce::ignoreUnused(root);
#endif
        }

        void close()
        {
#if JIVE_ENABLE_MELATONIN_INSPECTOR
            inspector = nullptr;
#endif
        }

    private:
#if JIVE_ENABLE_MELATONIN_INSPECTOR
        std::unique_ptr<melatonin::Inspector> inspector;
#endif
    };
} // namespace jive_examples
