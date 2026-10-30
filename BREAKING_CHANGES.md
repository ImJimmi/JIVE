# JIVE Breaking Changes

This file lists breaking changes introduced to JIVE with each release, if any. When migrating from an older version of JIVE, be sure to go through each version's changes methodically to ensure a proper migration with no regressions.

# Version 2.0

JIVE 2.0 introduced significant breaking changes which were necessary to address some major limitations in the framework.

## `jive_components` has been merged into `jive_style_sheets`

The components have been moved to [`jive_style_sheets/components/`](./jive_style_sheets/components/).

### Reason

[`jive::LookAndFeel`](./jive_style_sheets/graphics/jive_LookAndFeel.h) paints JIVE's own components so they have to be in the same module, and there was little value to the components being in their own module.

### Migrating

Remove the `jive_components` module from your CMake or Projucer projects, and replace any includes of `<jive_components/jive_components.h>` with `<jive_style_sheets/jive_style_sheets.h>`.

## Graphics have moved from `jive_core` to `jive_style_sheets`

`jive::LookAndFeel`, `jive::Styles`, `jive::StyleSelector`, `jive::Fill`, `jive::Gradient`, `jive::Shadow`, `jive::Icon`, the themes, and the `getBorderRadius()` and `getBorderWidth()` functions have been moved to [`jive_style_sheets/graphics/`](./jive_style_sheets/graphics/) and [`jive_style_sheets/geometry/`](./jive_style_sheets/geometry/).

### Reason

Everything JIVE paints is driven by styles, so the graphics belong in the same module as the style sheets that provide them.

### Migrating

Add the `jive_style_sheets` module to any Projucer projects that only used `jive_core`, and replace includes of `<jive_core/jive_core.h>` with `<jive_style_sheets/jive_style_sheets.h>` wherever you use any of the above. CMake projects linking `jive::jive_layouts` need no changes.

## Style sheets can no longer be turned off

The [`jive_layouts`](./jive_layouts/) module now depends on [`jive_style_sheets`](./jive_style_sheets/). The `JIVE_GUI_ITEMS_HAVE_STYLE_SHEETS` flag has been removed, and every GUI item gets a style sheet.

### Reason

GUI items rely on style sheets for text alignment, reading direction and line spacing, which are now style properties.

### Migrating

Remove `JIVE_GUI_ITEMS_HAVE_STYLE_SHEETS` from your build. Projucer projects need the `jive_core`, `jive_style_sheets` and `jive_layouts` modules. CMake projects only need to link `jive::jive_layouts`, which brings in the other two.

## Interaction states are pseudo-classes

Interaction-state keys now need a leading colon, like CSS pseudo-classes. Keys without the colon are read as type selectors and match nothing.

### Reason

This matches CSS, and lets states be combined with other selectors, for example `Checkbox:checked` or `:checked:hover`.

### Migrating

Add a colon to each state key:

```jsonc
// 1.x
{
    "hover": { "background": "lime" },
    "active": { "background": "darkgreen" },
    "focus": { "border": "yellow" },
    "disabled": { "background": "grey" },
    "checked": { "background": "blue" }
}
```

```jsonc
// 2.0
{
    ":hover": { "background": "lime" },
    ":active": { "background": "darkgreen" },
    ":focus": { "border": "yellow" },
    ":disabled": { "background": "grey" },
    ":checked": { "background": "blue" }
}
```

## Some font properties have been renamed

| 1.x                               | 2.0                                                                  |
| --------------------------------- | -------------------------------------------------------------------- |
| `"font-weight": "bold"`           | `"font-styles": "bold"`                                              |
| `"font-style": "italic"`          | `"font-styles": "italic"`                                            |
| `"text-decoration": "underlined"` | `"font-styles": "underlined"`                                        |
| `"letter-spacing": 2` (pixels)    | `"font-kerning": 0.1` (a factor of the font height, e.g. 2px / 20pt) |
| `"font-stretch": 1.2`             | `"font-scale": 1.2`                                                  |

### Reason

Each property now maps directly onto a `juce::Font` option, and `font-styles` can combine bold, italic and underlined in one value.

### Migrating

Rename the properties as shown above. Combine styles in a single value, for example `"font-styles": "bold italic"`.

## Gradients use CSS syntax

The `orientation`, `start` and `end` keys of gradient objects are no longer read.

### Reason

CSS gradients can point in any direction, use side and corner keywords, and position each colour stop, and the syntax is familiar from the web.

### Migrating

Write gradients as `linear-gradient()` or `radial-gradient()` strings:

