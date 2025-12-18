#include "Processor.h"

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new jive_example::GainPlugin;
}
