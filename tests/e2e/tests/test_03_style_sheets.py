import pytest
import xa11y

pytestmark = pytest.mark.parametrize("window", ["03-style-sheets"], indirect=True)


def test_shows_card_rule(window: xa11y.Locator):
    assert window.descendant("static_text[value^='\"#card\"']").exists()


@pytest.mark.parametrize("tab", ["Selectors", "States", "Tokens"])
def test_tabs_select(window: xa11y.Locator, tab: str):
    radio = window.descendant(f"radio_button[name='{tab}']")

    radio.press()

    radio.wait_until(lambda element: element is not None and element.checked == "on")
    assert window.descendant("radio_button[name='Properties']").element().checked == "off"
