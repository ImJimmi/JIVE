#include "Presenter.h"

#include <shared/Application.h>

#if defined(JUCE_PROJUCER_VERSION)
static const jive::SourceRoot sourceRoot{ juce::File{ PROJECT_ROOT } };
#endif

START_JUCE_APPLICATION(jive_examples::Application<jive_examples::layouts::Presenter>)
