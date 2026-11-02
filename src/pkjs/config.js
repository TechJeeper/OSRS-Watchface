module.exports = [
  {
    type: 'heading',
    defaultValue: 'Old School RuneScape'
  },
  {
    type: 'text',
    defaultValue:
      'Gielinor on your wrist — Comic Weather density with OSRS chrome. Weather via Open-Meteo (no API key).'
  },

  {
    type: 'section',
    items: [
      {
        type: 'heading',
        defaultValue: 'Time'
      },
      {
        type: 'select',
        messageKey: 'TimeFormat',
        defaultValue: '0',
        label: 'Hour format',
        options: [
          { label: 'System default', value: '0' },
          { label: '12-hour', value: '1' },
          { label: '24-hour', value: '2' }
        ]
      },
      {
        type: 'toggle',
        messageKey: 'LeadingZero',
        label: 'Leading zero on hours',
        defaultValue: true
      },
      {
        type: 'toggle',
        messageKey: 'ShowDate',
        label: 'Show date',
        defaultValue: true
      }
    ]
  },

  {
    type: 'section',
    items: [
      {
        type: 'heading',
        defaultValue: 'Weather'
      },
      {
        type: 'select',
        messageKey: 'TemperatureUnit',
        defaultValue: '0',
        label: 'Temperature unit',
        options: [
          { label: 'Celsius', value: '0' },
          { label: 'Fahrenheit', value: '1' }
        ]
      },
      {
        type: 'select',
        messageKey: 'RefreshMinutes',
        defaultValue: '60',
        label: 'Weather refresh',
        options: [
          { label: '15 minutes', value: '15' },
          { label: '30 minutes', value: '30' },
          { label: '60 minutes', value: '60' },
          { label: '2 hours', value: '120' },
          { label: '3 hours', value: '180' }
        ]
      },
      {
        type: 'toggle',
        messageKey: 'ShowForecast',
        label: '4-hour forecast strip',
        defaultValue: true
      },
      {
        type: 'toggle',
        messageKey: 'ShowTomorrow',
        label: 'Tomorrow high / low',
        defaultValue: true
      }
    ]
  },

  {
    type: 'section',
    items: [
      {
        type: 'heading',
        defaultValue: 'Interface'
      },
      {
        type: 'select',
        messageKey: 'Theme',
        defaultValue: '0',
        label: 'Theme',
        options: [
          { label: 'Classic', value: '0' },
          { label: 'Dark', value: '1' },
          { label: 'Wilderness', value: '2' }
        ]
      },
      {
        type: 'toggle',
        messageKey: 'ShowChips',
        label: 'Steps / heart / battery chips',
        defaultValue: true
      },
      {
        type: 'toggle',
        messageKey: 'ShowHeartRate',
        label: 'Show heart rate',
        defaultValue: true
      },
      {
        type: 'toggle',
        messageKey: 'ShowOrbs',
        label: 'Status orbs',
        description: 'HP≈battery · Prayer≈Bluetooth · Run≈minute',
        defaultValue: false
      },
      {
        type: 'toggle',
        messageKey: 'InvertBw',
        label: 'Invert on black & white',
        defaultValue: false
      }
    ]
  },

  {
    type: 'submit',
    defaultValue: 'Save'
  }
];
