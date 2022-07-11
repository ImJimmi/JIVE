from itertools import chain
from os import system
from pathlib import Path
from sys import exit
from xml.etree import ElementTree

if __name__ == "__main__":
    num_errors = 0

    for filepath in chain(Path(".").rglob("*.h"), Path(".").rglob("*.cpp")):
        if "build/" in str(filepath) or "runners/libraries/JUCE/" in str(filepath):
            continue

        system(
            f"clang-format --style=file -i --output-replacements-xml {filepath.absolute()} > clang-format-changes.xml"
        )
        replacements = (
            ElementTree.parse("clang-format-changes.xml")
            .getroot()
            .findall("replacement")
        )

        if len(replacements) > 0:
            num_errors += len(replacements)
            print(f"{filepath} needs formatting!")

    if num_errors > 0:
        error_or_errors = "error" if num_errors == 1 else "errors"
        print(
            f"\nFound {num_errors} formatting {error_or_errors}.\nPlease run clang-format to amend all relevant commits."
        )

    exit(num_errors)
