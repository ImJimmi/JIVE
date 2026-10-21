#include "PluginProcessor.h"

#if defined(JUCE_PROJUCER_VERSION)
static const jive::SourceRoot sourceRoot{ juce::File{ PROJECT_ROOT } };
#endif

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new jive_examples::widgets::Processor;
}
