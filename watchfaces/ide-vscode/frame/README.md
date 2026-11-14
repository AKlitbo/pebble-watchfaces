# Frames

`vscode~emery.html` is the VS Code desktop, baked once per `css/theme_<name>.css` sheet, as `frame.config.json` sets up:

```sh
paf gen ide-vscode background --frame vscode --theme all
```

Every other page here is a desktop of its own with one look, and links its own `css/desktop_<name>.css`. That name has no `theme_` in it, so the theme swap leaves the sheet alone. The frame plugin bakes a face with themes one theme at a time, so each desktop is baked under any theme and the PNG copied into place by hand. The theme sheet it adds sets only variables these pages never read.

```sh
paf gen ide-vscode background --frame win95 --theme dark --out targets/bake/win95.png
cp targets/bake/win95~emery.png resources/images/background-win95~emery.png
```

`paf gen ide-vscode all` refuses this face, since it has themes and more than one frame. Bake the VS Code themes and each desktop with the commands above instead.

TODO: bake the desktops through the frame plugin once it bakes themes per frame, and drop the hand copy.
