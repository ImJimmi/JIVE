#include "PluginProcessor.h"

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new jive_examples::widgets::Processor; // NOLINT
}
