/**
 * The Clay settings page for this face.
 *
 * Built from the shared template (see paf/ts/pkjs/config-builder.ts), tuned with the
 * desktop and editor theme pickers, a compact date default that fits the terminal panel,
 * and the battery section this face's readout needs.
 */
import buildConfig from '../../paf/ts/pkjs/config-builder';
import previews from './clay/previews.g';

export default buildConfig({
  heading: 'Runtime Settings',
  intro: 'Pick a desktop for the watchface, then set up the weather so its readouts have something to show.',
  // the Desktop dropdown leads, then the look for the chosen desktop with a shot of it, the way
  // Sketchbook's scene and palette pickers sit. the theme picker is spelled out here rather than
  // handed to buildConfig's theme shorthand, since that renders it first and cannot hide it
  appearanceItems: [
    {
      'type': 'select',
      'messageKey': 'APPEARANCE_DESKTOP',
      'label': 'Desktop',
      'description': 'Which computer the watchface is drawn as. Its look and a picture of it are below.',
      'defaultValue': 0,
      'options': [
        { 'label': 'VSCode IDE', 'value': 0 },
        { 'label': 'Desktop 95', 'value': 1 },
        { 'label': 'Desktop 2000', 'value': 2 },
        { 'label': 'DOS Norton Commander', 'value': 3 },
        { 'label': 'DOS Turbo C++ IDE', 'value': 4 },
        { 'label': 'NetBeans IDE', 'value': 5 },
      ],
    },
    // only VS Code has themes, so the picker hides for the other desktops and keeps its choice
    {
      'type': 'themePreview',
      'messageKey': 'APPEARANCE_THEME',
      'visibleWhen': { 'key': 'APPEARANCE_DESKTOP', 'in': [0] },
      'label': 'Editor Theme',
      'description': 'The colours of the VS Code desktop.',
      'defaultValue': 0,
      'options': [
        { 'label': 'Dark', 'value': 0 },
        { 'label': 'Light', 'value': 1 },
        { 'label': 'Terminal', 'value': 2 },
        { 'label': 'Cyberpunk', 'value': 3 },
        { 'label': "Synthwave '84", 'value': 4 },
        { 'label': 'Mono (Black & White)', 'value': 5 },
      ],
      'previews': previews.theme,
    },
    // every other desktop has a Classic, a Dark, and a Terminal look, kept apart from the VS Code
    // theme so each choice survives a switch between desktops. Dark saves 2, since it came after
    // Terminal, so a saved Classic or Terminal keeps its meaning. the shots under it follow the
    // Desktop picker
    {
      'type': 'themePreview',
      'messageKey': 'APPEARANCE_DESKTOP_THEME',
      'visibleWhen': { 'key': 'APPEARANCE_DESKTOP', 'notIn': [0] },
      'label': 'Desktop Theme',
      'description': "Classic keeps the desktop's own colours. Dark darkens its chrome and keeps its accents. Terminal draws it in green on black.",
      'defaultValue': 0,
      'options': [
        { 'label': 'Classic', 'value': 0 },
        { 'label': 'Dark', 'value': 2 },
        { 'label': 'Terminal', 'value': 1 },
      ],
      'previews': previews.desktopTheme,
      'previewsByKey': 'APPEARANCE_DESKTOP',
    },
  ],
  bluetooth: {
    description: 'Show a bluetooth glyph beside the battery. It is lit while the phone is connected and slashed when the link drops.',
  },
  location: { gpsDefault: false },
  weather: {},
  temperature: {},
  hourlyVibe: {},
  date: {
    default: '%b %d',
  },
  steps: {
    label: 'Stats Readout',
    description: 'What the steps readout shows: step count, or distance walked.',
  },
  battery: {
    description: 'What the battery readout shows: the icon, the percentage, or both. DOS Norton Commander and DOS Turbo C++ IDE give the percentage a line of its own, so it always shows there.',
  },
});
