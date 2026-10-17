# JIVE Examples: Plan

This is the working brief for replacing the single `jive-demo` project with a set of small examples, each focused on one area. The decisions below were agreed with the maintainer on 2026-10-04, so treat them as settled. The items under [Open questions](#open-questions) are not settled: ask before acting on them.

Once the examples exist, rewrite this file as a user-facing index to them.

## Decisions

- Six examples, listed below, replace `examples/jive-demo` entirely. That means both its app and plugin targets, `demo-app.jucer`, `Builds/` and `JuceLibraryCode/`.
- Views are XML files and styles are JSON files, loaded from disk and live-reloaded. This is the recommended practice for JIVE v2. C++ only adds behaviour (events, properties, parameters, custom views). It never builds view trees, except for the `jive::View` in 06.
- A `jive_add_example()` CMake helper removes the repeated setup each example would otherwise need.
- Folders are numbered from simplest to most complex.

| #   | Folder                 | Type   | Showcases                                                                  |
| --- | ---------------------- | ------ | -------------------------------------------------------------------------- |
| 01  | `01-text`              | App    | Text layout, automatic sizing and positioning, fonts                       |
| 02  | `02-layouts`           | App    | Flexbox, grid and absolute positioning                                     |
| 03  | `03-style-sheets`      | App    | Every style-sheet rule, from simple to complex                             |
| 04  | `04-transitions`       | App    | Every property that can be transitioned                                    |
| 05  | `05-widgets`           | Plugin | Every widget type, customising their designs, handling their changes in C++ |
| 06  | `06-custom-components` | Plugin | Writing a `jive::View` for an existing third-party component               |

Each example is one window or editor and should be readable in one sitting: a `Main.xml`, a `Styles.json`, a few files pulled in with `source="..."` only where that helps, and as little C++ as possible.

## The examples

### 01-text (App)

Shows that text needs no measuring and no `setSize()` calls.

- Use `<Text>` with its text written inside the XML element (`<Text>Hello</Text>`). Nest `<Text>` elements to mix styles within one paragraph. `gain-plugin/source/View.h` builds its "dB" suffix this way in C++.
- Automatic sizing: Text items calculate `jive::ideal-width` and `jive::ideal-height`, so their parents size themselves to fit. Demonstrate this with a resizable window (`resizable="true"`) where a paragraph re-wraps and everything below it moves.
- Item properties: `word-wrap`, `line-spacing`, `justification` and `direction`. Include a right-to-left sample.
- Style properties: `font-family` (one system font plus bundled custom fonts), `font-size`, `font-styles` (`bold`, `italic`, `underlined`), `font-kerning`, `font-scale`, `font-ascent-override` and `font-descent-override`, `text-align`, and `foreground`. Show `foreground` as a solid colour and, if it renders on text, as a gradient (check this first).
- Inheritance: fonts set once on the root element apply to every element inside it.
- Custom fonts: Rubik, Rubik Mono One and Source Code Pro are licensed under the OFL and currently live in `jive-demo/resources`. See [Migrating off jive-demo](#migrating-off-jive-demo). `jive-demo/source/jive_demo/Fonts.h` registers them with `juce::Typeface::createSystemTypefaceFor`, but nothing includes that file at the moment. Make sure fonts are registered before interpreting, and check that each one resolves by its `font-family` name.

### 02-layouts (App)

- Flex is the default `display`. Container properties: `flex-direction` (all four), `justify-content`, `align-items`, `align-content`, `flex-wrap` and `gap`. Item properties: `flex-grow`, `flex-shrink`, `flex-basis`, `order` and `align-self`.
- Grid (`display="grid"`): `grid-template-columns` and `grid-template-rows` (in px and `fr`), `grid-template-areas` with `grid-area`, `grid-column` and `grid-row` spans, `gap`, `justify-items`, `align-items`, `grid-auto-flow`, `grid-auto-rows` and `grid-auto-columns`.
- Absolute positioning (`display="block"`): place children with `x`, `y`, `centre-x` and `centre-y`, in px and `%`.
- Size and spacing on every element: `width` and `height` (px and `%`), the `min-` and `max-` sizes, `margin` and `padding`.
- Add controls whose C++ handlers change layout properties on the tree. The flex-direction page in `jive-demo` is the starting point: `views/demos/LayoutsDemo.xml`, `Presenter::presentLayoutsDemo()`, the `.box-*` classes and token palette in `Styles.json`, and the arrow icons in `views/icons/`. Port it rather than rewriting it.
- `jive_layouts/utilities/jive_Overflow.h` defines `hidden` and `scroll`, but I couldn't find any GUI item that uses it. Don't build scrolling into the demo without checking.

### 03-style-sheets (App)

Order the sections from simple to complex:

1. An inline `style` object compared with an external `style="Styles.json"` file, which is live-reloaded.
2. Values:
   - colours: hex, `rgb()`/`rgba()`, `hsl()` and named colours
   - `linear-gradient()` and `radial-gradient()`
   - `border`, `border-width` and `border-radius`
   - `shadow`, in the same syntax as CSS `box-shadow`
   - `opacity`
   - `fill` and `stroke`, which tint SVGs
   - fonts
3. Styles passing down from parent elements to their children.
4. Selectors: type (`Button`), `#id`, `.class`, compound (`Button.primary`), descendant (`#panel Text`), child (`#panel > .item`), universal (`*`), lists (`.a, .b`), and selector objects nested inside each other in the JSON.
5. Interaction states: `:hover`, `:active`, `:focus`, `:checked` and `:disabled`, including nested states (`":checked": { ":hover": { ... } }`).
6. Specificity: show a more specific selector winning over a less specific one.
7. Tokens:
   - define them with `"$name": value`
   - use them as a whole value (`"$widgetShadow"`) or inside one (`"linear-gradient(-10deg, $a, $b)"`)
   - use them inside interaction-state selectors
   - change their values from C++ at runtime to restyle the whole window, for example with a light/dark toggle

Leave out attribute selectors (`[type=text]`), pseudo-elements (`::before`) and `:not()`. `StyleSelector` only uses them when ranking specificity; `buildSinglePredicate()` never matches them. If the demo seems to need them, raise it with the maintainer.

Widget-specific properties (`accent`, `thumb`, `track`) belong in 05.

### 04-transitions (App)

- Syntax: `"<property> <duration> [<easing>] [<delay>]"`, with several transitions separated by commas. Easings: `linear`, `ease`, `ease-in`, `ease-out`, `ease-in-out` and `cubic-bezier(...)`.
- Transitions apply in two places:
  - **Layout properties**, with `transition` set on the element itself. Currently this covers `opacity`, flex `gap`, `flex-grow`, `flex-shrink`, `flex-basis`, block `x`, `y`, `centre-x`, `centre-y`, `width` and `height`, grid `grid-template-columns`, `grid-template-rows` and `gap`, and `border-width`.
  - **Style properties**, with `transition` inside a style object. A change of state such as `:hover`, or a change to a style or token, starts the transition. This covers `background`, `foreground`, `border`, `border-radius`, `border-width`, `shadow`, `fill`, `stroke`, `accent` and the `font-*` properties. Colours, gradients, shadows, corner radii, lengths and grid tracks all interpolate.
- Show one tile per property, each with something that triggers it. Style transitions driven by hover need no C++. Layout transitions need a button whose `on-click` handler sets a property.
- Show a transition being interrupted halfway: it carries on from its current value instead of jumping back.
- Check the list against the code before relying on it, because it may have grown. Search for `onTransitionProgressed` under `jive_layouts/`, and look at the `updateStyleProperty()` calls in `jive_core/graphics/jive_LookAndFeel.cpp`. I couldn't find transitions for `margin`, `padding`, or `width`/`height` on flex items, so confirm those before including them.
- Custom `jive::Interpolate<>` specialisations and custom `jive::Transition` timing functions (see `docs/Animations.md`) can only be done in C++. Include them only if they fit cleanly alongside the XML-and-JSON setup.

### 05-widgets (Plugin)

- Include every widget element:
  - `Button`: plain, `toggleable`/`toggle-on-click`, and `radio-group`
  - `Checkbox`
  - `ComboBox` with `<Option>` children
  - `Slider`, in every `orientation`
  - `Knob`, `Spinner`, `ProgressBar`, `Hyperlink` and `Label`
  - `Image` and inline `<svg>`

  Check the interpreter and `ComponentFactory` for any element missing from this list.
- Customising designs: put a default-styled instance next to a restyled one. Use the widget style properties (`background`, `foreground`, `border`, `border-radius`, `shadow`, `accent`, `thumb`, `track`, `fill`) and interaction states.
- Handling changes in C++: show both approaches and when to use each.
  1. **Plugin parameters.** Use `GuiItem::attachToParameter(param, &undoManager)`, backed by a `juce::AudioProcessorValueTreeState`. It works on `Slider`/`Knob`, `Button`/`Checkbox`, `ComboBox`, and `Text` (as a value readout).
  2. **State that only the UI uses.** Use `jive::Event` (`on-click`, `on-change`) and `jive::Property<T>` on `value`, `toggled` and `selected`. Examples: a slider driving a progress bar, or a checkbox enabling a button.
- Plugin setup:
  - The XML's top-level element is `<Editor>`.
  - Interpret it with `interpreter.interpret(file, this)` and `dynamic_cast` the result to `juce::AudioProcessorEditor`; `gain-plugin/source/Processor.h` shows how.
  - Also build the Standalone format so the example runs without a host.
- The plugin should do something trivial but audible, such as gain, pan or mute, so the parameters are real. Don't build any other DSP.

### 06-custom-components (Plugin)

- Write a `jive::View` subclass that wraps an existing component JIVE doesn't provide:
  - `initialise()` returns the element's tree: a custom type name, its size, and any state properties.
  - `createComponent()` constructs the third-party component.
  - `setup(jive::GuiItem&)` connects it: listeners and initial state.
  - Keep the tree's properties and the component in sync in both directions, using `jive::Property`, so the rest of the app can treat it like any other JIVE element.
- XML can't create a View directly. `jive::makeView<T>(args...)` returns a tree, and C++ has to insert it, for example into an empty element in `Main.xml` found by its `id`. This is the main pattern the example teaches, so show it clearly.
- Which component to wrap is an [open question](#open-questions). The recommended default is `juce::MidiKeyboardComponent`, driven by a `juce::MidiKeyboardState` that the processor owns. It adds no new dependency and suits a plugin.
- Leave out `ComponentFactory`, decorators and aliases unless the View approach needs them.

## Conventions

### Folder layout

```
examples/
    CMakeLists.txt          # add_subdirectory() for each NN-name, in order
    README.md               # this file
    resources/              # shared fonts, plus the licence README (moved from jive-demo)
    NN-name/
        CMakeLists.txt      # a single jive_add_example() call
        source/
            main.cpp        # apps
            PluginFilter.cpp, PluginProcessor.h   # plugins
            Presenter.h     # behaviour: events, properties, parameters, views
            views/
                Main.xml
                Styles.json
                ...         # anything pulled in with source="..."
```

### Runtime pattern

Follow the v2 pattern in `jive-demo/source/jive_demo/Presenter.h`:

- A `Presenter` owns the UI's `juce::ValueTree` (the state tree), a `jive::Interpreter` and the resulting `std::unique_ptr<jive::GuiItem>`.
- List the views with `jive_sources()` in the example's `CMakeLists.txt`, then interpret `jive::SourceFile{ "source/views/Main.xml" }`. Paths are relative to the `CMakeLists.txt`, and are read from disk in Debug builds and from an embedded zip in other builds. `01-text` shows this.
- Give an `id` to every element that C++ touches. Find elements on the state tree with `jive::findElementWithID()`, `findElementsWithClass()` and `findElementsWithText()`. Find GUI items with `jive::findItemWithID()`.
- Create every binding in `interpreter.onExternalSourceLoaded`: `jive::Event`, `jive::Property`, parameter attachments, and inserted Views. Reloading a source file replaces its tree completely, so any binding to the old tree stops working. Live reloading is one of the features being shown off, so the C++ behaviour must keep working after the XML or JSON is edited.
- Support `JIVE_ENABLE_MELATONIN_INSPECTOR` the same way the existing examples do.

### `jive_add_example()`

Put it in `cmake/jive_add_example.cmake` next to the other helpers, and include it from `examples/CMakeLists.txt`. Suggested signature:

```cmake
jive_add_example(<target>
                 APP | PLUGIN
                 PRODUCT_NAME <name>
                 [PLUGIN_CODE <4 chars>]
                 SOURCES <files...>
                 [RESOURCES <files...>])
```

It should:

- Call `juce_add_gui_app()` or `juce_add_plugin()`. Plugins get `FORMATS Standalone VST3 AU`, `PLUGIN_MANUFACTURER_CODE "Jive"`, and their own fixed `PLUGIN_CODE` (see commit `8100d9b`).
- Link `jive::compiler_and_linker_options`, `jive::jive_layouts`, `jive::jive_style_sheets` and the `juce::juce_recommended_*` flags. Plugins also link `juce::juce_audio_utils` and `juce::juce_audio_devices`.
- Define these compile definitions:
  - all examples: `JIVE_ENABLE_MELATONIN_INSPECTOR=$<BOOL:${JIVE_ENABLE_MELATONIN_INSPECTOR}>`
  - apps: `JUCE_APPLICATION_NAME` and `JUCE_APPLICATION_VERSION`
  - plugins: `JIVE_IS_PLUGIN_PROJECT=1` and `JUCE_VST3_CAN_REPLACE_VST2=0`
- Add the melatonin inspector when `JIVE_ENABLE_MELATONIN_INSPECTOR` is on, and call `add_pluginval_tests()` for plugins when `JIVE_ENABLE_PLUGINVAL` is on.
- Bundle `RESOURCES` with `juce_add_binary_data()`; the fonts need this.

Name targets `jive-NN-name`, for example `jive-01-text`, so they sort in order in IDEs.

### Style

Follow `.clang-format`, and match the code in the existing examples: `final` overrides, `[[nodiscard]]`, a namespace per example, brace initialisation, and few comments.

### Done means

- It builds from a clean configure with `-D JIVE_BUILD_EXAMPLES=ON`.
- It runs and shows everything listed for it above.
- Editing its XML or JSON while it runs updates the UI, and its C++ behaviour still works afterwards.
- Plugins pass pluginval at strictness level 10.
- It doesn't depend on anything this README lists as not working.

## Suggested order of work

1. Write `jive_add_example()` and create `examples/resources/`.
2. Build 01-text. It's the smallest example and tests the helper, font loading and live reload.
3. Build 02-layouts by porting the flex-direction page from `jive-demo`.
4. Build 03-style-sheets, then 04-transitions.
5. Build 05-widgets, then 06-custom-components. These test the plugin side of the helper and pluginval.
6. Remove `jive-demo` and everything that refers to it.

## Migrating off jive-demo

Do this last.

1. Move `examples/jive-demo/resources/*.ttf` and its licence `README.md` to `examples/resources/`. The arrow icons in `source/jive_demo/views/icons/` go with the flex-direction page into 02-layouts. They're Material Symbols, Apache 2.0, so keep the attribution.
2. Delete `examples/jive-demo/` and its `add_subdirectory()` call.
3. Update everything that refers to it:
   - `.github/scripts/pluginval-macos.sh` and `pluginval-windows.ps1` hard-code `build/runners/demo-runner/jive-demo-plugin_artefacts/.../JIVE Demo.vst3`. No workflow calls these scripts, and CTest's `add_pluginval_tests()` already covers pluginval. Ask before deleting them.
   - `tests/benchmarks/CMakeLists.txt` defines `JIVE_DEMO_VIEWS_SOURCE_DIRECTORY` as `examples/jive-demo/source/views`, and `tests/benchmarks/source/PageSwitchBenchmark.h` loads `app.xml` from it. That folder disappeared in commit `483f248`, so the benchmark's source is already missing. See the open questions.
   - `.github/workflows/test-runner.yml` configures with `JIVE_ENABLE_PROJUCER_PROJECTS=ON`, and `jive-demo` is the only example with a `.jucer` file. See the open questions.
4. At the time of writing, the working tree has staged, uncommitted changes to `jive-demo` (`Presenter.h`, `Styles.json`, `LayoutsDemo.xml`) mixed in with library changes. Don't delete the folder until those have been committed or ported into 02, and leave the library changes alone.

## Problems found while planning

These belong to the library or to other examples. Report them to the maintainer rather than fixing them as part of this work.

- Attribute selectors, `::before` and `:not()` affect specificity but never match anything.
- No GUI item uses `jive::Overflow`.
- Nothing includes `jive-demo`'s `Fonts.h`, so its fonts are never registered.
- `gain-plugin/source/View.h` uses `letter-spacing` and `font-weight`, which aren't style properties; the current ones are `font-kerning` and `font-styles`. Don't copy them.

## Open questions

Ask the maintainer about these before acting on them.

1. **`examples/gain-plugin` and `examples/xml-views`.** It hasn't been decided whether these stay. They overlap with 05 and with 01–03. Leave them alone until told otherwise.
2. **The component for 06.** The recommendation is `juce::MidiKeyboardComponent`. The alternative is a library from outside JUCE, fetched with CPM using `~/.cache/CPM` as its cache.
3. **Projucer projects.** The recommendation is to build the new examples with CMake only. If that's agreed, decide whether `JIVE_ENABLE_PROJUCER_PROJECTS` and `cmake/projucer.cmake` are still needed.
4. **Benchmarks.** Decide what `PageSwitchBenchmark` should load once `jive-demo` is gone.
