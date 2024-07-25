#pragma once

/* BEGIN_JUCE_MODULE_DECLARATION
    ID:                 jive_core
    vendor:             ImJimmi
    name:               JIVE Core
    license:            MIT
    dependencies:       juce_gui_basics
END_JUCE_MODULE_DECLARATION */

#define JIVE_CORE_H_INCLUDED

#include <juce_gui_basics/juce_gui_basics.h>

#include "logging/jive_ConsoleProgressBar.h"
#include "logging/jive_ScopeIndentedLogger.h"
#include "logging/jive_StringStreams.h"

#include "algorithms/jive_Find.h"
#include "algorithms/jive_Interpolate.h"
#include "algorithms/jive_TransferFunction.h"
#include "algorithms/jive_Visitor.h"

#include "algorithms/jive_Bezier.h"

#include "kinetics/jive_Easing.h"
#include "time/jive_TimeParser.h"
#include "time/jive_Timer.h"
#include "values/jive_PropertyBehaviours.h"

#include "values/jive_Colours.h"
#include "values/jive_Event.h"
#include "values/jive_Object.h"
#include "values/jive_Property.h"
#include "values/jive_ReferenceCountedValueTreeWrapper.h"
#include "values/jive_XmlParser.h"
#include "values/variant-converters/jive_AttributedStringVariantConverters.h"
#include "values/variant-converters/jive_FlexVariantConverters.h"
#include "values/variant-converters/jive_GridVariantConverters.h"
#include "values/variant-converters/jive_MiscVariantConverters.h"
#include "values/variant-converters/jive_VariantConvertion.h"

#include "geometry/jive_BorderRadii.h"
#include "geometry/jive_Length.h"
#include "geometry/jive_Orientation.h"

#include "geometry/jive_BoxModel.h"

#include "graphics/jive_FontUtilities.h"
#include "graphics/jive_Gradient.h"
#include "graphics/jive_LookAndFeel.h"

#include "graphics/jive_Fill.h"

#include "interface/jive_ComponentInteractionState.h"

#include "kinetics/jive_Transitions.h"
