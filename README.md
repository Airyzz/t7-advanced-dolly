# t7-advanced-dolly

Allows you to store FOV, Focus Distance, Aperture, and Camera Roll in the Dolly Markers in Black Ops III Theater mode

## Controls:
### While scrolling up or down, or pressing +\- hold one of these modifiers to change values
| Key | Adjusts              |
| :---:   | :---:            |
| Shift   | FOV              |
| Ctrl    | Focus Distance   |
| Alt     | Aperture         |
| Nothing | Roll             |

## Building
Requires Visual Studio + [Intel C/C++ compiler](https://marketplace.visualstudio.com/items?itemName=intel-corporation.dpcpponline)

The Intel Compiler is required because `hooks.h` makes use of inline assembly. It would be ideal to rewrite this file to not require a specific compiler!

PRs welcome ;D
