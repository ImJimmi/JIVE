#pragma once

#include <jive_layouts/jive_layouts.h>

#include <shared/Inspector.h>
#include <shared/Presenter.h>
#include <shared/Theme.h>

namespace jive_examples::text
{
    class Presenter : public BasePresenter
    {
    public:
        Presenter()
        {
            window = interpreter.interpret(jive::SourceFile{ "public/views/Main.xml" });
            interpreter.onLoad = [this](jive::GuiItem& root) {
                // bindControls();
                inspector.inspect(*root.getComponent());
            };
            interpreter.onLoad(*window);
        }

    private:
        template <typename T>
        [[nodiscard]] auto bindControl(const juce::String& id, const juce::Identifier& property, std::function<void()> callback)
        {
            auto controlState = jive::findElementWithID(window->state, id);
            auto control = std::make_unique<jive::Property<T>>(controlState, property);
            control->onValueChange = callback;

            return control;
        }

        template <typename T>
        [[nodiscard]] auto bindFontControl(const juce::String& id, const juce::Identifier& property)
        {
            return bindControl<T>(id, property, [this]() {
                applyFontSettings();
            });
        }

        template <typename T>
        [[nodiscard]] auto bindLanguageControl(const juce::String& id, const juce::Identifier& property)
        {
            return bindControl<T>(id, property, [this]() {
                applyLanguage();
            });
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

        void applyFontSettings()
        {
            const auto option = jive::findElementWithID(window->state, "font-family-combo-box")
                                    .getChild(fontFamily->get());

            jive::findElementWithID(window->state, "content")
                .setProperty(jive::ids::style(),
                             new jive::Object{
                                 { "font-size", fontSize->get() },
                                 { "font-family", option[jive::ids::text()] },
                                 { "font-styles", getFontStyles() },
                                 { "font-kerning", fontKerning->get() },
                                 { "font-scale", fontScale->get() },
                             },
                             nullptr);
            jive::findElementWithID(window->state, "font-size-label")
                .setProperty("text",
                             juce::String{ fontSize->get(), 1 } + "pt",
                             nullptr);
            jive::findElementWithID(window->state, "font-kerning-label")
                .setProperty("text",
                             juce::String{ fontKerning->get(), 2 },
                             nullptr);
            jive::findElementWithID(window->state, "font-scale-label")
                .setProperty("text",
                             juce::String{ fontScale->get(), 2 } + "x",
                             nullptr);
        }

        void applyLanguage()
        {
            auto languageExampleState = jive::findElementWithID(window->state, "language-example");

            if (selectedLanguageIndex->get() == 0)
                languageExampleState.setProperty(jive::ids::lang(), "public/locales/english.strings", nullptr);
            else if (selectedLanguageIndex->get() == 1)
                languageExampleState.setProperty(jive::ids::lang(), "public/locales/german.strings", nullptr);
            else if (selectedLanguageIndex->get() == 2)
                languageExampleState.setProperty(jive::ids::lang(), "public/locales/french.strings", nullptr);
        }

        void bindControls()
        {
            theme = bindThemeComboBox(window->state);

            fontSize = bindFontControl<float>("font-size-slider", jive::ids::value());
            fontFamily = bindFontControl<int>("font-family-combo-box", jive::ids::selected());
            bold = bindFontControl<bool>("bold-checkbox", jive::ids::toggled());
            italic = bindFontControl<bool>("italic-checkbox", jive::ids::toggled());
            underlined = bindFontControl<bool>("underlined-checkbox", jive::ids::toggled());
            fontKerning = bindFontControl<float>("font-kerning-slider", jive::ids::value());
            fontScale = bindFontControl<float>("font-scale-slider", jive::ids::value());

            selectedLanguageIndex = bindLanguageControl<int>("language-combo-box", jive::ids::selected());

            applyFontSettings();
            applyLanguage();
        }

        std::unique_ptr<jive::Property<int>> theme;
        std::unique_ptr<jive::Property<float>> fontSize;
        std::unique_ptr<jive::Property<int>> fontFamily;
        std::unique_ptr<jive::Property<bool>> bold;
        std::unique_ptr<jive::Property<bool>> italic;
        std::unique_ptr<jive::Property<bool>> underlined;
        std::unique_ptr<jive::Property<float>> fontKerning;
        std::unique_ptr<jive::Property<float>> fontScale;

        std::unique_ptr<jive::Property<int>> selectedLanguageIndex;

        Inspector inspector;
    };
} // namespace jive_examples::text
