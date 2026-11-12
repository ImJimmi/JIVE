import pytest
import xa11y

pytestmark = pytest.mark.parametrize("window", ["01-text"], indirect=True)


@pytest.mark.parametrize("style", ["Bold", "Italic", "Underlined"])
def test_font_style_toggles(window: xa11y.Locator, style: str):
    check_box = window.descendant(f"check_box[name='{style}']")

    check_box.toggle()

    check_box.wait_until(lambda element: element is not None and element.checked == "on")


def test_shows_poem(window: xa11y.Locator):
    assert window.descendant("static_text[value*='Ozymandias']").exists()
