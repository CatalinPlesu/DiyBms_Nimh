// Enums
const TE_DataType = {
  INT8: 0,
  INT16: 1,
  INT32: 2,
  UINT8: 3,
  UINT16: 4,
  UINT32: 5,
  FLOAT: 6
};

// Data structure for variables
class Variable {
  constructor(type, name, currentValue) {
    this.type = type;
    this.name = name;
    this.currentValue = currentValue;
  }
}

// Arrays for host and guest variables
const hostVariables = [];
const guestVariables = [];

// Logging state and settings for AVR and ESP
let avrLogState = false;
let avrLogRecurrence = 1000;
let avrLogColor = 'blue';
let espLogState = false;
let espLogRecurrence = 1000;
let espLogColor = 'green';

// Function to fetch host variables
function fetchHostVariables() {
  const hostVariablesURL = '/host.json';
  return $.ajax({
    url: hostVariablesURL,
    type: 'GET',
    dataType: 'json'
  });
}

// Function to fetch guest variables
function fetchGuestVariables() {
  const guestVariablesURL = '/guest.json';
  return $.ajax({
    url: guestVariablesURL,
    type: 'GET',
    dataType: 'json'
  });
}

// Function to initialize the shell
function initializeShell() {
  // Fetch host variables
  fetchHostVariables()
    .done(response => {
      hostVariables.length = 0; // Clear existing variables
      for (const variable of response) {
        const { type, name } = variable;
        hostVariables.push(new Variable(type, name));
      }
      console.log('Host variables:', hostVariables);
      initializeGuestVariables();
    })
    .fail(error => console.error('Error fetching host variables:', error));
}

// Function to initialize guest variables
function initializeGuestVariables() {
  // Fetch guest variables
  fetchGuestVariables()
    .done(response => {
      guestVariables.length = 0; // Clear existing variables
      for (const variable of response) {
        const { type, name } = variable;
        const existingVariable = hostVariables.find(v => v.name === name);
        if (existingVariable) {
          console.log(`Variable "${name}" is present on both host and guest MCUs.`);
          console.log('To access the host variable, type the variable name followed by "0".');
          console.log('To access the guest variable, type the variable name followed by "1".');
        }
        guestVariables.push(new Variable(type, name));
      }
      console.log('Guest variables:', guestVariables);
      shell();
    })
    .fail(error => console.error('Error fetching guest variables:', error));
}

// Function to handle log messages
function handleLogMessage(logMessage, logTarget, logColor) {
  const currentTime = new Date().toLocaleTimeString();
  const formattedMessage = `[${currentTime}] ${logMessage}`;

  // Check if the message is different from the previous message
  if (logTarget === 'avr') {
    if (formattedMessage !== avrPreviousLogMessage) {
      avrPreviousLogMessage = formattedMessage;
      console.log('%c' + formattedMessage, `color: ${logColor}`);
    }
  } else if (logTarget === 'esp') {
    if (formattedMessage !== espPreviousLogMessage) {
      espPreviousLogMessage = formattedMessage;
      console.log('%c' + formattedMessage, `color: ${logColor}`);
    }
  }
}

// Function to periodically check and display logs
function checkAndDisplayLogs() {
  setInterval(() => {
    if (avrLogState) {
      // Make a GET request to avr_log.json to fetch AVR MCU logs
      $.get('avr_log.json', function (data) {
        data.forEach(log => {
          const [logMessage, logColor] = log;
          handleLogMessage(logMessage, 'avr', logColor);
        });
      });
    }

    if (espLogState) {
      // Make a GET request to esp_log.json to fetch ESP MCU logs
      $.get('esp_log.json', function (data) {
        data.forEach(log => {
          const [logMessage, logColor] = log;
          handleLogMessage(logMessage, 'esp', logColor);
        });
      });
    }
  }, avrLogRecurrence);
}

