import pytest
import xa11y

HEADINGS = {
    "01-text": "Text",
    "02-layouts": "Layouts",
    "03-style-sheets": "Style Sheets",
    "04-transitions": "Transitions",
    "05-widgets": "Widgets",
}


@pytest.mark.parametrize("window", HEADINGS.keys(), indirect=True)
def test_shows_heading(window: xa11y.Locator, request: pytest.FixtureRequest):
    heading = HEADINGS[request.node.callspec.params["window"]]

    assert window.descendant(f"static_text[value='{heading}']").exists()


@pytest.mark.parametrize("window", HEADINGS.keys(), indirect=True)
def test_theme_picker_defaults_to_steel(window: xa11y.Locator):
    assert window.descendant("combo_box").first().element().value == "Steel"
