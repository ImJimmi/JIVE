#pragma once

/* BEGIN_JUCE_MODULE_DECLARATION
    ID:                 jive_layouts
    vendor:             ImJimmi
    version:            0.1.0
    name:               JIVE Layouts
    license:            MIT
    minimumCppStandard: 14
    dependencies:       juce_gui_basics
END_JUCE_MODULE_DECLARATION */

//======================================================================================================================
#include <juce_gui_basics/juce_gui_basics.h>

//======================================================================================================================
namespace jive
{
    class GuiItem;
} // namespace jive

//======================================================================================================================
#include "utilities/jive_IgnoredComponent.h"
#include "utilities/jive_TypedValue.h"

#include "layout/jive_BoxModel.h"

#include "layout/gui-items/jive_GuiItem.h"
#include "layout/gui-items/jive_GuiItemDecorator.h"
#include "layout/gui-items/widgets/jive_TextWidget.h"

#include "layout/gui-items/flex/jive_FlexContainer.h"
#include "layout/gui-items/flex/jive_FlexItem.h"
#include "layout/gui-items/widgets/jive_Label.h"

#include "layout/jive_ViewRenderer.h"
#include "utilities/jive_VariantConverters.h"
