#pragma once

#define JIVE_JUCE_VERSION(major, minor, patch) ((major << 16) + (minor << 8) + patch)
#define JIVE_JUCE_9                            (JUCE_VERSION >= JIVE_JUCE_VERSION(9, 0, 0))
