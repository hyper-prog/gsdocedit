![GsDocEdit logo](https://raw.githubusercontent.com/hyper-prog/gsdocedit/master/icons/gsdocedit.png)

GsDocEdit — gSafe Document Editor
=====================================

GsDocEdit is a graphical editor and PDF generator for the document description language used in the [gSafe](https://github.com/hyper-prog/gsafe) C++ library (`HPageTileRenderer` / `HTextProcessor`, defined in `gSAFE/po.h`).  

---

## Features

| Feature | Description |
|---|---|
| Code editor | Line-number gutter, current-line highlight |
| Live preview | Generates a PDF and opens it in the system viewer (F5) |
| PDF export | Save the generated PDF to any location (F6) |
| Batch export | Generate a separate PDF for every row of a CSV data file |
| Key-value replacement | Define named tokens substituted into the document at render time |
| CSV import / export | Load and save the replacement map as a semicolon-delimited CSV |
| Underlay PDF | Overlay rendered content on top of an existing PDF (requires qpdf) |
| Debug console | Built-in gSAFE debug console |

---

## Building

### Prerequisites

- Qt 6.x (tested with 6.10.x) with the following modules:  
  `gui`, `network`, `opengl`, `sql`, `xml`, `widgets`, `printsupport`
- A C++17 compiler (MinGW 64-bit on Windows, GCC/Clang on Linux/macOS)

### Steps

```bash
# 1. Open the project in Qt Creator or run qmake from the build directory:
qmake gsdocedit.pro

# 2. Build
mingw32-make -j4        # Windows / MinGW
# or
make -j4                # Linux / macOS
```

---

## Document Language Reference

Documents are plain-text files processed in two stages:

1. **Text pre-processing** (`HTextProcessor`) — token substitution, conditionals, and function calls.
2. **Rendering** (`HPageTileRenderer`) — line-oriented drawing instructions that produce the final PDF.

### Text Pre-processor

| Syntax | Meaning |
|---|---|
| `// comment text` | Line comment — ignored by the renderer |
| `// @AnnotationName:value` | Named annotation (e.g. `// @Title:My Report`) |
| `{{.container.key}}` | Substituted with the value from the named replacement map |
| `{{.c.key1\|.c.key2\|fallback}}` | Returns the first non-empty value |
| `{{s1=s2?[[val1]]:val2}}` | Conditional: returns `val1` if `s1 == s2`, else `val2` |
| `COND#val1#op#val2` … `ENDC` | Block conditional (`op`: `=` `!=` `<` `>` `<=` `>=`) |
| `FUNC#name` … `ENDF` | Define a reusable function block |
| `CAll#name` | Call a previously defined function |

**Built-in token literals:**

| Token | Value |
|---|---|
| `{{..point}}` | `.` |
| `{{..colon}}` | `:` |
| `{{..semicolon}}` | `;` |
| `{{..question}}` | `?` |
| `{{..openbrackets}}` | `{` |
| `{{..closebrackets}}` | `}` |

### Renderer Instructions

Each line holds one instruction. The command name and its arguments are separated by `#`.

**Cursor & flow**

| Instruction | Syntax | Description |
|---|---|---|
| `mova` | `mova#<pX>,<pY>` | Move cursor to absolute position |
| `movr` | `movr#<dX>,<dY>` | Move cursor relative to current position |
| `newl` | `newl` | Start a new line |
| `newp` | `newp` | Start a new page |
| `npif` | `npif#<height>` | New page if the remaining height is less than `<height>` |
| `npuc` | `npuc#<n>` | Advance pages until the document has at least `<n>` pages |

**Content elements**

| Instruction | Syntax | Description |
|---|---|---|
| `spac` | `spac#<sX>,<sY>` | Empty space — moves cursor without drawing |
| `rect` | `rect#<sX>,<sY>` or `rect#<pX>,<pY>,<sX>` | Rectangle (frame / fill, no content) |
| `text` | `text#<sX>#<content>` or `text#<pX>,<pY>,<sX>#<content>` | Plain text box |
| `html` | `html#<sX>#<content>` or `html#<pX>,<pY>,<sX>#<content>` | HTML text box |
| `mark` | `mark#<sX>#<content>` or `mark#<pX>,<pY>,<sX>#<content>` | Markdown text box |
| `imgr` | `imgr#<sX>#<filename>` or `imgr#<pX>,<pY>,<sX>#<filename>` | Image from file or Qt resource |
| `imgb` | `imgb#<sX>#<base64>` or `imgb#<pX>,<pY>,<sX>#<base64>` | Image from Base64-encoded data |
| `grid` | `grid#<sX>,<sY>,<w>,<h>` | Draw a grid |

> **Relative vs. absolute elements:** instructions *without* a start position (`<pX>,<pY>`) are placed at the cursor and advance it. Instructions *with* a start position are placed at the given absolute coordinates and do **not** move the cursor.

**Line height helpers**

| Instruction | Description |
|---|---|
| `fixh` | Buffer subsequent items until `newl`; use the tallest item's height for all |
| `smhz` | Reset minimum line height to zero |
| `smhv#<height>` | Set minimum line height to a fixed value |
| `smht#<sX>#<text>` | Raise minimum line height to the height of a plain text fragment |
| `smhh#<sX>#<html>` | Raise minimum line height to the height of an HTML fragment |
| `smhm#<sX>#<markdown>` | Raise minimum line height to the height of a Markdown fragment |
| `smhr#<sX>#<filename>` | Raise minimum line height to the height of an image (from file) |
| `smhi#<sX>#<base64>` | Raise minimum line height to the height of an image (Base64) |

**Areas (clipping regions)**

| Instruction | Syntax | Description |
|---|---|---|
| `area` | `area#<sX>,<sY>` or `area#<pX>,<pY>,<sX>,<sY>` | Restrict drawing to a sub-area |
| `reta` | `reta` | Return from the current restricted area |

**Styling**

| Instruction | Syntax | Description |
|---|---|---|
| `colf` | `colf#rrggbb` | Font (text) color — hex without `#` |
| `coll` | `coll#rrggbb` | Line / frame color |
| `colb` | `colb#rrggbb` | Fill / background color |
| `fram` | `fram#none\|all\|top\|right\|bottom\|left\|fill,...` | Border flags (comma-separated) |
| `alig` | `alig#left\|center\|right\|just` | Text alignment |
| `setf` | `setf#FontName,PointSize` | Set active font |
| `setd` | `setd#FontName,PointSize` | Set active font and store as default |
| `deff` | `deff` | Reset font to the stored default |
| `marg` | `marg#<top>,<right>,<bottom>,<left>` | Page margins (e.g. `marg#25mm,25mm,25mm,25mm`) |
| `sizc` | `sizc#1.075` | Size correction ratio for `mm`/`cm` units (default `1.000`) |

**Page headers / footers**

```
EVERYPAGE_START
  html#100%#<b>Page header</b>
EVERYPAGE_END
```

The block between `EVERYPAGE_START` and `EVERYPAGE_END` is replayed at the beginning of every new page.

**Stored positions**

| Instruction | Syntax | Description |
|---|---|---|
| `getp` | `getp#<name>` | Record the position of the *next* added element under `<name>` |

### Position Strings

All size/position arguments use a flexible *position string* format:

| Example | Meaning |
|---|---|
| `120` | 120 pixels |
| `20%` | 20 % of the page width or height |
| `2em` | 2 × the current letter width/height |
| `1cm` | 1 centimetre |
| `5mm` | 5 millimetres |
| `-10%` | 90 % (10 % back from the far edge) |
| `-80` | 80 pixels less than the page dimension |
| `>50%` | From the current cursor position to the 50 % mark |
| `>-2em` | From the current cursor position to 2 em before the far edge |

### Multi-line Instructions

Append `#{` to any instruction to start a multi-line block. End the block with a line containing only `}`:

```
html#100%#{
    This is a <strong>multiline</strong> HTML block.
    It continues until the closing brace.
}
```

### Underlay PDF

To render content on top of an existing PDF (e.g. a pre-printed form), add the annotation to the document source:

```
// @UnderlayPdf:form_template.pdf
```

The file is looked up in the `documents/` subdirectory of the working directory.  
This feature requires the `qpdf` binary placed in a `qpdf/` subdirectory next to the executable.

---

## Key-Value Replacement Data

Replacement data is stored in `key-values.csv` (semicolon-delimited) in the application's working directory and is loaded automatically on startup.

Keys are grouped by a dot-prefix:

```
values.name;John Doe
values.date;2026-03-16
company.name;Acme Corp
```

These become available in the document as `{{.values.name}}`, `{{.company.name}}`, etc.

Use **Functions → Edit Replacement Data** to manage key-value pairs through the built-in dialog, or **Export / Import Key-Value** to load/save a CSV file.

---

## Batch Export (Document Array)

**Functions → Export document array according to CSV file** generates one PDF per row of a data CSV file. Each column in the CSV overrides a key in the current replacement map, and the resulting PDFs are written to a chosen output directory.

---

## Dependencies

| Dependency | Role |
|---|---|
| [Qt 6](https://www.qt.io/) | GUI, PDF writer, image handling |
| [gSafe](https://github.com/hyper-prog/gsafe) | Document renderer (`HPageTileRenderer`, `HTextProcessor`, `HPdfPreviewDialog`) — included in `gSAFE/` |
| [qpdf](https://github.com/qpdf/qpdf) | Optional — merging rendered PDF with an underlay PDF |

---

## License

GsDocEdit is released under the **GNU General Public License v2**.  

**Author:** Deák Péter (hyper80@gmail.com)  
**License:** GPLv2  

