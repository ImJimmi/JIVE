#pragma once

#include "Benchmark.h"

class StyleSheetsConstructionBenchmark : public Benchmark
{
public:
    StyleSheetsConstructionBenchmark()
        : Benchmark{
            "Style Sheets - construct",
            juce::RelativeTime::seconds(5.0),
        }
    {
    }

protected:
    void doIteration(jive::Interpreter&) final
    {
        juce::Component component;
        static const juce::ValueTree state{
            "Component",
            {
                { "width", 100000 },
                { "height", 100000 },
                {
                    "style",
                    new jive::Object{
                        { "background", "#000000" },
                        {
                            "hover",
                            new jive::Object{
                                { "background", "#111111" },
                                {
                                    "disabled",
                                    new jive::Object{
                                        { "background", "#222222" },
                                    },

                                },
                            },
                        },
                        {
                            "disabled",
                            new jive::Object{
                                { "background", "#333333" },
                                {
                                    "focus",
                                    new jive::Object{
                                        { "background", "#444444" },
                                    },
                                },
                                {
                                    "#my-component",
                                    new jive::Object{
                                        { "background", "#555555" },
                                    },
                                },
                                {
                                    ".primary-button",
                                    new jive::Object{
                                        { "background", "#666666" },
                                        {
                                            "checked",
                                            new jive::Object{
                                                { "background", "#777777" },
                                                {
                                                    "active",
                                                    new jive::Object{
                                                        { "background", "#888888" },
                                                    },
                                                },
                                            },
                                        },
                                    },
                                },
                            },
                        },
                    },
                },
            },
        };

        juce::ignoreUnused(jive::StyleSheet::create(component, state));
    }
};

class StyleSheetsQueryingBenchmark : public Benchmark
{
public:
    StyleSheetsQueryingBenchmark()
        : Benchmark{
            "Style Sheets - querying styles",
            juce::RelativeTime::seconds(2.0),
        }
    {
        static const juce::ValueTree state{
            "Component",
            {
                { "width", 100000 },
                { "height", 100000 },
                {
                    "style",
                    new jive::Object{
                        { "background", "#000000" },
                        {
                            "hover",
                            new jive::Object{
                                { "background", "#111111" },
                                {
                                    "disabled",
                                    new jive::Object{
                                        { "background", "#222222" },
                                    },

                                },
                            },
                        },
                        {
                            "disabled",
                            new jive::Object{
                                { "background", "#333333" },
                                {
                                    "focus",
                                    new jive::Object{
                                        { "background", "#444444" },
                                    },
                                },
                                {
                                    "#my-component",
                                    new jive::Object{
                                        { "background", "#555555" },
                                    },
                                },
                                {
                                    ".primary-button",
                                    new jive::Object{
                                        { "background", "#666666" },
                                        {
                                            "checked",
                                            new jive::Object{
                                                { "background", "#777777" },
                                                {
                                                    "active",
                                                    new jive::Object{
                                                        { "background", "#888888" },
                                                    },
                                                },
                                            },
                                        },
                                    },
                                },
                            },
                        },
                    },
                },
            },
        };

        styleSheet = jive::StyleSheet::create(component, state);
    }

protected:
    void doIteration(jive::Interpreter&) final
    {
        juce::ignoreUnused(styleSheet->getBackground());
        juce::ignoreUnused(styleSheet->getForeground());
        juce::ignoreUnused(styleSheet->getBorderFill());
        juce::ignoreUnused(styleSheet->getBorderRadii());
        juce::ignoreUnused(styleSheet->getFont());
    }

private:
    juce::Component component;
    jive::StyleSheet::ReferenceCountedPointer styleSheet;
};
