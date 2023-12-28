#include <jive_demo/PluginProcessor.h>

juce::AudioProcessor* createPluginFilter()
{
    return new jive_demo::PluginProcessor{};
}
