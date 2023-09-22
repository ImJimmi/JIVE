# JIVE Style Sheets

- [JIVE Style Sheets](#jive-style-sheets)
    - [Properties](#properties)
        - [`background`](#background)
        - [`foreground`](#foreground)
        - [`border`](#border)
        - [`border-radius`](#border-radius)
        - [`font-family`](#font-family)
        - [`font-size`](#font-size)
        - [`font-stretch`](#font-stretch)
        - [`font-style`](#font-style)
        - [`font-weight`](#font-weight)
        - [`letter-spacing`](#letter-spacing)
        - [`text-decoration`](#text-decoration)
    - [Selectors](#selectors)
        - [Interaction States](#interaction-states)
            - [`hover`](#hover)
            - [`active`](#active)
            - [`focus`](#focus)
            - [`disabled`](#disabled)
            - [Example](#example)
        - [IDs](#ids)
        - [Classes](#classes)
        - [Child Components](#child-components)


The `jive_style_sheets` module provides tools for styling components using common CSS-like properties defined in JSON documents.

A `jive::StyleSheet` can be attached to a `juce::Component` along with a corresponding `juce::ValueTree` element to apply any styles from the element's `style` property to the component. Style sheets are hereditary, meaning that if a style property isn't explicitly defined for a component, the style sheet will inherit the style from the component's parent instead.

The `style` property can be set using a JSON-formatted string, for example:

```cpp
juce::Component component;
juce::ValueTree tree{ "Component" };

tree.setProperty("style",
                 R"({
                    "background": "#7F007F",
                    "foreground": "#7FFF7F",
                 })",
                 nullptr);

jive::StyleSheet styleSheet{ component, tree };
```

This approach however will not allow the style to be easily modified at runtime (without some sort of string-builder), and will be constantly parsed to a `jive::Object` internally. Therefore it's a lot more useful to instead set the `style` property using a `jive::Object` directly.

`jive::Object` derives from `juce::DynamicObject` but adds a useful listener mechanism allowing clients to get callbacks when a property is changed in the object. They can be constructed by parsing a JSON string:

```cpp
juce::Component component;
juce::ValueTree tree{ "Component" };
auto styleValue = jive::parseJSON(R"({
    "background": "#7F007F",
    "foreground": "#7FFF7F",
})");

tree.setProperty("style", style, nullptr);
jive::StyleSheet styleSheet{ component, tree };

// Note that jive::parseJSON() returns a juce::var.
// To get the underlying jive::Object, simply use a dynamic_cast:
if (auto* styleObject = dynamic_cast<jive::Object>(tree["style"].getDynamicObject()))
    styleObject->addListener(this);
```

## Properties

### `background`

The component's background style. Can be either a solid colour or a gradient:

```json
{
    // Hex colour
    "background": "#00FF00",

    // RGB colour
    "background": "rgb(255, 0, 0)",

    // RGBA colour
    "background": "rgba(255, 0, 0, 0.25)",

    // HSL colour
    "background": "hsl(240, 100%, 50%)",

    // Named colour from juce::Colours namespace
    "background": "mediumorchid",

    // Vertical linear gradient
    "background": {
        "stops": {
            "0.0": "hsl(300, 100%, 80%)",
            "0.3": "#FF00FF",
            "1.0": "rgba(255, 0, 255, 0.5)"
        },
        "gradient": "linear", // optional
        "orientation": "vertical", // optional
    },

    // Horizontal linear gradient
    "background": {
        "orientation": "horizontal",
        "stops": {...},
    },

    // Radial gradient
    // (Centred, ending in top-right corner)
    "background": {
        "gradient": "radial",
        "start": "0.5, 0.5", // Normalised coordinates
        "end": "1.0, 0.0",
    }
}
```

### `foreground`

The component's foreground colour - applied to text and icons. Can be either solid colour or gradient (see `background` for examples).

### `border`

The component's border colour. Can be either a solid colour of gradient (see `background` for examples).

Note that the border thickness is specified as a separate property on the `juce::ValueTree`, _not_ in the style sheet. This is because it's also used by `jive_layouts`.

### `border-radius`

The radius of the component's corners.

```json
{
    // Same radius for all-four corners
    "border-radius": 5,

    // First value applied to top-left and bottom-right
    // Second value applied to bottom-left and top-right
    // (Blame CSS for this madness)
    "border-radius": "3, 7",

    // First value applied to top-left
    // Second value applied to bottom-left and top-right
    // Third value applied to bottom-right
    "border-radius": "1, 4, 9",

    // Each corner with a different value
    "border-radius": "2, 4, 6, 7",
}
```

### `font-family`

The font family to use, e.g. Helvetica, Arial, etc.

### `font-size`

The **point-height** to use for the font.

### `font-stretch`

The horizontal scale of the font, with `1.0` being the default.

### `font-style`

The style to use for the font, either `"normal"` or `"italic"`.

### `font-weight`

The weight to use for the font, either `"normal"` or `"bold`".

### `letter-spacing`

The amount of extra space to leave between characters, in pixels.

### `text-decoration`

The decoration to use for the font, either `"normal"` or `"underlined"`.

## Selectors

As well as the above properties, style sheets can also use _selectors_ to apply styles to child components and/or different interaction states.

### Interaction States

#### `hover`

Styles in the `hover` object will be applied when the mouse is hovering over the component.

#### `active`

Styles in the `active` object will be applied when the mouse is held down on a component, or when it is dragged.

#### `focus`

Styles in the `focus` object will be applied when the component has keyboard focus.

#### `disabled`

Styles in the `disabled` object will be applied when the component is not enabled.

#### Example

```json
{
    "background": "green",

    "hover": {
        "background": "lime",
    },
    "active": {
        "background": "darkgreen",
    },
    "focus": {
        "border": "yellow",
    },
    "disabled": {
        "background": "grey",
    }
}
```

### IDs

Styles can be applied to elements with a specified `"id"` property. The style sheet must append a `"#"` to the ID of the element it's targetting. For example:

```xml
<Button id="save-button"></Button>
```

```json
{
    "#save-button": {
        "foreground": "blue"
    }
}
```

### Classes

Styles can be applied to elements according to one or more tokens from the element's `"class"` property. The style sheet must append a `"."` to the class name it's targetting. For example:

```xml
<Button class="button-primary bg-red"></Button>
```

```json
{
    ".button-primary": {
        "border-radius": 10,
        "font-weight": "bold",
    },
    ".bg-red": {
        "background": "red",
    }
}
```

Note that if two or more classes define the same property, the element will take the value of the _first_ element in its `"class"` list. For example:

```json
{
    ".foo": {
        "background": "red"
    },
    ".bar": {
        "background": "green"
    }
}
```

This element will have a red background:

```xml
<Component class="foo bar"></Component>
```

This element will have a green background:

```xml
<Component class="bar foo"></Component>
```

### Child Components

If a property in the object matches the type name of any child elements of the `juce::ValueTree` on which the object is assigned to the `style` property, the styles defined in that property will be applied to those children.

```json
{
    "background": "darkgrey", // Window background

    "Button": {
        "background": "blue", // Make all buttons blue
    },
    "Knob": {
        "border": "yellow",
    },
    "MyCustomGroup": {
        // Make all <MyCustomWidget/> components within <MyCustomGroup/> components have pink text.
        "MyCustomWidget": {
            "foreground": "pink",
        },
    },
}
```
