#pragma once

#include <BinaryData.h>
#include <jive_layouts/jive_layouts.h>

#if JIVE_ENABLE_MELATONIN_INSPECTOR
    #include <melatonin_inspector/melatonin_inspector.h>
#endif

namespace jive_examples::text
{
    class Presenter
    {
    public:
        Presenter()
            : window{ interpreter.interpret(juce::File{ JIVE_EXAMPLE_VIEWS_DIR }.getChildFile("Main.xml")) }
        {
#if JIVE_ENABLE_MELATONIN_INSPECTOR
            inspector = std::make_unique<melatonin::Inspector>(*window->getComponent(), false);
            inspector->setVisible(true);

            interpreter.onExternalSourceLoaded = [this](const juce::File&) {
                inspector->setRoot(*window->getComponent());
            };
#endif
        }

    private:
        [[nodiscard]] static juce::Typeface::Ptr registerFont(const char* data, int size)
        {
            return juce::Typeface::createSystemTypefaceFor(data, static_cast<std::size_t>(size));
        }

        // Fonts can only be found by their family names while these are alive,
        // so they're declared before anything that gets interpreted.
        const std::array<juce::Typeface::Ptr, 3> fonts{
            registerFont(binary_data::rubik_ttf, binary_data::rubik_ttfSize),
            registerFont(binary_data::rubikmonoone_ttf, binary_data::rubikmonoone_ttfSize),
            registerFont(binary_data::sourcecodepro_ttf, binary_data::sourcecodepro_ttfSize),
        };

        jive::Interpreter interpreter;
        std::unique_ptr<jive::GuiItem> window;

#if JIVE_ENABLE_MELATONIN_INSPECTOR
        std::unique_ptr<melatonin::Inspector> inspector;
#endif
    };
} // namespace jive_examples::text
