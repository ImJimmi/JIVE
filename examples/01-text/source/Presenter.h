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
            : window{ interpreter.interpret(jive::SourceFile{ "public/views/Main.xml" }) }
        {
            interpreter.onExternalSourceLoaded = [this](const jive::SourceFile&) {
                bindControls();

#if JIVE_ENABLE_MELATONIN_INSPECTOR
                inspector->setRoot(*window->getComponent());
#endif
            };
            bindControls();

#if JIVE_ENABLE_MELATONIN_INSPECTOR
            inspector = std::make_unique<melatonin::Inspector>(*window->getComponent(), false);
            inspector->setVisible(true);
#endif
        }

    private:
        void bindControls()
        {
            fontSize = bindControl<float>("font-size-slider", "value");
            fontFamily = bindControl<int>("font-family-combo-box", "selected");
            bold = bindControl<bool>("bold-checkbox", "toggled");
            italic = bindControl<bool>("italic-checkbox", "toggled");
            underlined = bindControl<bool>("underlined-checkbox", "toggled");
            fontKerning = bindControl<float>("font-kerning-slider", "value");
            fontScale = bindControl<float>("font-scale-slider", "value");

            const auto languageComboBox = jive::findElementWithID(window->state,
                                                                  "language-combo-box");
            selectedLanguageIndex = std::make_unique<jive::Property<int>>(languageComboBox,
                                                                          "selected");
            selectedLanguageIndex->onValueChange = [this]() {
                applyLanguage();
            };

            const auto languageExample = jive::findElementWithID(window->state,
                                                                 "language-example");
            languageExampleLang = std::make_unique<jive::Property<juce::String>>(languageExample,
                                                                                 "lang");

            applyFontSettings();
            applyLanguage();
        }

        void applyLanguage()
        {
            if (selectedLanguageIndex->get() == 0)
                *languageExampleLang = "public/locales/english.strings";
            else if (selectedLanguageIndex->get() == 1)
                *languageExampleLang = "public/locales/german.strings";
            else if (selectedLanguageIndex->get() == 2)
                *languageExampleLang = "public/locales/french.strings";
        }

        template <typename T>
        [[nodiscard]] std::unique_ptr<jive::Property<T>> bindControl(const juce::String& id, const juce::Identifier& property)
        {
            auto control = std::make_unique<jive::Property<T>>(jive::findElementWithID(window->state, id), property);
            control->onValueChange = [this]() {
                applyFontSettings();
            };

            return control;
        }

        void applyFontSettings()
        {
            const auto option = jive::findElementWithID(window->state, "font-family-combo-box").getChild(fontFamily->get());

            jive::findElementWithID(window->state, "content")
                .setProperty("style",
                             new jive::Object{
                                 { "font-size", fontSize->get() },
                                 { "font-family", option["text"] },
                                 { "font-styles", getFontStyles() },
                                 { "font-kerning", fontKerning->get() },
                                 { "font-scale", fontScale->get() },
                             },
                             nullptr);

            setText("font-size-label", juce::String{ fontSize->get(), 1 } + "pt");
            setText("font-kerning-label", juce::String{ fontKerning->get(), 2 });
            setText("font-scale-label", juce::String{ fontScale->get(), 2 } + "x");
        }

        [[nodiscard]] juce::String getFontStyles() const
        {
            juce::StringArray styles;

            if (bold->get())
                styles.add("bold");
            if (italic->get())
                styles.add("italic");
            if (underlined->get())
                styles.add("underlined");

            return styles.isEmpty() ? "normal" : styles.joinIntoString(" ");
        }

        void setText(const juce::String& id, const juce::String& text)
        {
            jive::findElementWithID(window->state, id).setProperty("text", text, nullptr);
        }

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
        std::unique_ptr<jive::Property<float>> fontSize;
        std::unique_ptr<jive::Property<int>> fontFamily;
        std::unique_ptr<jive::Property<bool>> bold;
        std::unique_ptr<jive::Property<bool>> italic;
        std::unique_ptr<jive::Property<bool>> underlined;
        std::unique_ptr<jive::Property<float>> fontKerning;
        std::unique_ptr<jive::Property<float>> fontScale;

        std::unique_ptr<jive::Property<int>> selectedLanguageIndex;
        std::unique_ptr<jive::Property<juce::String>> languageExampleLang;

#if JIVE_ENABLE_MELATONIN_INSPECTOR
        std::unique_ptr<melatonin::Inspector> inspector;
#endif
    };
} // namespace jive_examples::text
