#include "jive_ConsoleProgressBar.h"

namespace jive
{
    static const juce::Array<juce::juce_wchar> blocks{
        ' ',                                       //
        *juce::CharPointer_UTF8{ "\xe2\x96\x8f" }, // ▏
        *juce::CharPointer_UTF8{ "\xe2\x96\x8e" }, // ▎
        *juce::CharPointer_UTF8{ "\xe2\x96\x8d" }, // ▍
        *juce::CharPointer_UTF8{ "\xe2\x96\x8c" }, // ▌
        *juce::CharPointer_UTF8{ "\xe2\x96\x8b" }, // ▋
        *juce::CharPointer_UTF8{ "\xe2\x96\x8a" }, // ▊
        *juce::CharPointer_UTF8{ "\xe2\x96\x89" }, // ▉
        *juce::CharPointer_UTF8{ "\xe2\x96\x88" }, // █
    };

    [[nodiscard]] juce::String buildProgressBar(double progressNormalised, int maxStringLength)
    {
        jassert(progressNormalised >= 0.0 && progressNormalised <= 1.0);
        progressNormalised = juce::jlimit(0.0, 1.0, progressNormalised);

        static constexpr auto decimalPlaces = 2;
        static constexpr auto hundredPercentAndWhitespaceWidth = decimalPlaces + 6; // 100.00%
        const auto barLength = maxStringLength - hundredPercentAndWhitespaceWidth - 2;
        const auto filledPortionExactWidth = barLength * progressNormalised;
        const auto numFull = static_cast<int>(std::floor(filledPortionExactWidth));
        const auto partialBlock = blocks[juce::roundToInt((filledPortionExactWidth - numFull) * (blocks.size() - 1))];
        const auto bar = juce::String{}
                             .paddedRight(blocks.getLast(), numFull)
                             .paddedRight(partialBlock, juce::jmin(numFull + 1, barLength))
                             .paddedRight(blocks.getFirst(), barLength);

        static const juce::String cap{ juce::CharPointer_UTF8("\xe2\x94\x82") };
        const auto percentage = (juce::String{ progressNormalised * 100, decimalPlaces }
                                 + "%")
                                    .paddedLeft(' ', hundredPercentAndWhitespaceWidth);

        return cap + bar + cap + percentage;
    }
} // namespace jive
