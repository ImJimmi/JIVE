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
#include "components/jive_DocumentWindow.h"
#include "components/jive_IgnoredComponent.h"
#include "components/jive_NormalisedProgressBar.h"
#include "components/jive_TextComponent.h"

#include "utilities/jive_AutoValue.h"
#include "utilities/jive_Drawable.h"
#include "utilities/jive_Length.h"
#include "utilities/jive_Orientation.h"
#include "utilities/jive_Overflow.h"
#include "utilities/jive_TypedValue.h"

#if JIVE_UNIT_TESTS
    #include "utilities/jive_StringStreams.h"
#endif

#include "layout/jive_BoxModel.h"

#include "layout/gui-items/jive_GuiItem.h"
#include "layout/gui-items/jive_GuiItemDecorator.h"

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
#include "utilities/jive_VariantConverters.h"
