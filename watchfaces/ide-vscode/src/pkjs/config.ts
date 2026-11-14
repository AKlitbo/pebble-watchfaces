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
  // the Desktop picker leads, then the look for the chosen desktop, each with a shot of the choice
  // under it. the theme picker is spelled out here rather than handed to buildConfig's theme
  // shorthand, since that renders it first and cannot hide it
  appearanceItems: [
    {
      'type': 'themePreview',
      'messageKey': 'APPEARANCE_DESKTOP',
      'label': 'Desktop',
      'description': 'The look of the whole watchface: a VS Code editor, a 95 or 2000 desktop, a DOS file manager, or a DOS code editor.',
      'defaultValue': 0,
      'options': [
        { 'label': 'VS Code', 'value': 0 },
        { 'label': '95', 'value': 1 },
        { 'label': '2000', 'value': 2 },
        { 'label': 'DOS', 'value': 3 },
        { 'label': 'DOS IDE', 'value': 4 },
      ],
      'previews': previews.desktop,
    },
    // only VS Code has themes, so the picker hides for the other desktops and keeps its choice
    {
      'type': 'themePreview',
      'messageKey': 'APPEARANCE_THEME',
      'visibleWhen': { 'key': 'APPEARANCE_DESKTOP', 'in': [0] },
      'label': 'Editor Theme',
      'description': 'Colour scheme for the editor frame and readouts.',
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
    // every other desktop has a Classic and a Terminal look, kept apart from the VS Code theme so
    // each choice survives a switch between desktops. the shots under it follow the Desktop picker
    {
      'type': 'themePreview',
      'messageKey': 'APPEARANCE_DESKTOP_THEME',
      'visibleWhen': { 'key': 'APPEARANCE_DESKTOP', 'notIn': [0] },
      'label': 'Desktop Theme',
      'description': 'The desktop in its own colours, or in green on black.',
      'defaultValue': 0,
      'options': [
        { 'label': 'Classic', 'value': 0 },
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
    description: 'What the battery readout shows: the icon, the percentage, or both. DOS and DOS IDE give the percentage a line of its own, so it always shows there.',
  },
});
