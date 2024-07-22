# JIVE Layouts

- [JIVE Layouts](#jive-layouts)
    - [The Interpreter](#the-interpreter)
    - [GUI Items](#gui-items)
        - [Properties](#properties)
            - [Common](#common)
            - [Block Items](#block-items)
            - [Buttons](#buttons)
            - [Combo Boxes](#combo-boxes)
            - [Flex Containers](#flex-containers)
            - [Flex Items](#flex-items)
            - [Grid Containers](#grid-containers)
            - [Grid Items](#grid-items)
            - [Hyperlinks](#hyperlinks)
            - [Images](#images)
            - [Progress Bars](#progress-bars)
            - [Sliders](#sliders)
            - [Spinners](#spinners)
            - [Text](#text)
            - [Windows](#windows)

The `jive_layouts` module provides tools for creating GUIs by describing them using declarative markup in the form of `juce::ValueTree`s.

## The Interpreter

The interpreter encapsulates the required logic to properly construct the required hierarchy of `jive::GuiItem`s from a given `juce::ValueTree`.

The interpreter can be customised in a few ways:

- **Custom Components** - by adding entries to the interpreter's `jive::ComponentFactory`, it can be made to construct custom component types for a given `juce::ValueTree` type name.
- **Custom GUI Items** - the interpreter will decorate items wrapping `juce::ValueTree` elements of the specified type with the specified decorators.
- **Aliases** - when the interpreter encounters an element in the given `juce::ValueTree` with the name of one of its aliases, it will replace it with a replacement `juce::ValueTree`. This can be useful for reusing complex elements, for example:

    ```xml
    <SearchGroup/>

    <!-- expands to... -->

    <Component id="search-group">
        <Text id="search-label" margin="0 0 5 0">Search:</Text>

        <Component display="flex" flex-direction="row">
            <TextBox id="search-box" flex-grow="1"></TextBox>
            <Button id="search-submit"><Text>Go</Text></Button>
        </Component>
    </Component>
    ```

## GUI Items

The core of JIVE Layouts is the `jive::GuiItem` class which wraps a `juce::Component` and applies the required properties from the corresponding `juce::ValueTree`.

`jive::GuiItem` itself is a base class from which other classes should derive to implement more specific behaviour for a particular type of component. These derived types are applied using the Decorator Pattern meaning that they should not only _derive_ from `jive::GuiItem` (or more specifically, `jive::GuiItemDecorator`) but should also _hold_ a `jive::GuiItem` (which `jive::GuiItemDecorator` manages for you). This approach means that different behaviour can be added to a GUI item at runtime.

There are two main categories of GUI item:

- Container Items - implement a particular layout pattern such as Flex or Grid.
- Widget Items - implement the behaviour of a specific type of widget such as a button or slider.

There likely won't be much need to write custom `jive::GuiItem` derivatives, but if there is a need they should be implemented by the following rules:

- Prefer to derive from `jive::GuiItemDecorator` rather than `jive::GuiItem` itself.
- When overriding virtual methods from the base class, be sure to always call the same method on the base class so other decorators managing the same component can properly implement their behaviour.
- Don't rely on the order in which decorators are added.

_Avoid constructing GUI items manually, always prefer to use the `jive::Interpreter`._

### Properties

Changing the properties of the `juce::ValueTree` markup source will change the behaviour of the created components.

Properties can either be set directly on a `juce::ValueTree` using their stringified forms:

```cpp
window.setProperty("display", "grid", nullptr);
```

Or by using a `juce::VariantConverter` specialisation, implemented by JIVE:

```cpp
window.setProperty("cursor",
                   juce::VariantConverter<juce::MouseCursor::StandardCursorType>::toVar(juce::MouseCursor::PointingHandCursor),
                   nullptr);
```

However the best approach is to use a `jive::Property`, which is similar to `juce::Value` in that it represents a property on a `juce::ValueTree` except it's type safe, encapsulates the conversions to/from `juce::var`, and has a handy listener mechanism:

```cpp
jive::Property<jive::Display> display{ window, "display" };
display.onValueChange = [](){
    // Do something when the "display" property is changed...
};

display = jive::Display::grid;
```

The tables below each property will tell you what C++ type to use with `jive::Property` for that property.

Where possible, property names and their accepted values should match their equivalents from CSS. This helps keep a consistent terminology across teams and may even let you copy-paste properties from tools like Figma.

#### Common

The following properties will apply to all GUI items.

| Identifier               | JUCE Property                                                                                                                              | CSS Property                                                               | Type                                                            |
| ------------------------ | ------------------------------------------------------------------------------------------------------------------------------------------ | -------------------------------------------------------------------------- | --------------------------------------------------------------- |
| `"always-on-top"`        | [`juce::Component::setAlwaysOnTop()`](https://docs.juce.com/master/classComponent.html#a9ec074c0a1a25014c235a5ebafdbaaf7)                  | N/A                                                                        | `bool`                                                          |
| `"border-width"`         | N/A                                                                                                                                        | [`"border-width"`](https://www.w3schools.com/css/css_border_width.asp)     | `juce::BorderSize<float>`                                       |
| `"buffered-to-image"`    | [`juce::Component::setBufferedToImage()`](https://docs.juce.com/master/classComponent.html#af19bbc2186e3297ddd55c328e46c014b)              | N/A                                                                        | `bool`                                                          |
| `"clicking-grabs-focus"` | [`juce::Component::setMouseClickGrabsKeyboardFocus()`](https://docs.juce.com/master/classComponent.html#a5e27530ab343f52b524c1c3f1a1d98eb) | [`"cursor"`](https://www.w3schools.com/cssref/pr_class_cursor.php)         | `juce::MouseCursor::StandardCursorType`                         |
| `"description"`          | [`juce::Component::setDescription()`](https://docs.juce.com/master/classComponent.html#ad527bbb31f160269381a8a21a88509df)                  | N/A                                                                        | `juce::String`                                                  |
| `"display"`              | N/A                                                                                                                                        | [`"display"`](https://www.w3schools.com/cssref/pr_class_display.php)       | [`jive::Display`](./utilities/jive_Display.h)                   |
| `"enabled"`              | [`juce::Component::setEnabled()`](https://docs.juce.com/master/classComponent.html#a6f956f690d72256838146c7dfe99bc0e)                      | N/A                                                                        | `bool`                                                          |
| `"focus-order"`          | [`juce::Component::setExplicitFocusOrder()`](https://docs.juce.com/master/classComponent.html#acea5b1758143ff5815fd13ae57f2172e)           | N/A                                                                        | `int`                                                           |
| `"focus-outline"`        | [`juce::Component::setHasFocusOutline()`](https://docs.juce.com/master/classComponent.html#a75d4febc425c0c0ffe4567e265b45ef2)              | N/A                                                                        | `bool`                                                          |
| `"focusable"`            | [`juce::Component::setWantsKeyboardFocus()`](https://docs.juce.com/master/classComponent.html#a6a1f21a76b971d54870cb5c32c041055)           | N/A                                                                        | `bool`                                                          |
| `"height"`               | [`juce::Component::setSize()`](https://docs.juce.com/master/classComponent.html#af7e0443344448fcbbefc2b3dd985e43f)                         | N/A                                                                        | [`jive::Length`](../jive_core/geometry/jive_Length.h)           |
| `"id"`                   | [`juce::Component::setComponentID()`](https://docs.juce.com/master/classComponent.html#ac41c215031a087064add6d3a180abd1d)                  | N/A                                                                        | `juce::String`                                                  |
| `"margin"`               | N/A                                                                                                                                        | [`"margin"`](https://www.w3schools.com/css/css_margin.asp)                 | `juce::BorderSize<float>`                                       |
| `"min-height"`           | N/A                                                                                                                                        | [`"min-height"`](https://www.w3schools.com/cssref/pr_dim_min-height.php)   | [`jive::Length`](../jive_core/geometry/jive_Length.h)           |
| `"min-width"`            | N/A                                                                                                                                        | [`"min-width"`](https://www.w3schools.com/cssref/pr_dim_min-width.php)     | [`jive::Length`](../jive_core/geometry/jive_Length.h)           |
| `"name"`                 | [`juce::Component::setName()`](https://docs.juce.com/master/classComponent.html#a64d1ad9a0f8f0d1757e62ac738b36b35)                         | N/A                                                                        | `juce::String`                                                  |
| `"opacity"`              | [`juce::Component::setAlpha()`](https://docs.juce.com/master/classComponent.html#a1b9329a87c71ed01319071e0fedac128)                        | [`"opacity"`](https://www.w3schools.com/cssref/css3_pr_opacity.php)        | `float`                                                         |
| `"opaque"`               | [`juce::Component::setOpaque()`](https://docs.juce.com/master/classComponent.html#a7320d543cba40183c894474ab78798ea)                       | N/A                                                                        | `bool`                                                          |
| `"padding"`              | N/A                                                                                                                                        | [`"padding"`](https://www.w3schools.com/css/css_padding.asp)               | `juce::BorderSize<float>`                                       |
| `"title"`                | [`juce::Component::setTitle()`](https://docs.juce.com/master/classComponent.html#a288d6fd5d1baebd8bc6ed458f33451c8)                        | N/A                                                                        | `juce::String`                                                  |
| `"tooltip"`              | [`juce::Component::setHelpText()`](https://docs.juce.com/master/classComponent.html#a34ac1b9e742bc619f738b3ab4e889854)                     | N/A                                                                        | `juce::String`                                                  |
| `"transition"`           | N/A                                                                                                                                        | [`transition`](https://www.w3schools.com/css/css3_transitions.asp)         | [`jive::Transitions`](../jive_core/kinetics/jive_Transitions.h) |
| `"visible"`              | [`juce::Component::setVisible()`](https://docs.juce.com/master/classComponent.html#ac8483af6fe4dc3254e7176df0d8e9f7a)                      | [`"visibility"`](https://www.w3schools.com/cssref/pr_class_visibility.php) | `bool`                                                          |
| `"width"`                | [`juce::Component::setSize()`](https://docs.juce.com/master/classComponent.html#af7e0443344448fcbbefc2b3dd985e43f)                         | N/A                                                                        | [`jive::Length`](../jive_core/geometry/jive_Length.h)           |

#### Block Items

The following properties will only apply elements that are children of an element with a `display` type of `block`.

| Identifier   | JUCE Property                                                                                                        | CSS Property | Type                                                  |
| ------------ | -------------------------------------------------------------------------------------------------------------------- | ------------ | ----------------------------------------------------- |
| `"centre-x"` | [`juce::Component::setBounds()`](https://docs.juce.com/master/classComponent.html#a64cbeee83a441e20ed5e7acb600ae107) | N/A          | [`jive::Length`](../jive_core/geometry/jive_Length.h) |
| `"centre-y"` | [`juce::Component::setBounds()`](https://docs.juce.com/master/classComponent.html#a64cbeee83a441e20ed5e7acb600ae107) | N/A          | [`jive::Length`](../jive_core/geometry/jive_Length.h) |
| `"x"`        | [`juce::Component::setBounds()`](https://docs.juce.com/master/classComponent.html#a64cbeee83a441e20ed5e7acb600ae107) | N/A          | [`jive::Length`](../jive_core/geometry/jive_Length.h) |
| `"y"`        | [`juce::Component::setBounds()`](https://docs.juce.com/master/classComponent.html#a64cbeee83a441e20ed5e7acb600ae107) | N/A          | [`jive::Length`](../jive_core/geometry/jive_Length.h) |

#### Buttons

The following properties apply only to `<Button>` elements.

| Identifier          | JUCE Property                                                                                                                | CSS Property | Type                                                                     |
| ------------------- | ---------------------------------------------------------------------------------------------------------------------------- | ------------ | ------------------------------------------------------------------------ |
| `"on-click"`        | N/A                                                                                                                          | N/A          | [`jive::Event`](../jive_core/values/jive_Event.h)                        |
| `"radio-group"`     | [`juce::Button::setRadioGroupId()`](https://docs.juce.com/master/classButton.html#a29adebc6ed27829f75b69c6ba40ef958)         | N/A          | `int`                                                                    |
| `"toggle-on-click"` | [`juce::Button::setClickingTogglesState()`](https://docs.juce.com/master/classButton.html#a675d6848c16e265ff334b2d51328d8d2) | N/A          | `bool`                                                                   |
| `"toggleable"`      | [`juce::Button::setToggleable()`](https://docs.juce.com/master/classButton.html#a30fd613e3b4e883898474bb389899579)           | N/A          | `bool`                                                                   |
| `"toggled"`         | [`juce::Button::setToggleState()`](https://docs.juce.com/master/classButton.html#a063753c2ce6b9016456ebe353298b342)          | N/A          | `bool`                                                                   |
| `"trigger-event"`   | [`juce::Button::setTriggeredOnMouseDown()`](https://docs.juce.com/master/classButton.html#a727e042a6bf3b5f342c7fb5c5c4ce47e) | N/A          | [`jive::Button::TriggerEvent`](./layout/gui-items/widgets/jive_Button.h) |

#### Combo Boxes

The following properties apply only to `<ComboBox>` elements.

| Identifier    | JUCE Property                                                                                                                 | CSS Property | Type                                              |
| ------------- | ----------------------------------------------------------------------------------------------------------------------------- | ------------ | ------------------------------------------------- |
| `"editable"`  | [`juce::ComboBox::setEditableText()`](https://docs.juce.com/master/classComboBox.html#af11edea3da851e2e0cb231f2cffa57aa)      | N/A          | `bool`                                            |
| `"on-change"` | N/A                                                                                                                           | N/A          | [`jive::Event`](../jive_core/values/jive_Event.h) |
| `"selected"`  | [`juce::ComboBox::setSelectedItemIndex()`](https://docs.juce.com/master/classComboBox.html#ad477b02d6a2af5c14b6e7ce673ec7979) | N/A          | `int`                                             |

#### Flex Containers

The following properties will apply to any element with a `display` value of `flex`.

| Identifier          | JUCE Property                                                                                                       | CSS Property                                                                        | Type                            |
| ------------------- | ------------------------------------------------------------------------------------------------------------------- | ----------------------------------------------------------------------------------- | ------------------------------- |
| `"align-content"`   | [`juce::FlexBox::alignContent`](https://docs.juce.com/master/classFlexBox.html#a8e1ae8a873d7ff198f439d3088192bcf)   | [`"align-content"`](https://www.w3schools.com/cssref/css3_pr_align-content.php)     | `juce::FlexBox::AlignContent`   |
| `"align-items"`     | [`juce::FlexBox::alignItems`](https://docs.juce.com/master/classFlexBox.html#a9e928c10773d54e50e4bfcfc2814f541)     | [`"align-items"`](https://www.w3schools.com/cssref/css3_pr_align-items.php)         | `juce::FlexBox::AlignItems`     |
| `"flex-direction"`  | [`juce::FlexBox::direction`](https://docs.juce.com/master/classFlexBox.html#a6fff1e86d4ae97ed4a0dd5face653914)      | [`"flex-direction"`](https://www.w3schools.com/cssref/css3_pr_flex-direction.php)   | `juce::FlexBox::Direction`      |
| `"flex-wrap"`       | [`juce::FlexBox::flexWrap`](https://docs.juce.com/master/classFlexBox.html#a58870e7df132cee2eda6d7586d026303)       | [`"flex-wrap"`](https://www.w3schools.com/cssref/css3_pr_flex-wrap.php)             | `juce::FlexBox::Wrap`           |
| `"justify-content"` | [`juce::FlexBox::justifyContent`](https://docs.juce.com/master/classFlexBox.html#a20627d266f82422c5e72152ba07e5bc1) | [`"justify-content"`](https://www.w3schools.com/cssref/css3_pr_justify-content.php) | `juce::FlexBox::JustifyContent` |

#### Flex Items

The following properties will only apply to the children of components with a `display` property of `flex`.

| Identifier      | JUCE Property                                                                                                     | CSS Property                                                                | Type                        |
| --------------- | ----------------------------------------------------------------------------------------------------------------- | --------------------------------------------------------------------------- | --------------------------- |
| `"align-self"`  | [`juce::FlexItem::alignSelf`](https://docs.juce.com/master/classFlexItem.html#a7b9190e838f5ee0d70d1cac494621fd7)  | [`"align-self"`](https://www.w3schools.com/cssref/css3_pr_align-self.php)   | `juce::FlexItem::AlignSelf` |
| `"flex-basis"`  | [`juce::FlexItem::flexBasis`](https://docs.juce.com/master/classFlexItem.html#a893ec8b92e7bb576da0f8637f51686f6)  | [`"flex-basis"`](https://www.w3schools.com/cssref/css3_pr_flex-basis.php)   | `float`                     |
| `"flex-grow"`   | [`juce::FlexItem::flexGrow`](https://docs.juce.com/master/classFlexItem.html#ae2ac46ab80a5a81da22d6f4a2f0debe1)   | [`"flex-grow"`](https://www.w3schools.com/cssref/css3_pr_flex-grow.php)     | `float`                     |
| `"flex-shrink"` | [`juce::FlexItem::flexShrink`](https://docs.juce.com/master/classFlexItem.html#a67fdfe4ec6cdd882057e8ed39994fb93) | [`"flex-shrink"`](https://www.w3schools.com/cssref/css3_pr_flex-shrink.php) | `float`                     |
| `"order"`       | [`juce::FlexItem::order`](https://docs.juce.com/master/classFlexItem.html#a65a0d2e16a904472eaa9ada3cdfcbf13)      | [`"order"`](https://www.w3schools.com/cssref/css3_pr_order.php)             | `int`                       |

#### Grid Containers

The following properties will apply to any element with a `display` value of `grid`.

| Identifier                | JUCE Property                                                                                                                                                                                                   | CSS Property                                                                               | Type                                 |
| ------------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------ | ------------------------------------ |
| `"align-content"`         | [`juce::Grid::alignContent`](https://docs.juce.com/master/classGrid.html#a7b9190e838f5ee0d70d1cac494621fd7)                                                                                                     | [`"align-content"`](https://www.w3schools.com/cssref/css3_pr_align-content.php)            | `juce::Grid::AlignContent`           |
| `"align-items"`           | [`juce::Grid::alignItems`](https://docs.juce.com/master/classGrid.html#a1ea337d65d875793edcfd4b195040f39)                                                                                                       | [`"align-items"`](https://www.w3schools.com/cssref/css3_pr_align-items.php)                | `juce::Grid::AlignItems`             |
| `"gap"`                   | [`juce::Grid::columnGap`](https://docs.juce.com/master/classGrid.html#a79c7bfbb5bab511a2b950f8f4d33ba5c), [`juce::Grid::rowGap`](https://docs.juce.com/master/classGrid.html#aa23bb39ce1bf833161597a8ee5076771) | [`"gap"`](https://www.w3schools.com/cssref/css3_pr_gap.php)                                | `juce::Grid::Px`                     |
| `"grid-auto-columns"`     | [`juce::Grid::autoColumns`](https://docs.juce.com/master/classGrid.html#a5bd50b3f91dfebb6ff06a39d2b3a3744)                                                                                                      | [`"grid-auto-columns"`](https://www.w3schools.com/cssref/pr_grid-auto-columns.php)         | `juce::Grid::TrackInfo`              |
| `"grid-auto-flow"`        | [`juce::Grid::autoFlow`](https://docs.juce.com/master/classGrid.html#a9281a2b5ca845deb6bd2691c8449b4c7)                                                                                                         | [`"grid-auto-flow"`](https://www.w3schools.com/cssref/pr_grid-auto-flow.php)               | `juce::Grid::AutoFlow`               |
| `"grid-auto-rows"`        | [`juce::Grid::autoRows`](https://docs.juce.com/master/classGrid.html#a79c7bfbb5bab511a2b950f8f4d33ba5c)                                                                                                         | [`"grid-auto-rows"`](https://www.w3schools.com/cssref/pr_grid-auto-rows.php)               | `juce::Grid::TrackInfo`              |
| `"grid-template-areas"`   | [`juce::Grid::templateAreas`](https://docs.juce.com/master/classGrid.html#afae4829bdb6a35f1c59c79fbca5f8e26)                                                                                                    | [`"grid-template-areas"`](https://www.w3schools.com/cssref/pr_grid-template-areas.php)     | `juce::StringArray`                  |
| `"grid-template-columns"` | [`juce::Grid::templateColumns`](https://docs.juce.com/master/classGrid.html#ae4093236e35159e4f2b9e8c36d688d79)                                                                                                  | [`"grid-template-columns"`](https://www.w3schools.com/cssref/pr_grid-template-columns.php) | `juce::Array<juce::Grid::TrackInfo>` |
| `"grid-template-rows"`    | [`juce::Grid::templateRows`](https://docs.juce.com/master/classGrid.html#a4b64ee32653f572098afc74ec0cacd71)                                                                                                     | [`"grid-template-rows"`](https://www.w3schools.com/cssref/pr_grid-template-rows.php)       | `juce::Array<juce::Grid::TrackInfo>` |
| `"justify-content"`       | [`juce::Grid::justifyContent`](https://docs.juce.com/master/classGrid.html#a1ffb17b0278671325d144c18ad3d90d8)                                                                                                   | [`"justify-content"`](https://www.w3schools.com/cssref/css3_pr_justify-content.php)        | `juce::Grid::JustifyContent`         |
| `"justify-items"`         | [`juce::Grid::justifyItems`](https://docs.juce.com/master/classGrid.html#afb7f619f54e013d739bf4eba008c6f8b)                                                                                                     | [`"justify-items"`](https://www.w3schools.com/cssref/css_pr_justify-items.php)             | `juce::Grid::JustifyItems`           |

#### Grid Items

The following properties will only apply to the children of components with a `display` property of `flex`.

| Identifier       | JUCE Property                                                                                                      | CSS Property                                                                 | Type                                  |
| ---------------- | ------------------------------------------------------------------------------------------------------------------ | ---------------------------------------------------------------------------- | ------------------------------------- |
| `"align-self"`   | [`juce::GridItem::alignSelf`](https://docs.juce.com/master/classGridItem.html#a8c48a9ecf979e0b9493e12ecc629a916)   | [`"align-self"`](https://www.w3schools.com/cssref/css3_pr_align-self.php)    | `juce::GridItem::AlignSelf`           |
| `"grid-area"`    | [`juce::GridItem::area`](https://docs.juce.com/master/classGridItem.html#aa1d3cb3665477cdb7b58a607b779f8f3)        | [`"grid-area"`](https://www.w3schools.com/cssref/pr_grid-area.php)           | `juce::String`                        |
| `"grid-column"`  | [`juce::GridItem::column`](https://docs.juce.com/master/classGridItem.html#a4009d56282795631940ec981f856255f)      | [`"grid-column"`](https://www.w3schools.com/cssref/pr_grid-column.php)       | `juce::GridItem::StartAndEndProperty` |
| `"grid-row"`     | [`juce::GridItem::row`](https://docs.juce.com/master/classGridItem.html#a7cb916f4a5cdc7da46727f54e5af89f8)         | [`"grid-row"`](https://www.w3schools.com/cssref/pr_grid-row.php)             | `juce::GridItem::StartAndEndProperty` |
| `"justify-self"` | [`juce::GridItem::justifySelf`](https://docs.juce.com/master/classGridItem.html#ad99b06aa55356196c54bf40321301147) | [`"justify-self"`](https://www.w3schools.com/cssref/css_pr_justify-self.php) | `juce::GridItem::JustifySelf`         |
| `"max-height"`   | [`juce::GridItem::maxHeight`](https://docs.juce.com/master/classGridItem.html#aebe1c6bf8c616570acf4021b32d9572a)   | [`"max-height"`](https://www.w3schools.com/cssref/pr_dim_max-height.php)     | `float`                               |
| `"max-width"`    | [`juce::GridItem::maxWidth`](https://docs.juce.com/master/classGridItem.html#aa9141fbfdf1ca5bb5246cdc144095202)    | [`"max-width"`](https://www.w3schools.com/cssref/pr_dim_max-width.php)       | `float`                               |
| `"order"`        | [`juce::GridItem::order`](https://docs.juce.com/master/classGridItem.html#a4c5e79aea3733b6ad55efbe986b3e32c)       | [`"order"`](https://www.w3schools.com/cssref/css3_pr_order.php)              | `int`                                 |

#### Hyperlinks

The following properties apply only to `<Hyperlink>` elements.

| Identifier | JUCE Property                                                                                                                 | CSS Property | Type        |
| ---------- | ----------------------------------------------------------------------------------------------------------------------------- | ------------ | ----------- |
| `"url"`    | [`juce::HyperlinkButton::setURL()`](https://docs.juce.com/master/classHyperlinkButton.html#af822610fabccf3a135936de178b6b1ac) | N/A          | `juce::URL` |

#### Images

The following properties apply only to `<Image>` elements.

| Identifier    | JUCE Property                                                                                                                          | CSS Property | Type                                                          |
| ------------- | -------------------------------------------------------------------------------------------------------------------------------------- | ------------ | ------------------------------------------------------------- |
| `"placement"` | [`juce::ImageComponent::setImagePlacement()`](https://docs.juce.com/master/classImageComponent.html#a3ae8afb2d7fae2d7e0f8728e3be4cfd5) | N/A          | `juce::RectanglePlacement`                                    |
| `"source"`    | N/A                                                                                                                                    | N/A          | [`jive::Drawable`](../jive_layouts/utilities/jive_Drawable.h) |

#### Progress Bars

The following properties apply only to `<ProgressBar>` elements.

| Identifier | JUCE Property | CSS Property | Type     |
| ---------- | ------------- | ------------ | -------- |
| `"value"`  | N/A           | N/A          | `double` |

#### Sliders

The following properties apply only to `<Knob>`, `<Slider>`, and `<Spinner>` elements.

| Identifier               | JUCE Property                                                                                                                                 | CSS Property | Type                                                            |
| ------------------------ | --------------------------------------------------------------------------------------------------------------------------------------------- | ------------ | --------------------------------------------------------------- |
| `"interval"`             | [`juce::NormalisableRange<>::interval`](https://docs.juce.com/master/classNormalisableRange.html#a7717c3a5ebfe5ac44ab7d55ad63e5354)           | N/A          | `juce::String`                                                  |
| `"max"`                  | [`juce::NormalisableRange<>::end`](https://docs.juce.com/master/classNormalisableRange.html#aa8aba814e8121157ba79ccbf03152b82)                | N/A          | `juce::String`                                                  |
| `"mid"`                  | [`juce::NormalisableRange<>::setSkewForCentre()`](https://docs.juce.com/master/classNormalisableRange.html#a3a583db475e34e251edef004a4e3e3d6) | N/A          | `juce::String`                                                  |
| `"min"`                  | [`juce::NormalisableRange<>::start`](https://docs.juce.com/master/classNormalisableRange.html#adc186176e94c6f80eb2c626384054fe3)              | N/A          | `juce::String`                                                  |
| `"on-change"`            | N/A                                                                                                                                           | N/A          | [`jive::Event`](../jive_core/values/jive_Event.h)               |
| `"orientation"`          | [`juce::Slider::setSliderStyle()`](https://docs.juce.com/master/classSlider.html#a6b6917dd3753c7552778977733f0b9ef)                           | N/A          | [`jive::Orientation`](../jive_core/geometry/jive_Orientation.h) |
| `"sensitivity"`          | [`juce::Slider::setMouseDragSensitivity()`](https://docs.juce.com/master/classSlider.html#a4bdd8cdb79dcdb06b1e97a21ff2a6c4b)                  | N/A          | `double`                                                        |
| `"snap-to-mouse"`        | [`juce::Slider::setSliderSnapsToMousePosition()`](https://docs.juce.com/master/classSlider.html#acf58b7346a74cfc0a8c809567836df61)            | N/A          | `bool`                                                          |
| `"velocity-mode"`        | [`juce::Slider::setVelocityBasedMode()`](https://docs.juce.com/master/classSlider.html#a59b616e79738d3adb83093dc532822d0)                     | N/A          | `bool`                                                          |
| `"velocity-offset"`      | [`juce::Slider::setVelocityModeParameters()`](https://docs.juce.com/master/classSlider.html#a47c37989ff5f6453f2c44f1a7455e1c1)                | N/A          | `double`                                                        |
| `"velocity-sensitivity"` | [`juce::Slider::setVelocityModeParameters()`](https://docs.juce.com/master/classSlider.html#a47c37989ff5f6453f2c44f1a7455e1c1)                | N/A          | `double`                                                        |
| `"velocity-threshold"`   | [`juce::Slider::setVelocityModeParameters()`](https://docs.juce.com/master/classSlider.html#a47c37989ff5f6453f2c44f1a7455e1c1)                | N/A          | `int`                                                           |

#### Spinners

The following properties apply only to `<Spinner>` elements.

| Identifier    | JUCE Property                                                                                                             | CSS Property | Type   |
| ------------- | ------------------------------------------------------------------------------------------------------------------------- | ------------ | ------ |
| `"draggable"` | [`juce::Slider::setIncDecButtonsMode()`](https://docs.juce.com/master/classSlider.html#a188fa57db0d90f931529dc367e76b9f5) | N/A          | `bool` |

#### Text

The following properties apply only to `<Text>` elements.

| Identifier        | JUCE Property                                                                                                                                | CSS Property                                                            | Type                                       |
| ----------------- | -------------------------------------------------------------------------------------------------------------------------------------------- | ----------------------------------------------------------------------- | ------------------------------------------ |
| `"direction"`     | [`juce::AttributedString::setReadingDirection()`](https://docs.juce.com/master/classAttributedString.html#a96ba8ec9194f937c568bcaa990c1ca55) | [`"direction"`](https://www.w3schools.com/cssref/pr_text_direction.php) | `juce::AttributedString::ReadingDirection` |
| `"justification"` | [`juce::AttributedString::setJustification()`](https://docs.juce.com/master/classAttributedString.html#af23239716a8ccd36a6e25315afcce470)    | N/A                                                                     | `juce::Justification`                      |
| `"line-spacing"`  | [`juce::AttributedString::setLineSpacing()`](https://docs.juce.com/master/classAttributedString.html#a0506d7b2000aaebd5873ea23eca6ae6a)      | N/A                                                                     | `float`                                    |
| `"text"`          | [`juce::AttributedString::setText()`](https://docs.juce.com/master/classAttributedString.html#a998c643b1311fa75b80e93d4d02a012b)             | N/A                                                                     | `juce::String`                             |
| `"word-wrap"`     | [`juce::AttributedString::setWordWrap()`](https://docs.juce.com/master/classAttributedString.html#ab986ab6a8a0a0aff7d7701c5b6c25572)         | [`"word-wrap"`](https://www.w3schools.com/cssref/css3_pr_word-wrap.php) | `juce::AttributedString::WordWrap`         |

#### Windows

The following properties apply only to `<Window>` elements.

| Identifier            | JUCE Property                                                                                                                                   | CSS Property | Type                                    |
| --------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------- | ------------ | --------------------------------------- |
| `"corner-resizer"`    | [`juce::ResizableWindow::setResizable()`](https://docs.juce.com/master/classResizableWindow.html#a538709f0111ad32b05d858a511bcd5be)             | N/A          | `bool`                                  |
| `"draggable"`         | [`juce::ResizableWindow::setDraggable`](https://docs.juce.com/master/classResizableWindow.html#adede3d77c744014ab574422978de16f2)               | N/A          | `bool`                                  |
| `"full-screen"`       | [`juce::ResizableWindow::setFullScreen()`](https://docs.juce.com/master/classResizableWindow.html#a08a0f1863417c80e6494d113a5c1fbd2)            | N/A          | `bool`                                  |
| `"minimised"`         | [`juce::ResizableWindow::setMinimised()`](https://docs.juce.com/master/classResizableWindow.html#a8b3174e4cad2a85ff1405a0c22e12148)             | N/A          | `bool`                                  |
| `"native"`            | [`juce::TopLevelWindow::setUsingNativeTitleBar()`](https://docs.juce.com/master/classTopLevelWindow.html#a749fbd5e688ed8c9af3d0d99b21e18c8)     | N/A          | `bool`                                  |
| `"resizable"`         | [`juce::ResizableWindow::setResizable()`](https://docs.juce.com/master/classResizableWindow.html#a538709f0111ad32b05d858a511bcd5be)             | N/A          | `bool`                                  |
| `"shadow"`            | [`juce::TopLevelWindow::setDropShadowEnabled()`](https://docs.juce.com/master/classTopLevelWindow.html#aed7017ec50e075cca022b5c11ce5fdff)       | N/A          | `bool`                                  |
| `"title-bar-buttons"` | [`juce::DocumentWindow::setTitleBarButtonsRequired()`](https://docs.juce.com/master/classDocumentWindow.html#af9549ecc64c2b7e91161a49c44b6ae17) | N/A          | `juce::DocumentWindow::TitleBarButtons` |
| `"title-bar-height"`  | [`juce::DocumentWindow::setTitleBarHeight()`](https://docs.juce.com/master/classDocumentWindow.html#a02567fc3120d918ac5d7929813c1a56b)          | N/A          | `float`                                 |
