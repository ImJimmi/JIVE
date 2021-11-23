#pragma once

/* BEGIN_JUCE_MODULE_DECLARATION
    ID:                 jive
    vendor:             James Johnson
    version:            0.1.0
    name:               James' Intuitive View Explicator
    license:            GPL/Commercial
    minimumCppStandard: 14
    dependencies:       juce_gui_basics
END_JUCE_MODULE_DECLARATION */

//======================================================================================================================
#include <juce_gui_basics/juce_gui_basics.h>

//======================================================================================================================
#include "core/gui-items/jive_GuiItem.h"
#include "core/gui-items/jive_GuiItemDecorator.h"
#include "utilities/jive_TypedValue.h"

#include "core/gui-items/jive_GuiFlexContainer.h"
#include "core/gui-items/jive_GuiFlexItem.h"
#include "core/gui-items/widgets/jive_ComboBox.h"

#include "core/jive_VariantConverters.h"
#include "core/jive_ViewRenderer.h"
