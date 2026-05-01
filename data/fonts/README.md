# Fonts Directory

This directory contains font files used for contour labeling.

## Default Font

- **default.ttf**: Roboto Regular - Default font for contour labels

## Roboto Font Family

The Roboto font family is included under the Apache License 2.0.

### Available Fonts

- Roboto-Regular.ttf - Regular weight
- Roboto-Bold.ttf - Bold weight
- Roboto-Light.ttf - Light weight
- Roboto-Medium.ttf - Medium weight
- And more variants...

## License

Roboto fonts are licensed under the Apache License 2.0.
See LICENSE file in the roboto directory for details.

## Usage

To use a custom font in your application:

```c
cf_font_t* font;
cf_font_load("data/fonts/default.ttf", 24.0f, &font);
```

## Adding Custom Fonts

You can add your own TrueType (.ttf) fonts to this directory.
Make sure the fonts are licensed for your use case.
