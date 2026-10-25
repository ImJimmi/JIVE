# JIVE Style Sheets

The `jive_style_sheets` module styles JIVE GUIs with JSON style sheets, using CSS-like selectors and properties.

Every `jive::GuiItem` gets a `jive::StyleSheet`, which reads the item's `style` property and applies its rules to the corresponding component.

The `style` property is the path of a JSON file:

```xml
<Window style="public/styles/Styles.json">
    <!-- ... -->
</Window>
```

Paths starting with `./` or `../` are relative to the file the element is declared in. Any other path is relative to the interpreter's source directories. Style sheet files on disk are hot-reloaded whenever they change.

Once a file has been loaded, the `style` property holds its parsed `jive::Object`. Changing that object's properties from C++ updates the styles straight away, which is useful for things like changing a token at runtime:

```cpp
jive::Property<jive::Object::ReferenceCountedPointer> style{
  root.state,
  "style",
};

if (auto sheet = style.get())
    sheet->setProperty("background", "#E77674");
```

## Rules

The properties at the top level of a style sheet apply to the component that declares it.

```jsonc
{
  "background" : "blue",
  "font-family": "Comic Sans",
  "font-size": 14,
}
```

Selector properties can also be specified to add rules for other components that match the selector, such as components of a specific type, components with a given ID, or components with a given `class` property. Selectors can be combined in a single property name, or by adding nested selectors.

```jsonc
{
  // Rules for all Button components
  "Button": {
    "foreground": "black",

    // Nested rule for all Buttons that have the "primary" class.
    ".primary": {
      "background": "green"
    }
  },

  // Combined selector to add rules for all components with BOTH
  // "primary" and "large" classes.
  ".primary.large": {
    "font-size": 16
  },

  // ID selector to add rules for a specific element
  "#save-button": {
    "border": "yellow"
  }
}
```

Style sheets can also have pseudo classes to add rules for elements in specific states, such as when they're hovered by the mouse, or focused by the keyboard. Pseudo classes can also be combined, or nested as with other selectors.

```jsonc
{
  "background": "blue",

  // Add rules for when the root component is hovered.
  ":hover": {
    "background": "skyblue"
  },

  "Button": {
    "background": "red",

    // Nested selector to add rules for Buttons that have been toggled-on
    ":checked": {
      "background": "green"
    }
  },

  // Combined selector to add rules for sliders that have keyboard focus
  "Slider:focus": {
    "border": "yellow"
  }
}
```

_Selectors only match the root component and its descendants._

### Selectors

| Selector     | Example                                                | Matches                                                                                            |
| ------------ | ------------------------------------------------------ | -------------------------------------------------------------------------------------------------- |
| Universal    | `*`                                                    | Every component.                                                                                   |
| Type         | `Button`                                               | Components of the given type (see below).                                                          |
| Class        | `.toggle`                                              | Elements with the class in their `class` property.                                                 |
| ID           | `#title`                                               | The element with the given `id`.                                                                   |
| Pseudo-class | `:hover`, `:active`, `:focus`, `:disabled`, `:checked` | Components in the given interaction state.                                                         |
| Compound     | `Checkbox:checked`, `.heading.large`, `:hover:active`  | Components that match every part.                                                                  |
| Descendant   | `#toolbar Button`                                      | Components matching the second selector that are descendants of components matching the first.     |
| Child        | `#toolbar > Button`                                    | Components matching the second selector that are direct children of components matching the first. |
| List         | `Button, ComboBox`                                     | Components matching any of the selectors.                                                          |

#### Type selectors

Type selectors match the type of the underlying JUCE component, not the element's name. The built-in types are:

`Button`, `TextButton`, `Checkbox`, `ComboBox`, `Hyperlink`, `Label`, `ProgressBar`, `TextEditor`, `ScrollBar`, `Slider`, `Knob`, `Spinner`, `Popup`, `Option`, `Window`, and `Editor`.

`Button` matches every kind of button, including checkboxes and hyperlinks, while `TextButton` matches only `<Button>` elements. `Slider` doesn't match knobs or spinners. `Popup` and `Option` match a combo box's popup menu and its items.

To select a custom component by type, register a predicate for it:

```cpp
jive::ComponentTypePredicates::addPredicate([](const juce::Component& component, const juce::String& typeName) {
    return typeName == "XYPad" && dynamic_cast<const XYPad*>(&component) != nullptr;
});
```

