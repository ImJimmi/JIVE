# JIVE Layouts

JIVE's layouts API enables you to build UIs using XML.

The interpreter will following the hierarchy of your XML document to build the same hierarchy of components, and will follow the properties on those elements to decide how components should be laid out. The XML sources are parsed to `juce::ValueTree`s allowing you to then edit the UI at runtime by adding, removing, and changing elements.

See [Your First View](../docs/guides/01-Your%20First%20View.md) for an in-depth walkthrough of building a UI with JIVE. This document details all the different UI elements, and their corresponding properties.

## Properties

### Common

The following properties apply to all elements.

| Property                 | Description                                                                                                                                                                                                                                                                                                                                                                                                                                        |
| ------------------------ | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `"accessible"`           | Whether the element is visible to accessibility clients.<br>See [`juce::Component::setAccessible()`](https://docs.juce.com/master/classjuce_1_1Component.html#a25a51c7896cf01093ff7af26ed0f3ec7).                                                                                                                                                                                                                                                  |
| `"always-on-top"`        | Whether the element is kept in front of its siblings.<br>See [`juce::Component::setAlwaysOnTop()`](https://docs.juce.com/master/classjuce_1_1Component.html#a7f2af71c2c981fb7c43d7262563d185f).                                                                                                                                                                                                                                                    |
| `"border-width"`         | The width of the component's border.<br>See CSS [`border-width`](https://developer.mozilla.org/en-US/docs/Web/CSS/Reference/Properties/border-width) for supported values.<br>_Can also be specified in style sheets._                                                                                                                                                                                                                             |
| `"buffered-to-image"`    | Whether the element caches its graphics to an image, to save on expensive renderer.<br>See [`juce::Component::setBufferedToImage()`](https://docs.juce.com/master/classjuce_1_1Component.html#aaee47f5eae3503983f5761e6b7768615).                                                                                                                                                                                                                  |
| `"class"`                | A space-separated list of style classes that this component belongs to. Style sheets can then select components using a given class with `".className"`.                                                                                                                                                                                                                                                                                           |
| `"clicking-grabs-focus"` | Whether clicking the element gives it keyboard focus.<br>See [`juce::Component::setMouseClickGrabsKeyboardFocus()`](https://docs.juce.com/master/classjuce_1_1Component.html#abae9ee1cc71155087c5096b45cac9e39).                                                                                                                                                                                                                                   |
| `"cursor"`               | The mouse cursor shown over the element.<br>One of `inherit`, `none`, `default`, `wait`, `text`, `crosshair`, `copy`, `pointer`, `grab`, `left-right`, `up-down`, `move`, `up`, `down`, `left`, `right`, `up-left`, `up-right`, `down-left`, or `down-right`.<br>See [`juce::Component::setMouseCursor()`](https://docs.juce.com/master/classjuce_1_1Component.html#a86eae51f335c478f4638746b15f9d1ec).                                            |
| `"description"`          | The element's description for accessibility clients.<br>See [`juce::Component::setDescription()`](https://docs.juce.com/master/classjuce_1_1Component.html#a842845eee5be204fa512b13e0ef13a4b).                                                                                                                                                                                                                                                     |
| `"display"`              | The layout engine used to position the component's children.<br>One of `flex`, `grid`, or `block`.<br>See [Flex containers](#flex-containers), [Flex items](#flex-items), [Grid containers](#grid-containers), [Grid items](#grid-items), and [Block items](#block-items).                                                                                                                                                                         |
| `"enabled"`              | The component's enablement state.<br>See [`juce::Component::setEnabled()`](https://docs.juce.com/master/classjuce_1_1Component.html#a8a3928e0a8124fc44a24dfc11afbaf91).                                                                                                                                                                                                                                                                            |
| `"focus-order"`          | The component's position in the keyboard focus order among its siblings.<br>By default, this will be the position of the element in the XML.<br>See [`juce::Component::setExplicitFocusOrder()`](https://docs.juce.com/master/classjuce_1_1Component.html#ad5b9e906fd9222cc67686bb080cbae58).                                                                                                                                                      |
| `"focus-outline"`        | Whether an outline is drawn around the component when it has keyboard focus.<br>See [`juce::Component::setHasFocusOutline()`]([setHasFocusOutline](https://docs.juce.com/master/classjuce_1_1Component.html#ab76b1a1cbac597be27c63b081f62c1ca)).                                                                                                                                                                                                   |
| `"focusable"`            | Whether the component can receive keyboard focus.<br>See [`juce::Component::setWantsKeyboardFocus()`](https://docs.juce.com/master/classjuce_1_1Component.html#ab7ddcb4bf8a7c6534f6e1d9a22e6330b).                                                                                                                                                                                                                                                 |
| `"height"`               | The component's height, in pixels or as a percentage of its parent's height.                                                                                                                                                                                                                                                                                                                                                                       |
| `"id"`                   | The component's ID. It's assumed that ID's are unique.<br>See [`juce::Component::setComponentID()`](https://docs.juce.com/master/classjuce_1_1Component.html#a190b586046ade66445e0ed23c2e9587f).                                                                                                                                                                                                                                                   |
| `"lang"`                 | The path of a `.strings` file to translate the text of the element and its descendants.<br>See [`juce::LocalisedStrings`](https://docs.juce.com/master/classjuce_1_1LocalisedStrings.html).                                                                                                                                                                                                                                                        |
| `"margin"`               | The amount of space to leave around a component.<br>See CSS [`margin`](https://developer.mozilla.org/en-US/docs/Web/CSS/Reference/Properties/margin) for supported values.                                                                                                                                                                                                                                                                         |
| `"max-height"`           | The maximum height the component can have.                                                                                                                                                                                                                                                                                                                                                                                                         |
| `"max-width"`            | The maximum width the component can have.                                                                                                                                                                                                                                                                                                                                                                                                          |
| `"min-height"`           | The minimum height the component can have.                                                                                                                                                                                                                                                                                                                                                                                                         |
| `"min-width"`            | The minimum width the component can have.                                                                                                                                                                                                                                                                                                                                                                                                          |
| `"name"`                 | The component's name.<br>See [`juce::Component::setName()`](https://docs.juce.com/master/classjuce_1_1Component.html#a8a7af7334f6136ed745b82a1b55d93d1).                                                                                                                                                                                                                                                                                           |
| `"opaque"`               | Whether the component paints every pixel of its bounds, which lets JUCE skip painting anything behind it.<br>See [`juce::Component::setOpaque()`](https://docs.juce.com/master/classjuce_1_1Component.html#a862fe9788d2f2a16f9ad2bb7d8f2e0ec).                                                                                                                                                                                                     |
| `"padding"`              | The amount of space to inset the component's children by.<br>See CSS [`padding`](https://developer.mozilla.org/en-US/docs/Web/CSS/Reference/Properties/padding) for supported values.                                                                                                                                                                                                                                                              |
| `"source"`               | The path to an XML or SVG file to build the component from. This should be a relative path to a source root defined with either `jive_sources()` in CMake, or `jive::SourceRoot` in C++, or should be a path relative to the current file starting with `"./"` or `"../"`.<br>See [Your First View](../docs/guides/01-Your%20First%20View.md) for a more in-depth guide to sources, and the [example projects](../examples/) for working examples. |
| `"title"`                | The element's title for accessibility clients.<br>See [`juce::Component::setTitle()`](https://docs.juce.com/master/classjuce_1_1Component.html#a51b89d7bd4d43ef495f21ceab8c008e6).                                                                                                                                                                                                                                                                 |
| `"tooltip"`              | Text shown when the mouse hovers over the component.<br>Tooltips are only shown if the application or plugin has a [`juce::TooltipWindow`](https://docs.juce.com/master/classjuce_1_1TooltipWindow.html).<br>See [`juce::Component::setHelpText()`](https://docs.juce.com/master/classjuce_1_1Component.html#ad84ab870f0f31829fac6d9714ecf4558).                                                                                                   |
| `"transition"`           | The rules for how layout properties transition from one value to another.<br>See CSS [`transition`](https://developer.mozilla.org/en-US/docs/Web/CSS/Reference/Properties/transition) for supported values.                                                                                                                                                                                                                                        |
| `"visibility"`           | Whether the component is visible, either `true` or `false`.<br>See [`juce::Component::setVisible()`](https://docs.juce.com/master/classjuce_1_1Component.html#ac85c5bf98ad6ffc1a252cad38f1aca40).                                                                                                                                                                                                                                                  |
| `"width"`                | The component's width, in pixels or as a percentage of its parent's width.                                                                                                                                                                                                                                                                                                                                                                         |

### Flex Layouts

Component's with a `"display"` property of `flex` use the [`juce::FlexBox`](https://docs.juce.com/master/classjuce_1_1FlexBox.html) layout, based on [CSS FlexBox](https://developer.mozilla.org/en-US/docs/Web/CSS/Guides/Flexible_box_layout/Basic_concepts).

This is the default layout, with a `"direction"` of `column`, and an `"align-items"` of `start`, so a components' children are laid out top-to-bottom, aligned to the left by default.

#### Flex Containers

Flex containers can have the following properties from CSS FlexBox:

- `"align-content"`
- `"align-items"`
- `"flex-direction"`
- `"flex-wrap"`
- `"gap"`
- `"justify-content"`

#### Flex Items

Children of flex containers can have the following properties from CSS FlexBox:

- `"align-self"`
- `"flex-basis"`
- `"flex-grow"`
- `"flex-shrink"`
- `"order"`

### Grid Layouts

Component's with a `"display"` property of `grid` use the [`juce::Grid`](https://docs.juce.com/master/classjuce_1_1Grid.html) layout, based on [CSS Grid](https://developer.mozilla.org/en-US/docs/Web/CSS/Guides/Grid_layout).

#### Grid Containers

Grid containers can have the following properties from CSS:

- `"align-content"`
- `"align-items"`
- `"gap"`
- `"grid-auto-columns"`
- `"grid-auto-flow"`
- `"grid-auto-rows"`
- `"grid-template-areas"`
- `"grid-template-columns"`
- `"grid-template-rows"`
- `"justify-content"`
- `"justify-items"`

#### Grid Items

Children of grid containers can have the following properties from CSS FlexBox:

- `"align-self"`
- `"grid-area"`
- `"grid-column"`
- `"grid-row"`
- `"justify-self"`
- `"order"`

### Block Layouts

Components with a `"display"` property of `block` don't arrange their children. Instead, each child gives its own position and size.

#### Block Containers

There are no additional properties that can be set on block containers themselves.

#### Block Items

Children of block containers can have the following properties:

| Property   | Description                                                                                            |
| ---------- | ------------------------------------------------------------------------------------------------------ |
| `"anchor"` | The point on the child that is placed at `"x"` and `"y"`.                                              |
| `"x"`      | How far the anchor is from the left of the parent, in pixels or as a percentage of the parent's width. |
| `"y"`      | How far the anchor is from the top of the parent, in pixels or as a percentage of the parent's height. |

Sizes are set with the `"width"` and `"height"` properties.

The anchor can be one of the named points `top-left`, `centred-top`, `top-right`, `centred-left`, `centred`, `centred-right`, `bottom-left`, `centred-bottom`, or `bottom-right`. It can also be a horizontal and a vertical distance from the child's top-left corner, such as `10px 50%` or `32px` to specify both dimensions at once. Percentages in an anchor are of the child's own size, not its parent's, so `100% 100%` is equivalent to `bottom-right`.

### Windows and Editors

The following properties apply only to `<Window>` elements, except `theme` which also applies to `<Editor>` elements.

| Property              | Description                                                                                                                                                                                                                                                                  |
| --------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `"corner-resizer"`    | Whether the window is resized with a handle in its bottom-right corner rather than from its edges.<br>See [`juce::ResizableWindow::setResizable()`](https://docs.juce.com/master/classjuce_1_1ResizableWindow.html#a029c9ae46231ac2347628addc4dea9e2).                       |
| `"draggable"`         | Whether the window can be moved by dragging it.<br>See [`juce::ResizableWindow::setDraggable()`](https://docs.juce.com/master/classjuce_1_1ResizableWindow.html#a3406012b9ae0532886baa4e1a4e6ea90).                                                                          |
| `"full-screen"`       | Whether the window is full-screen.<br>See [`juce::ResizableWindow::setFullScreen()`](https://docs.juce.com/master/classjuce_1_1ResizableWindow.html#a0d00782c82a84f03a88a4079796b25a4).                                                                                      |
| `"minimised"`         | Whether the window is minimised.<br>See [`juce::ResizableWindow::setMinimised()`](https://docs.juce.com/master/classjuce_1_1ResizableWindow.html#af8a590a371ae703fa7ac435ca6da3482).                                                                                         |
| `"native"`            | Whether the window uses the operating system's title bar rather than JUCE's.<br>See [`juce::TopLevelWindow::setUsingNativeTitleBar()`](https://docs.juce.com/master/classjuce_1_1TopLevelWindow.html#ad2c42cecc9730c0b4b9f3d78c31b4e7d).                                     |
| `"resizable"`         | Whether the window can be resized.<br>See [`juce::ResizableWindow::setResizable()`](https://docs.juce.com/master/classjuce_1_1ResizableWindow.html#a029c9ae46231ac2347628addc4dea9e2).                                                                                       |
| `"shadow"`            | Whether the window has a drop shadow.<br>See [`juce::TopLevelWindow::setDropShadowEnabled()`](https://docs.juce.com/master/classjuce_1_1TopLevelWindow.html#a48988bcbdd516bb41827f4d583a45f74).                                                                              |
| `"theme"`             | The theme to use as a base for style sheets.<br>One of `juce`, `steel`, `material`, `synthwave`, or `high-contrast`.                                                                                                                                                         |
| `"title-bar-buttons"` | Which buttons a non-native title bar shows. Any of `close`, `minimise`, and `maximise`, separated by spaces.<br>See [`juce::DocumentWindow::setTitleBarButtonsRequired()`](https://docs.juce.com/master/classjuce_1_1DocumentWindow.html#aae769ea9d51a1d89b97f8a312975a5d1). |
| `"title-bar-height"`  | The height of a non-native title bar.<br>See [`juce::DocumentWindow::setTitleBarHeight()`](https://docs.juce.com/master/classjuce_1_1DocumentWindow.html#a9288b3e3bac755afa1af36167002f20c).                                                                                 |

### Text

The following properties apply only to `<Text>` elements.

| Property         | Description                                                                                                                                                                                         |
| ---------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `"line-spacing"` | Extra spacing between lines of text.<br>_Can also be specified in style sheets._                                                                                                                    |
| `"text"`         | The text to show. Text can also be written inline (`<Text>Hello</Text>`) and it will be parsed to the `"text"` property (`<Text text="Hello"/>`) as `juce::ValueTree` does not support inline text. |
| `"word-wrap"`    | Where lines of text can be broken.<br>One of `none`, `by-word`, or `by-character`.                                                                                                                  |

### Buttons

The following properties apply only to `<Button>`, `<Checkbox>`, and `<Hyperlink>` elements.

Inline text in buttons is parsed as a `<Text>` child component - i.e. `<Button>Click Me!</Button>` becomes `<Button><Text text="Click Me!"/></Button>`.

| Property            | Description                                                                                                                                                                                                                                            |
| ------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| `"on-click"`        | The event triggered when the button is clicked.                                                                                                                                                                                                        |
| `"radio-group"`     | A number shared by buttons that should act as a group, where turning one on turns the others off.<br>See [`juce::Button::setRadioGroupId()`](https://docs.juce.com/master/classjuce_1_1Button.html#ac7b60ccbdff7ac0f41bf3428734646b0)                  |
| `"toggle-on-click"` | Whether clicking the button toggles it on and off.<br>See [`juce::Button::setClickingTogglesState()`](https://docs.juce.com/master/classjuce_1_1Button.html#ad60fbb26321b67312a2f112714ccee45)                                                         |
| `"toggleable"`      | Whether the button has an on and off state.<br>See [`juce::Button::setToggleable()`](https://docs.juce.com/master/classjuce_1_1Button.html#a54481d41657a7efd6174f9c442d8416a).                                                                         |
| `"toggled"`         | Whether the button is on.<br>See [`juce::Button::setToggleState()`](https://docs.juce.com/master/classjuce_1_1Button.html#a3e785fdf7d365fb30c63c95bb25bec79).                                                                                          |
| `"trigger-event"`   | Whether the button is clicked when the mouse is pressed or released.<br>One of `mouse-down` or `mouse-up`.<br>See [`juce::Button::setTriggeredOnMouseDown()`](https://docs.juce.com/master/classjuce_1_1Button.html#a541346eefa893d6d67ad7ecd84682345) |

#### Hyperlinks

The following properties apply only to `<Hyperlink>` elements.

| Property | Description                                                                                                                                                                     |
| -------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `"url"`  | The URL opened when the link is clicked.<br>See [`juce::Hyperlink::setURL()`](https://docs.juce.com/master/classjuce_1_1HyperlinkButton.html#a313b6d4bba18f19846bfece1a9423de9) |

### Combo Boxes

The following properties apply only to `<ComboBox>` elements.

| Property      | Description                                                                                                                                                                                           |
| ------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `"editable"`  | Whether the user can type their own text into the combo box.<br>See [`juce::ComboBox::setEditableText()`](https://docs.juce.com/master/classjuce_1_1ComboBox.html#aabe0db2e0cc6d62f016ea7975e36b56b). |
| `"on-change"` | The event triggered when the selection changes.                                                                                                                                                       |
| `"selected"`  | The index of the selected option, starting from `0`.<br>See [`juce::ComboBox::setSelectedItemIndex()`](https://docs.juce.com/master/classjuce_1_1ComboBox.html#a664c8726480a2f50772449d5d043f9d9)     |

Choices are added to combo boxes with `<Option>`, `<Header>`, and `<Separator>` children. Options take their text from their inline text, and can also have `enabled`, `selected`, `id`, and `class` properties.

```xml
<ComboBox>
    <Header>Channels</Header>
    <Option>Mono</Option>
    <Option selected="true">Stereo</Option>
    <Separator/>
    <Option>5.0</Option>
    <Option>5.1</Option>
    <Option>7.0</Option>
    <Option>7.1</Option>
</ComboBox>
```

### Sliders

The following properties apply only to `<Slider>`, `<Knob>`, and `<Spinner>` elements.

| Property                 | Description                                                                                                                                                                                                                                       |
| ------------------------ | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `"interval"`             | The step between values the slider can be set to.<br>See [`juce::Slider::setRange()`](https://docs.juce.com/master/classjuce_1_1Slider.html#af38f0040c0f1e028cdb89514a878ab7a).                                                                   |
| `"max"`                  | The slider's maximum value.<br>See [`juce::Slider::setRange()`](https://docs.juce.com/master/classjuce_1_1Slider.html#af38f0040c0f1e028cdb89514a878ab7a).                                                                                         |
| `"mid"`                  | The value at the slider's centre, to skew its range. If this isn't set, the range is linear.<br>See [`juce::Slider::setSkewFactor()`](https://docs.juce.com/master/classjuce_1_1Slider.html#a405a18402f5832ffd2355c774a829e0f).                   |
| `"min"`                  | The slider's minimum value.<br>See [`juce::Slider::setRange()`](https://docs.juce.com/master/classjuce_1_1Slider.html#af38f0040c0f1e028cdb89514a878ab7a).                                                                                         |
| `"on-change"`            | The event triggered when the value changes.                                                                                                                                                                                                       |
| `"orientation"`          | The direction a `<Slider>` is drawn and dragged in.<br>One of `horizontal` or `vertical`.                                                                                                                                                         |
| `"sensitivity"`          | How far the value moves when the slider is dragged. Higher numbers move further.<br>See [`juce::Slider::setMouseDragSensitivity()`](https://docs.juce.com/master/classjuce_1_1Slider.html#af72d1b8cc9a30e6bbc16e74be4426728).                     |
| `"snap-to-mouse"`        | Whether clicking a slider's track moves the thumb to the mouse.<br>See [`juce::Slider::setSliderSnapsToMousePosition()`](https://docs.juce.com/master/classjuce_1_1Slider.html#adf006b95e6911e7b42a2b5dbd11f247e).                                |
| `"value"`                | The slider's current value.<br>See [`juce::Slider::getValue()`](https://docs.juce.com/master/classjuce_1_1Slider.html#acc63d8618294c03981cd90ce83a8f776)                                                                                          |
| `"velocity-mode"`        | Whether dragging faster changes the value by more.<br>See [`juce::Slider::setVelocityBasedMode()`](https://docs.juce.com/master/classjuce_1_1Slider.html#a75a4cf0e1d62996b0c8ec6a118d1157b).                                                      |
| `"velocity-offset"`      | In velocity mode, a minimum speed the value always moves at while dragging.<br>See [`juce::Slider::setVelocityModeParameters()`](https://docs.juce.com/master/classjuce_1_1Slider.html#a28f59b951a863aaa313eae188ed22310).                        |
| `"velocity-sensitivity"` | In velocity mode, how much the speed of the drag affects the value.<br>See [`juce::Slider::setVelocityModeParameters()`](https://docs.juce.com/master/classjuce_1_1Slider.html#a28f59b951a863aaa313eae188ed22310)..                               |
| `"velocity-threshold"`   | In velocity mode, the number of pixels the mouse must move before velocity is taken into account.<br>See [`juce::Slider::setVelocityModeParameters()`](https://docs.juce.com/master/classjuce_1_1Slider.html#a28f59b951a863aaa313eae188ed22310).. |

#### Spinners

The following properties apply only to `<Spinner>` elements.

| Property      | Description                                                                                                                                                                                         |
| ------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `"draggable"` | Whether the value can be changed by dragging the buttons.<br>See [`juce::Slider::setIncDecButtonsMode()`](https://docs.juce.com/master/classjuce_1_1Slider.html#a6273cd5aa658bf69c22f1c32d4a99efb). |

### Progress Bars

The following properties apply only to `<ProgressBar>` elements.

Progress bars' ranges are always normalised from `0` to `1`, and are always linear.

| Property  | Description                                |
| --------- | ------------------------------------------ |
| `"value"` | The normalised progress, from `0` to `1`.. |

### Images

The following properties apply only to `<Image>` and `<svg>` elements.

| Property      | Description                                                                                                                                                                                                                                                                                                                          |
| ------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| `"placement"` | How the image is fitted into the element's bounds. Any of `left`, `right`, `centred-x`, `top`, `bottom`, `centred-y`, `stretch`, `fill`, `reduce-only`, `increase-only`, `do-not-resize`, and `centred`, separated by spaces.<br>See [`juce::RectanglePlacement`](https://docs.juce.com/master/classjuce_1_1RectanglePlacement.html) |
| `"source"`    | The path of the image file to draw (SVG, PNG, JPEG or GIF), found the same way as other [sources](#common).                                                                                                                                                                                                                          |
