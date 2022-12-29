#pragma once

/* BEGIN_JUCE_MODULE_DECLARATION
    ID:                 jive_layouts
    vendor:             ImJimmi
    version:            0.1.0
    name:               JIVE Layouts
    license:            MIT
    minimumCppStandard: 14
    dependencies:       jive_components jive_core
END_JUCE_MODULE_DECLARATION */

#define JIVE_LAYOUTS_H_INCLUDED

//======================================================================================================================
#include <jive_components/jive_components.h>
#include <jive_core/jive_core.h>

//======================================================================================================================
namespace jive
{
    class GuiItem;
} // namespace jive

//======================================================================================================================
#include "utilities/jive_ComponentFactory.h"
#include "utilities/jive_Display.h"
#include "utilities/jive_Drawable.h"
#include "utilities/jive_LayoutStrategy.h"
#include "utilities/jive_Orientation.h"
#include "utilities/jive_Overflow.h"

#include "layout/gui-items/jive_GuiItem.h"
#include "layout/gui-items/jive_GuiItemDecorator.h"

#include "layout/gui-items/jive_ContainerItem.h"

#include "layout/gui-items/block/jive_BlockContainer.h"
#include "layout/gui-items/block/jive_BlockItem.h"
#include "layout/gui-items/content/jive_Image.h"
#include "layout/gui-items/content/jive_Text.h"
#include "layout/gui-items/flex/jive_FlexContainer.h"
#include "layout/gui-items/flex/jive_FlexItem.h"
#include "layout/gui-items/grid/jive_GridContainer.h"
#include "layout/gui-items/grid/jive_GridItem.h"
#include "layout/gui-items/widgets/jive_Button.h"
#include "layout/gui-items/widgets/jive_ComboBox.h"
#include "layout/gui-items/widgets/jive_Label.h"
#include "layout/gui-items/widgets/jive_ProgressBar.h"
#include "layout/gui-items/widgets/jive_Slider.h"
#include "layout/gui-items/widgets/jive_Window.h"

#include "layout/gui-items/widgets/jive_Hyperlink.h"
#include "layout/gui-items/widgets/jive_Knob.h"
#include "layout/gui-items/widgets/jive_Spinner.h"

#include "layout/jive_Interpreter.h"
