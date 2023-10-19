#pragma once

#include <jive_demo/gui/tokens/Colours.h>

namespace jive_demo
{
    class CodeExamplePage
    {
    public:
        [[nodiscard]] auto& withBodyText(const juce::String& text)
        {
            bodyText = text;
            return *this;
        }

        [[nodiscard]] auto& withCodeSnippet(const juce::String& codeSnippet)
        {
            code = codeSnippet;
            return *this;
        }

        [[nodiscard]] auto& withComparisonSnippet(const juce::String& comparisonSnippet)
        {
            view = codeView(comparisonSnippet);
            return *this;
        }

        [[nodiscard]] auto& withExampleView(const juce::ValueTree& exampleView)
        {
            view = exampleView.createCopy();
            return *this;
        }

        [[nodiscard]] auto operator()()
        {
            return juce::ValueTree{
                "Component",
                {
                    { "padding", 25 },
                    { "justify-content", "space-between" },
                },
                {
                    juce::ValueTree{
                        "Component",
                        {
                            { "margin", "0 0 20 0" },
                        },
                        {
                            juce::ValueTree{
                                "Text",
                                {
                                    { "text", bodyText },
                                    { "margin", "0 0 20 0" },
                                },
                            },
                            examplesRow(),
                        },
                    },
                },
            };
        }

    private:
        class Panel
        {
        public:
            [[nodiscard]] auto& withContent(const juce::ValueTree& newContent)
            {
                content = newContent.createCopy();
                return *this;
            }

            [[nodiscard]] juce::ValueTree operator()()
            {
                static constexpr auto style = [] {
                    return new jive::Object{
                        { "background", jive::toVar(colours::panel) },
                        { "border", "black" },
                        { "border-radius", 10 },
                    };
                };

                return juce::ValueTree{
                    "Component",
                    {
                        { "style", style() },
                        { "flex-grow", 1 },
                        { "padding", 10 },
                        { "border-width", 1.5 },
                    },
                    {
                        content.createCopy(),
                    },
                };
            }

        private:
            juce::ValueTree content;
        };

        [[nodiscard]] juce::ValueTree codeView(const juce::String& text) const
        {
            return juce::ValueTree{
                "Text",
                {
                    { "text", text },
                    { "word-wrap", "none" },
                    {
                        "style",
                        new jive::Object{
                            { "font-family", "Source Code Pro" },
                            { "font-size", 12 },
                        },
                    },
                },
            };
        }

        [[nodiscard]] juce::ValueTree examplesRow()
        {
            return juce::ValueTree{
                "Component",
                {
                    { "display", "grid" },
                    { "grid-template-columns", "1fr 1fr" },
                    { "grid-template-rows", "1fr" },
                    { "gap", 15 },
                },
                {
                    Panel{}
                        .withContent(codeView(code))(),
                    Panel{}
                        .withContent(view)(),
                },
            };
        }

        juce::String bodyText;
        juce::String code;
        juce::ValueTree view;
    };
} // namespace jive_demo
