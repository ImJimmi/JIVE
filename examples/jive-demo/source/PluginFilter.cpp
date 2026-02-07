#include <jive_demo/PluginProcessor.h>

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new jive_demo::PluginProcessor{};
}
