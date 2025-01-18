#include "Processor.h"

[[nodiscard]] juce::AudioProcessor* createPluginFilter()
{
    return new jive_example::GainPlugin;
}
