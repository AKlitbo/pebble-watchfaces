/**
 * Which palette group each panel sits in, keyed by module label like module-meta.ts so a lookup
 * survives a numeric id drift.
 *
 * One table serves every Mosaic face, since a panel carries the same label on each. A face only
 * reads the rows for the panels it has. Each face's config.ts merges the group onto its
 * MODULE_OPTIONS, and the layout editor's palette reads it from there.
 */

import type { ModuleGroup } from './types';

const moduleGroups: Record<string, ModuleGroup> = {
  'Digital Clock':     'Clock',
  'Big Hour':          'Clock',
  'Big Minutes':       'Clock',
  'Big Time':          'Clock',
  'Time (1x2)':        'Clock',
  'Analog Clock':      'Clock',
  'Time Zone 1':       'Clock',
  'Beats':             'Clock',
  'Epoch Clock':       'Clock',
  'Julian Date':       'Clock',
  'Date':              'Date',
  'Weekday Dots':      'Date',
  'Week Number':       'Date',
  'Weeks Left':        'Date',
  'Day of Year':       'Date',
  'Days Left':         'Date',
  'Month Grid':        'Date',
  'Day Progress':      'Progress',
  'Hour Progress':     'Progress',
  'Week Progress':     'Progress',
  'Year Progress':     'Progress',
  'Weather':           'Weather',
  'Conditions':        'Weather',
  'Temperature':       'Weather',
  'Feels Like':        'Weather',
  'Hi/Low':            'Weather',
  'Humidity':          'Weather',
  'Precipitation':     'Weather',
  'Dew Point':         'Weather',
  'Wind Speed':        'Weather',
  'Pressure':          'Weather',
  'UV':                'Weather',
  'Forecast (Hourly)': 'Weather',
  'Forecast (4-Day)':  'Weather',
  'Sun':               'Sky',
  'Sunrise':           'Sky',
  'Sunset':            'Sky',
  'Daylight':          'Sky',
  'Moon':              'Sky',
  'Next Moon':         'Sky',
  'Steps':             'Health',
  'Steps Graph':       'Health',
  'Distance':          'Health',
  'Calories':          'Health',
  'Activity':          'Health',
  'Sleep':             'Health',
  'Heart Rate':        'Health',
  'HR Graph':          'Health',
  'Step Goal':         'Goals',
  'Distance Goal':     'Goals',
  'Calorie Goal':      'Goals',
  'Sleep Goal':        'Goals',
  'Activity Goal':     'Goals',
  'Next Event':        'Calendar & Stocks',
  'Timeline':          'Calendar & Stocks',
  'Agenda':            'Calendar & Stocks',
  'Stock':             'Calendar & Stocks',
  'Watchlist':         'Calendar & Stocks',
  'Battery':           'Status',
  'Status':            'Status',
  'Connection':        'Status',
  'Quiet Time':        'Status',
  'Next Alarm':        'Status',
};

export default moduleGroups;
