/**
 * The plain Clay rows both faces in the family build their settings page out of.
 *
 * Neither face uses the shared page template, since a layout builder and a theme editor need
 * their own rows in their own order. They still ask most of the same questions, so the little
 * builders for a select, a heading and a toggle live here rather than being written out twice
 * and drifting apart.
 *
 * VIBE_OPTIONS is here rather than beside the rows that read it because three separate sections
 * want it: the two bluetooth pickers and the hourly chime.
 */

import type { ClayItem, ClayOption } from './types';

/**
 * A Clay select row. The description is optional so a self-evident picker can skip it.
 *
 * @param messageKey The key the page sends the watch.
 * @param label The row's title.
 * @param options What the picker offers.
 * @param def The value a fresh install starts on.
 * @param description The help line under the row, left off when the label says it all.
 * @return The row, ready to drop into a section.
 */
export function select(
  messageKey: string,
  label: string,
  options: ClayOption[],
  def: string | number,
  description?: string
): ClayItem {
  const item: ClayItem = {
    type: 'select',
    messageKey: messageKey,
    label: label,
    defaultValue: def,
    options: options,
  };
  if (description) {
    item.description = description;
  }
  return item;
}

/**
 * A section heading row.
 *
 * @param text The heading to show.
 * @return The row, ready to drop into a section.
 */
export function heading(text: string): ClayItem {
  return { type: 'heading', defaultValue: text };
}

/**
 * A Clay on/off toggle row.
 *
 * @param messageKey The key the page sends the watch.
 * @param label The row's title.
 * @param description The help line under the row.
 * @param def Whether a fresh install starts it on.
 * @return The row, ready to drop into a section.
 */
export function toggle(messageKey: string, label: string, description: string, def: boolean): ClayItem {
  return {
    type: 'toggle',
    messageKey: messageKey,
    label: label,
    description: description,
    defaultValue: def,
  };
}

/** The buzz a plain vibration picker offers. The values line up with the watch's own VibeChoice. */
export const VIBE_OPTIONS: ClayOption[] = [
  { label: 'None', value: 0 },
  { label: 'Short', value: 1 },
  { label: 'Long', value: 2 },
  { label: 'Double', value: 3 },
];

export default { select, heading, toggle, VIBE_OPTIONS };