### Precedence

When more than one rule sets the same property on a component, the most specific selector wins, as in CSS: IDs beat classes and pseudo-classes, which beat types. Properties at the top level of a style sheet beat any selector.

## Tokens

Top-level properties whose names start with `$` are tokens. A token can be referenced in any property value in the same style sheet, including as part of a longer value:

```jsonc
{
  "$accent": "#4FADE4",
  "$radius": 6,

  "Button, ComboBox": {
    "border-radius": "$radius",
  },
  ".toggle:checked": {
    "background": "$accent",
    "shadow": "0 0 12px $accent",
  },
}
```

## Themes

The `theme` property of a `<Window>` or `<Editor>` picks a set of built-in styles: `juce`, `steel`, `material`, `synthwave`, or `high-contrast`. The default theme is `steel`.

A theme's styles never take precedence over the rules in a style sheet, so a style sheet only has to set what it wants to change.

## Properties

Where possible, property names and their accepted values match their equivalents from CSS.

Inherited properties apply to the descendants of any component they're set on, unless a descendant sets its own value. Font styles are combined with inherited ones rather than replacing them.

| Property                  | Description                                                                                                                                                                                                                                                                                                                                                                                                                                                                        |
| ------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `"accent"`                | The colour of a checkbox's tick.<br>Can be a colour, or a gradient (see [Fills](#fills) below).                                                                                                                                                                                                                                                                                                                                                                                    |
| `"background"`            | The component's background fill.<br>Can be a colour, or a gradient (see [Fills](#fills) below).                                                                                                                                                                                                                                                                                                                                                                                    |
| `"border"`                | The component's border fill.<br>Can be a colour, or a gradient (see [Fills](#fills) below).                                                                                                                                                                                                                                                                                                                                                                                        |
| `"border-radius"`         | The radius of the component's rounded corners.<br>See CSS [`border-radius`](https://developer.mozilla.org/en-US/docs/Web/CSS/Reference/Properties/border-radius) for supported values.                                                                                                                                                                                                                                                                                             |
| `"border-width"`          | The width of the component's border.<br>See CSS [`border-width`](https://developer.mozilla.org/en-US/docs/Web/CSS/Reference/Properties/border-width) for supported values.<br>_Can also be specified as a property on GUI items._                                                                                                                                                                                                                                                  |
| `"direction"`             | The reading direction of text.<br>See CSS [`direction`](https://developer.mozilla.org/en-US/docs/Web/CSS/Reference/Properties/direction) for supported values.                                                                                                                                                                                                                                                                                                                     |
| `"fill"`                  | The fill colour of SVG shapes.<br>JIVE replaces the fill property of any matching SVG elements that have `fill="currentColor"` with the value specified by this property.<br>See CSS [`fill`](https://developer.mozilla.org/en-US/docs/Web/CSS/Reference/Properties/fill) for supported values.<br>_Components will inherit this value from their parents if no rule matches them directly._                                                                                       |
| `"font-ascent-override"`  | Overrides the font's ascent, as a proportion of its height.<br>**Requires JUCE 8 or above.**<br>_Components will inherit this value from their parents if no rule matches them directly._                                                                                                                                                                                                                                                                                          |
| `"font-descent-override"` | Overrides the font's descent, as a proportion of its height.<br>**Requires JUCE 8 or above.**<br>_Components will inherit this value from their parents if no rule matches them directly._                                                                                                                                                                                                                                                                                         |
| `"font-family"`           | The name of the font to use.<br>JUCE requires any non-standard fonts to be registered with [`juce::Typeface::createSystemTypefaceFor()`](https://docs.juce.com/master/classjuce_1_1Typeface.html#a24b3cafbfaca34528b6c366dce5e7ba7) and the returned value to be kept in-scope while the font is in use. See the [05-widgets example](../examples/05-widgets/) for a working example.<br>_Components will inherit this value from their parents if no rule matches them directly._ |
| `"font-kerning"`          | Extra spacing between characters, as a proportion of the font's height.<br>_Components will inherit this value from their parents if no rule matches them directly._                                                                                                                                                                                                                                                                                                               |
| `"font-scale"`            | The font's horizontal scale.<br>_Components will inherit this value from their parents if no rule matches them directly._                                                                                                                                                                                                                                                                                                                                                          |
| `"font-size"`             | The font's point size.<br>_Components will inherit this value from their parents if no rule matches them directly._                                                                                                                                                                                                                                                                                                                                                                |
| `"font-styles"`           | Any of `bold`, `italic`, and `underlined`, separated by spaces.<br>_Components will inherit this value from their parents if no rule matches them directly._                                                                                                                                                                                                                                                                                                                       |
| `"foreground"`            | The colour of text, and of a combo box's arrow.<br>See [Fills](#fills) for supported values.<br>_Components will inherit this value from their parents if no rule matches them directly._                                                                                                                                                                                                                                                                                          |
| `"icon"`                  | A combo box's arrow, as SVG markup or the path of an SVG file.                                                                                                                                                                                                                                                                                                                                                                                                                     |
| `"line-spacing"`          | Extra spacing between lines of text.<br>_Components will inherit this value from their parents if no rule matches them directly._<br>_Can also be specified as a property on GUI items._                                                                                                                                                                                                                                                                                           |
| `"shadow"`                | The component's drop shadow.<br>See CSS [`box-shadow`](https://developer.mozilla.org/en-US/docs/Web/CSS/Reference/Properties/box-shadow) for supported values.                                                                                                                                                                                                                                                                                                                     |
| `"stroke"`                | The stroke colour of SVG shapes.<br>JIVE replaces the stroke property of any matching SVG elements that have `stroke="currentColor"` with the value specified by this property.<br>See CSS [`stroke`](https://developer.mozilla.org/en-US/docs/Web/CSS/Reference/Properties/fill) for supported values.<br>_Components will inherit this value from their parents if no rule matches them directly._                                                                               |
| `"text-align"`            | The justification of text.<br>One of `centred-left`, `top-left`, `centred-top`, `top-right`, `centred-right`, `bottom-right`, `centred-bottom`, `bottom-left`, or `centred`.                                                                                                                                                                                                                                                                                                       |
| `"thumb"`                 | The thumb colour of sliders, knobs, or progress bars.<br>See [Fills](#fills) for supported values.                                                                                                                                                                                                                                                                                                                                                                                 |
| `"track"`                 | The track colour of sliders, knobs, or progress bars.<br>See [Fills](#fills) for supported values.                                                                                                                                                                                                                                                                                                                                                                                 |
| `"transition"`            | The rules for how a property transitions from one value to another.<br>**Only read at the top level of a style sheet.**<br>See CSS [`transition`](https://www.w3schools.com/css/css3_transitions.asp) for supported values. _Components will inherit this value from their parents if no rule matches them directly._                                                                                                                                                              |

### Fills

JUCE supports both solid-colour and gradient fills, using CSS syntax.

#### Colours

Colours can be one of:

- **Named colour**<br>`skyblue`, `hotpink`, `darkgoldenrod`<br>See [`juce::Colours` namespace](https://docs.juce.com/master/namespacejuce_1_1Colours.html)
- **Hex colour** with 3, 4, 6, or 8 values<br>`#BC9`, `#7D3B`, `#931F1D`, `#C45BAA2F`<br>See [CSS `<hex-color>`](https://developer.mozilla.org/en-US/docs/Web/CSS/Reference/Values/hex-color)
- **Colour function**<br>`rgb()`, `rgba()`, `hsl()`, or `hsla()`<br>See [CSS `rgb()`](https://developer.mozilla.org/en-US/docs/Web/CSS/Reference/Values/color_value/rgb) and [CSS `hsl()`](https://developer.mozilla.org/en-US/docs/Web/CSS/Reference/Values/color_value/hsl) for supported syntax

#### Gradients

Gradients can be one of:

- **Linear Gradient**<br>`linear-gradient(#f69d3c, #3f87a6)`, `linear-gradient(137deg, #333, #333 50%, #eee 75%, #333 75%)`<br>See [CSS `linear-gradient()`](https://developer.mozilla.org/en-US/docs/Web/CSS/Reference/Values/gradient/linear-gradient) for supported syntax
- **Radial Gradient**<br>`radial-gradient(#e66465, #9198e5)`, `radial-gradient(ellipse at top, #e66465, transparent)`<br>See [CSS `radial-gradient()`](https://developer.mozilla.org/en-US/docs/Web/CSS/Reference/Values/gradient/radial-gradient) for supported syntax
