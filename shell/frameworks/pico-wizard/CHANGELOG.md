# Changelog
> All notable changes to this project will be documented in this file.
> This project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]
### Added
  - Initial implementation of Locale module
  - Support for Plasma Bigscreen interface via Qt FileSelectors
  - Added Mycroft enablement module for Plasma Bigscreen
  - Added support for HDMI CEC module for Plasma Bigscreen
  - Better input method hints and improvements to Plasma Bigscreen key navigation interface

## [v0.1.0] - 2021-06-22
### Added
  - First run wizard
  - Functionality to enable/disable modules from config file [`/etc/pico-wizard/pico-wizard.conf`]
  - Available Modules : Welcome, Timezone, Wifi, User, Finish
    - Timezone Module :
      - List available timezones
      - Set selected timezone
    - Wifi Module :
      - List available wirelexx connections and sort by signal strength
      - Connect with password
    - User Module :
      - Create user based on input
      - Configurable password type : alphanumeric / digitsonly
    - Finish Module :
      - Run user scripts from `/etc/pico-wizard/scripts.d/`
      - Run finish hook after completion of user scripts `/etc/pico-wizard/scripts.d/finish.hook`
  - Functionality to have custom user modules
  - Functionality to run user scripts on finish module
  - Use policy-kit to run commands / scripts with elevated permissions
  - Configurable log level
  - Logging to journald
