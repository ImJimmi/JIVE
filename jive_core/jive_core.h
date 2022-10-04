#pragma once

/* BEGIN_JUCE_MODULE_DECLARATION
    ID:                 jive_core
    vendor:             ImJimmi
    version:            0.1.0
    name:               JIVE Core
    license:            MIT
    minimumCppStandard: 14
    dependencies:       juce_gui_basics
END_JUCE_MODULE_DECLARATION */

#define JIVE_CORE_H_INCLUDED

//======================================================================================================================
#include <juce_gui_basics/juce_gui_basics.h>

//======================================================================================================================
#include "logging/jive_StringStreams.h"

#include "values/jive_AutoValue.h"
#include "values/jive_TypedValue.h"
#include "values/variant-converters/jive_AttributedStringVariantConverters.h"
#include "values/variant-converters/jive_FlexVariantConverters.h"
#include "values/variant-converters/jive_GridVariantConverters.h"
#include "values/variant-converters/jive_MiscVariantConverters.h"
