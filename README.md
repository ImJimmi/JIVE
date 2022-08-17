![JIVE Banner](assets/banner.svg)

[![codecov](https://img.shields.io/codecov/c/gh/ImJimmi/JIVE?label=CodeCov&logo=codecov&style=for-the-badge&token=827SYOR678)](https://codecov.io/gh/ImJimmi/JIVE)

[![codecov](https://codecov.io/gh/ImJimmi/JIVE/branch/main/graphs/sunburst.svg?token=827SYOR678)](https://codecov.io/gh/ImJimmi/JIVE)

JIVE is a collection of JUCE modules aimed at making front-end development much simpler.

Developing UIs with JUCE can be cumbersome and downright frustrating at times. Using a proper architecture, such as the [MVP pattern](https://www.martinfowler.com/eaaDev/uiArchs.html#Model-view-presentermvp), can massively improve this experience; however there's a limit to how well you can polish the turd that is UI development in C++. Each of JIVE's modules aims to tackle a specific part of UI development to make the whole experience much more enjoyable!

# Modules

## [`jive_layouts`](jive_layouts/)

_Use XML to describe the content and layout of your UI, removing the bloat of having dozens of component classes, and hundreds of files._

Specifically designed to fit with the MVP pattern, `jive_layouts` aims to humanise the _view_ layer of your application by dynamically constructing components from XML documents, which can be dynamically constructed and manipulated by _presenters_ at runtime.

### Example

Describe your content and layout using XML, or Value Trees:

#### XML

```xml
<Component id="mainPage">
    <Label font-size="30">Welcome!</Label>
    <Label>JIVE is super cool and awesome!</Label>
</Component>
```

#### Value Trees

```cpp
juce::ValueTree buildMainPageMarkup()
{
    juce::ValueTree mainPage{ "Component" };
    mainPage.setProperty("id", "mainPage", nullptr);

    mainPage.appendChild({
        "Label",
        {
            { "font-size", 30 },
            { "text", "Welcome!" }
        }
    }, nullptr);

    mainPage.appendChild({
        "Label",
        {
            { "text", "JIVE is super cool and awesome!" }
        }
    }, nullptr);

    return mainPage;
}
```

Then, dynamically construct your UI at runtime in a presenter class:

```cpp
std::unique_ptr<jive::GuiItem> MainPage::createView()
{
    const auto markup = buildMainPageMarkup(); // Or, read the XML file.

    jive::ViewRenderer viewRenderer;
    return viewRenderer.renderView(markup);
}
```

### Highlights

<details>
<summary>Reduce boilerplate by only writing code that's actually relevant</summary>

Same UI, 3 different ways.

<blockquote>
<details>
<summary>Typical JUCE component class<br>(42 lines, 1137 characters)</summary>

```cpp
class MainPage : public juce::Component
{
public:
    MainPage()
    {
        coloursBox.setComponentID("colours");
        coloursBox.addItemList({"Red", "Green", "Blue"}, 1);
        addAndMakeVisible(coloursBox);

        shapesBox.setComponentID("shapes");
        shapesBox.addItemList({"Square", "Triangle", "Circle", 1});
        addAndMakeVisible(shapesBox);

        setComponentID("mainPage");
        setSize(400, 300);
    }

    void resized() override
    {
        juce::FlexBox flex;

        flex.direction = juce::FlexBox::Direction::column;
        flex.justifyContent = juce::FlexBox::JustifyContent::center;
        flex.items = {
            juce::FlexItem{ coloursBox }
                .withWidth(100.f)
                .withHeight(20.f)
                .withMargin({ 0.0f, 0.0f, 10.f, 0.0f }),
            juce::FlexItem{ shapesBox }
                .withWidth(150.f)
                .withHeight(25.f);
        };

        flex.performLayout(getLocalBounds().reduced(10));
    }

private:
    juce::ComboBox coloursBox;
    juce::ComboBox shapesBox;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainPage)
};
```
</details>
</blockquote>

<blockquote>
<details>
    <summary>JIVE, using juce::ValueTree<br>(50 lines, 1083 characters)</summary>

```cpp
juce::ValueTree createColoursBoxMarkup()
{
    return {
        "ComboBox",
        {
            { "width", 100 },
            { "height", 20 },
            { "margin", "0 0 10 0" }
        },
        {
            { "Option", { { "text", "Red" } } },
            { "Option", { { "text", "Green" } } },
            { "Option", { { "text", "Blue" } } }
        }
    };
}

juce::ValueTree createShapesBoxMarkup()
{
    return {
        "ComboBox",
        {
            { "width", 150 },
            { "height", 25 }
        },
        {
            { "Option", { { "text", "Square" } } },
            { "Option", { { "text", "Triangle" } } },
            { "Option", { { "text", "Circle" } } }
        }
    };
}

juce::ValueTree createMainPageMarkup()
{
    return {
        "Component",
        {
            { "id", "mainPage" },
            { "width", 400 },
            { "height", 300 },
            { "justify-content", "centre" },
            { "padding", 10 }
        },
        {
            createColoursBoxMarkup(),
            createShapesBoxMarkup()
        }
    };
}
```
</details>
</blockquote>

<blockquote>
<details>
<summary>JIVE, using XML<br>(13 lines, 443 characters)</summary>

```xml
<Component id="mainPage" width="400" height="300" justify-content="centre" padding="10">
    <ComboBox id="colours" width="100" height="20" margin="0 0 10 0">
        <Option>Red</Option>
        <Option>Green</Option>
        <Option>Blue</Option>
    </ComboBox>

    <ComboBox id="shapes" width="150" height="25">
        <Option>Square</Option>
        <Option>Triangle</Option>
        <Option>Circle</Option>
    </ComboBox>
</Component>
```
</details>
</blockquote>
</details>
</details>

<details>
<summary>Fewer classes and fewer files - reduce the need for awkward class names</summary>

<blockquote>
<details>
<summary>Typical naming of JUCE component classes</summary>

```
MainPage
    MainHeader
    MainContent
        InputPanelsGroup
            InputPanel
                InputPanelButtonsRow
            InputPanel
                InputPanelButtonsRow
        OutputsPanel
    MainFooter
```

- Number of _custom_ component classes: 8
- Number of files: 8 (header-only, 16 for .h/.cpp)

</details>
</blockquote>

<blockquote>
<details>
<summary>Same hierarchy, using JIVE</summary>

```xml
<Component id="mainPage">
    <Component id="header"></Component>

    <Component id="mainContent">
        <Component id="inputs">
            <Panel id="input0">
                <Component id="row">
                </Component>
            </Panel>

            <Panel id="input1">
                <Component id="row">
                </Component>
            </Panel>
        </Component>

        <Panel id="outputs">
        </Panel>
    </Component>

    <Component id="footer"></Component>
</Component>
```

- Number of _custom_ component classes: 1 (`<Panel>`)
- Number of files: 2 (header-only - `main-page.xml`, `MainPage.h`, 3 for .h/.cpp)

</details>
</blockquote>
</details>

<details>
<summary>Make changes at runtime to allow for easier debugging, or allow your UI designers to demo changes in the actual app, all without needing to recompile</summary>

```cpp
void timerCallback() override
{
    // Read the XML file on a timer - every time you save the changes, the
    // UI will update in realtime. No need to recompile!
    const juce::File jiveFile{ "main-page.xml" };
    const auto jiveSource = jiveFile.loadFileAsString();

    jive::ViewRenderer renderer;
    view = renderer.renderView (jiveSource);
}
```
</details>

<details>
<summary>Fully supports custom components, allowing for a mix of JIVE and JUCE which helps integrate with legacy code</summary>

```cpp
const auto jiveSource = R"(
    <MyWindow width="300" height="200">
        <MyWidget></MyWidget
    </MyWindow>
)";

jive::ViewRenderer renderer;
renderer.setFactory ("MyWindow", []() { return std::make_unique<MyWindow>() });
renderer.setFactory ("MyWidget", []() { return std::make_unique<MyWidget>() });

auto view = renderer.renderView (jiveSource);
```
</details>

<details>
<summary>Makes front-end testing much simpler by removing the need to test components</summary>

```cpp
struct MainPageTests : public juce::UnitTest
{
    MainPageTests()
        : juce::UnitTest{ "Main Page Tests" }
    {
    }

    void runTest() final
    {
        // No need to test the actual rendered UI, we can simply test that our
        // view builders produce the correct objects so that JIVE will render
        // our view as we expect
        const juce::ValueTree jiveTree = buildMainPageJiveTree();

        beginTest("Main page is 300px x 200px");
        expect(jiveTree["width"] == 300);
        expect(jiveTree["height"] == 200);

        beginTest("Main page has a title label");
        const auto title = jiveTree.getChildWithProperty ("id", "title");
        expect(title.hasType("Label"));
        expect(title["text"] == "Welcome!");
    }
};
```
</details>