// Main interactive shell loop
function shell() {
  const promptText = '>>> ';

  function handleCommand(command) {
    const parts = command.split(' ');
    const action = parts[0];

    switch (action) {
      case 'read':
        const varNameRead = parts[1];
        const varIndexRead = parseInt(parts[2]);

        let targetArrayRead;
        if (varIndexRead === 0) {
          targetArrayRead = hostVariables;
        } else if (varIndexRead === 1) {
          targetArrayRead = guestVariables;
        } else {
          console.log('Invalid index. Please use "0" for host or "1" for guest.');
          break;
        }

        let variableRead = targetArrayRead.find(v => v.name === varNameRead);
        if (!variableRead) {
          // Check if the variable name exists in both host and guest arrays
          const hostVariableIndex = hostVariables.findIndex(v => v.name === varNameRead);
          const guestVariableIndex = guestVariables.findIndex(v => v.name === varNameRead);
          if (hostVariableIndex !== -1 && guestVariableIndex !== -1) {
            console.log(`Variable "${varNameRead}" is present on both host and guest MCUs.`);
            console.log('To access the host variable, type the variable name followed by "0".');
            console.log('To access the guest variable, type the variable name followed by "1".');
            break;
          }
          console.log('Variable not found:', varNameRead);
          break;
        }

        const readUrl = varIndexRead === 0 ? 'avr_read.json' : 'esp_read.json';
        $.get(readUrl, { index: varIndexRead, name: varNameRead }, function (data) {
          console.log(`Read variable "${variableRead.name}":`, data.currentValue);
        });
        break;

      case 'write':
        const varNameWrite = parts[1];
        const value = parts[2];
        const varIndexWrite = parseInt(parts[3]);
      
        let targetArrayWrite;
        if (varIndexWrite === 0) {
          targetArrayWrite = hostVariables;
        } else if (varIndexWrite === 1) {
          targetArrayWrite = guestVariables;
        } else {
          console.log('Invalid index. Please use "0" for host or "1" for guest.');
          break;
        }
      
        let variableWrite = targetArrayWrite.find(v => v.name === varNameWrite);
        if (!variableWrite) {
          // Check if the variable name exists in both host and guest arrays
          const hostVariableIndex = hostVariables.findIndex(v => v.name === varNameWrite);
          const guestVariableIndex = guestVariables.findIndex(v => v.name === varNameWrite);
          if (hostVariableIndex !== -1 && guestVariableIndex !== -1) {
            console.log(`Variable "${varNameWrite}" is present on both host and guest MCUs.`);
            console.log('To access the host variable, type the variable name followed by "0".');
            console.log('To access the guest variable, type the variable name followed by "1".');
            break;
          }
          console.log('Variable not found:', varNameWrite);
          break;
        }
      
        const writeUrl = varIndexWrite === 0 ? 'avr_write.json' : 'esp_write.json';
        $.post(writeUrl, { index: varIndexWrite, name: varNameWrite, value: value }, function (data) {
          console.log(`Successfully wrote variable "${variableWrite.name}" with value:`, value);
        });
        break;

      case 'log':
        const logTarget = parts[1];
        const logState = parts[2];

        switch (logTarget) {
          case 'avr':
            if (logState === 'on') {
              avrLogState = true;
              console.log('Logging turned on for AVR MCU.');
            } else if (logState === 'off') {
              avrLogState = false;
              console.log('Logging turned off for AVR MCU.');
            } else {
              console.log('Invalid state. Please use "on" or "off" to control AVR logging.');
            }
            break;

          case 'esp':
            if (logState === 'on') {
              espLogState = true;
              console.log('Logging turned on for ESP MCU.');
            } else if (logState === 'off') {
              espLogState = false;
              console.log('Logging turned off for ESP MCU.');
            } else {
              console.log('Invalid state. Please use "on" or "off" to control ESP logging.');
            }
            break;

          default:
            console.log('Invalid target. Please specify "avr" or "esp" for logging.');
        }
        break;

      case 'recurrence':
        const logTargetSet = parts[1];
        const recurrenceValue = parseInt(parts[2]);

        switch (logTargetSet) {
          case 'avr':
            avrLogRecurrence = recurrenceValue;
            console.log('AVR log recurrence set to:', recurrenceValue, 'ms');
            break;

          case 'esp':
            espLogRecurrence = recurrenceValue;
            console.log('ESP log recurrence set to:', recurrenceValue, 'ms');
            break;

          default:
            console.log('Invalid target. Please specify "avr" or "esp" for setting log recurrence.');
        }
        break;

      case 'color':
        const logTargetColor = parts[1];
        const color = parts[2];

        switch (logTargetColor) {
          case 'avr':
            avrLogColor = color;
            console.log('AVR log color set to:', color);
            break;

          case 'esp':
            espLogColor = color;
            console.log('ESP log color set to:', color);
            break;

          default:
            console.log('Invalid target. Please specify "avr" or "esp" for setting log color.');
        }
        break;

      case 'exit':
        console.log('Exiting shell...');
        return;

      default:
        console.log('Invalid command. Supported commands: read, write, log, logset, logcolor, exit');
    }

    shell();
  }

  const readline = require('readline');
  const rl = readline.createInterface({
    input: process.stdin,
    output: process.stdout
  });

  rl.question(promptText, command => {
    // Remove the last character if it is '0' or '1' to handle host/guest variable indexing
    const modifiedCommand = command.trim();
    if (modifiedCommand.endsWith('0') || modifiedCommand.endsWith('1')) {
      handleCommand(modifiedCommand.slice(0, -1));
    } else {
      handleCommand(modifiedCommand);
    }
    rl.close();
  });
}

// Start the interactive shell
initializeShell();

// Call checkAndDisplayLogs immediately after initializing the shell
checkAndDisplayLogs();