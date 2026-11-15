# Frames

`vscode~emery.html` is the VS Code desktop, baked once per `css/theme_<name>.css` sheet, as `frame.config.json` sets up:

```sh
paf gen ide-vscode background --frame vscode --theme all
```

Every other page here is a desktop of its own, and links its own `css/desktop_<name>.css`. That name has no `theme_` in it, so the theme swap leaves the sheet alone. The sheet sets its colours by job in variables named with the desktop's prefix. `css/desktop_<name>_dark.css` sets them again for the Dark look, and `css/desktop_<name>_terminal.css` sets them in greens for the Terminal look.

The frame plugin bakes a face with themes one theme at a time, so each desktop is baked under a theme and the PNG copied into place by hand. Under `light` the page keeps its Classic colours, since that sheet sets only VS Code's variables. `theme_dark.css` imports every desktop's Dark sheet and `theme_terminal.css` every Terminal sheet, so under `dark` and `terminal` the page takes those looks.

```sh
paf gen ide-vscode background --frame win95 --theme light --out targets/bake/win95.png
cp targets/bake/win95~emery.png resources/images/background-win95~emery.png
paf gen ide-vscode background --frame win95 --theme dark --out targets/bake/win95-dark.png
cp targets/bake/win95-dark~emery.png resources/images/background-win95-dark~emery.png
paf gen ide-vscode background --frame win95 --theme terminal --out targets/bake/win95-terminal.png
cp targets/bake/win95-terminal~emery.png resources/images/background-win95-terminal~emery.png
```

`paf gen ide-vscode all` refuses this face, since it has themes and more than one frame. Bake the VS Code themes and each desktop with the commands above instead.

TODO: bake the desktops through the frame plugin once it bakes themes per frame, and drop the hand copy.