```jsonc
// 1.x
"background": {
    "gradient": "linear",
    "orientation": "horizontal",
    "stops": {
        "0.0": "red",
        "0.3": "#FF00FF",
        "1.0": "blue"
    }
}
```

```jsonc
// 2.0
"background": "linear-gradient(to right, red 0%, #FF00FF 30%, blue 100%)"
```

See [`<gradient>` CSS type](https://developer.mozilla.org/en-US/docs/Web/CSS/Reference/Values/gradient), with which JIVE's gradients should be compatible.

## Text alignment and direction have moved to style sheets

The `justification` and `direction` attributes of `<Text>` have been removed.

### Reason

They shouldn't have any impact on layout as they are purely stylistic, and so they fit better as style sheet properties.

### Migrating

Set `text-align` and `direction` in a style sheet instead. `text-align` takes the same values as `justification` did (`centred-left`, `top-right` and so on).

#### 1.x

```xml
<Text justification="centred" direction="right-to-left">...</Text>
```

#### 2.0

```xml
<Text id="greeting">...</Text>
```

```jsonc
{
  "#greeting": {
    "text-align": "centred",
    "direction": "right-to-left"
  }
}
```

`word-wrap` is still an attribute. `line-spacing` can be set as an attribute or as a style property.

## `opacity` has moved to style sheets

The `opacity` attribute has been removed.

### Reason

Opacity is purely stylistic, so it belongs with the other style properties. As a style property it can also change with interaction states such as `:hover`.

### Migrating

Set `opacity` in a style sheet instead. It also accepts percentages, such as `"50%"`. To transition it, list it in the style sheet's `transition` rather than the element's.

#### 1.x

```xml
<Component opacity="0.5" transition="opacity 200ms" />
```

#### 2.0

```xml
<Component id="faded" />
```

```jsonc
{
  "transition": "opacity 200ms",

  "#faded": {
    "opacity": 0.5
  }
}
```

## `centre-x` and `centre-y` have been replaced by `anchor`

The `centre-x` and `centre-y` attributes have been removed. Block items are always positioned with `x` and `y`, and the new `anchor` attribute sets which point on the item sits at that position.

### Reason

`centre-x` and `centre-y` could only position an item by its top-left corner or its centre, and setting one cleared the other. `anchor` can use any point on the item, such as `bottom-right` or `10px 50%`, without changing which properties hold the position.

### Migrating

Replace `centre-x` and `centre-y` with `x` and `y`, and add `anchor="centred"`. Any transitions on `centre-x` or `centre-y` should now be on `x` or `y`.

#### 1.x

```xml
<Component centre-x="50%" centre-y="50%" width="60" height="40" />
```

#### 2.0

```xml
<Component x="50%" y="50%" anchor="centred" width="60" height="40" />
```

## Default values are no longer written to the tree

In 1.x, GUI items wrote default values for unset properties into the `juce::ValueTree`. For example, `display="flex"`, `opacity="1"`, `enabled="1"`, `flex-direction="column"` and `justification="centred-left"` appeared on elements that never set them. 2.0 leaves unset properties unset and applies the defaults internally.

### Reason

Every write to a tree notifies its listeners, so leaving defaults out makes building views cheaper. The tree now only holds the values you set.

### Migrating

Code that reads a property it never set, such as `tree["display"]` or `tree["flex-direction"]`, now gets an empty `juce::var`. Supply the default yourself, for example with `jive::Property::getOr()`.

## The `em` and `rem` units have been removed

Lengths are now either pixels or percentages (`%`).

**Why:** 1.x worked these units out by searching the element's tree for a `font-size` in its `style` object. That missed fonts set by selectors, style sheet files and themes.

**How to migrate:** Replace `em` and `rem` lengths with pixel or percentage values.

## Aliases have been removed

`Interpreter::setAlias()` has been removed.

### Reason

External sources do the same job, and can be edited and hot-reloaded while the app is running.

### Migrating

Move the markup you aliased into its own XML file and pull it in with `template`:

```xml
<Component template="views/Header.xml"/>
```

## `jive::View` has been removed

`jive::View`, `jive::makeView()` and `GuiItem::getView()` have been removed, along with the `GuiItem` constructor that took a view.

### Reason

Each of its hooks has a replacement that keeps working when a view is hot-reloaded: `template` files for `initialise()`, the interpreter's `ComponentFactory` for `createComponent()`, and `Interpreter::onLoad` for `setup()`.

### Migrating

Declare the element in XML, register its component with the interpreter's `ComponentFactory`, and do any setup in `onLoad`:

```cpp
// 1.x
class KeyboardView : public jive::View
{
public:
    explicit KeyboardView(juce::MidiKeyboardState& state)
        : keyboardState{ state }
    {
    }

protected:
    juce::ValueTree initialise() final
    {
        return juce::ValueTree{ "Keyboard", { { "width", 400 }, { "height", 80 } } };
    }

    std::unique_ptr<juce::Component> createComponent(const juce::ValueTree&) final
    {
        return std::make_unique<juce::MidiKeyboardComponent>(keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard);
    }

    void setup(jive::GuiItem& item) final
    {
        // ...
    }

private:
    juce::MidiKeyboardState& keyboardState;
};

footer.appendChild(jive::makeView<KeyboardView>(keyboardState), nullptr);
```

```xml
<!-- 2.0 -->
<Keyboard id="keyboard" width="400" height="80"/>
```

```cpp
// 2.0
interpreter.getComponentFactory().set("Keyboard", [&keyboardState] {
    return std::make_shared<juce::MidiKeyboardComponent>(keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard);
});

interpreter.onLoad = [](jive::GuiItem& root) {
    auto* keyboard = jive::findItemWithID(root, "keyboard");
    // ...
};
```

## The `mouse` and `keyboard` properties have been removed

1.x wrote `mouse` (`hover`, `active`) and `keyboard` (`focus`) properties into each element's tree. 2.0 no longer does.

### Reason

Interaction states are now read straight from the component when styles are resolved, so there's nothing to keep in the tree.

### Migrating

Use the `:hover`, `:active` and `:focus` pseudo-classes in style sheets, or listen to the component directly.

## Internal properties have a `jive::` prefix

`ideal-width`, `ideal-height`, `component-width` and `component-height` have been renamed to `jive::ideal-width`, `jive::ideal-height`, `jive::component-width` and `jive::component-height`.

### Reason

The prefix marks properties that JIVE manages itself, so they don't clash with attributes you write.

### Migrating

Update any code that reads these properties to use the new names.

## `Interpreter::interpret()` is no longer `const`

### Reason

The interpreter now watches source files for changes so that it can hot-reload them.

### Migrating

Call `interpret()` on a non-`const` interpreter, and keep the interpreter alive for as long as you want hot reloading.

## `jive::Window` is now a `jive::DocumentWindow`

`Window::getWindow()` has been removed, because `jive::Window` is now the window itself. The item that `interpret()` returns for a `<Window>` can be cast straight to a `juce::DocumentWindow`, just as an `<Editor>` can be cast to a `juce::AudioProcessorEditor`.

### Reason

When a `<Window>` is hot-reloaded, the new content is swapped into the same native window.

### Migrating

Call the window's methods directly:

```cpp
// 1.x
if (auto* window = dynamic_cast<jive::Window*>(item.get()))
    window->getWindow().toFront(true);
```

```cpp
// 2.0
if (auto* window = dynamic_cast<jive::Window*>(item.get()))
    window->toFront(true);
```

## `jive::Length` is a value type

`jive::Length` is now a plain value with a `value` and a `unit`, instead of a `jive::Property<juce::String>`. `toPixels()` now takes the length to resolve percentages against, rather than the parent's bounds.

### Reason

Lengths can now be stored, compared, interpolated for transitions, and used in gradient colour stops.

### Migrating

Use `jive::Property<jive::Length>` to bind a length to a tree:

```cpp
// 1.x
jive::Length width{ tree, "width" };
const auto pixels = width.toPixels(parentBounds);
```

```cpp
// 2.0
jive::Property<jive::Length> width{ tree, "width" };
const auto pixels = width.get().toPixels(parentBounds.getWidth());
```

## An image's `source` can only be a path

The `source` of an `<Image>` is now always the path of an image file. Setting it to a string of SVG markup, or to a `juce::Image` from C++, draws nothing.

### Reason

Images are found the same way as every other source, so PNG, JPEG and GIF files can be used as well as SVGs, and SVGs are kept out of the view's markup.

### Migrating

Move SVG markup into its own SVG file and refer to it by its path, or write the `<svg>` element directly in the view:

```xml
<!-- 1.x -->
<Image source="&lt;svg viewBox=&quot;0 0 24 24&quot;&gt;...&lt;/svg&gt;"/>
```

```xml
<!-- 2.0 -->
<Image source="icons/star.svg"/>
```

Save a `juce::Image` to a file in your sources and refer to it by its path, or draw it with a `juce::ImageComponent`.
