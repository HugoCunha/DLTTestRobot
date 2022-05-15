# DLTTestRobot

DLTTestRobot is used to run Test sripts to automate tests in combination with the DLt Viewer.

![Image of DLTTestRobot](https://github.com/alexmucde/DLTTestRobot/blob/main/doc/images/DLTTestRobot.jpg)

For further information about DLT visit the DLT Viewer project:

https://github.com/GENIVI/dlt-viewer

### Features

* Init status

## Protocol with DLT Viewer Plugin DLTTestRobot

The DLTTestRobot application sets up a TCP connection at Port 4490 with the DLT Viewer plugin.
The protocol is based on ASCII commands. Each command is terminated by a newline '\n'.
The follwoing commands are currently supported:

* filter clear
* filter add <ecuId> <appId> <ctxId>
* injection <ecuId> <command>
* newFile <filename>
* openFile <filename>
* saveAsFile <filename>
* reopenFile
* clearfile
* quitDltViewer
* marker
* connectAllEcu
* disconnectAllEcu

The DLt Viewer plugin sends back the filtered DLT messages in the format ending with a newline '\n':

<ecuId> <appId> <ctxId> <decoded payload>

## Fileformat of test description *.dtr

All simulation and test commands are written into a text file.
Multiple tests are defined in a single file.
All empty lines and lines beginning with a ';' character will be ignored as comments

The following general commands are used

* version <versionnumber>

The following commands are used in each test

* test id <name_without_seperators>
* test description <extended description>
* test repeat <number>
* test begin
* <test command1>
* <test command2>
* <test commandx>
* test end

The following test commands inside a test can be used:

* filter clear
* filter add <ecuId> <appId> <ctxId>
* injection <ecuId> <command>
* sleep <time in ms>
* find equal <time in ms> <ecuId> <appId> <ctxId> <find text>
* find greater <time in ms> <ecuId> <appId> <ctxId> <valueId> <value>
* find smaller <time in ms> <ecuId> <appId> <ctxId> <valueId> <value>

## Installation

To build this SW the Qt Toolchain must be used.

## Usage

* DLTTestRobot.exe [options] configuration

* Options:
*  -?, -h, --help          Help
*  -v, --version           Version
*  -a                      Autostart Communication

* Arguments:
*  configuration           Configuration file

## Contributing

Contibutions are always welcome! Please provide a Pull Request on Github.

https://github.com/alexmucde/DLTTestRobot

## Donation

If you find this SW useful and you want to donate my work please select one of the following donations:

Paypal Donation:

[![Donations](https://www.paypalobjects.com/en_US/DK/i/btn/btn_donateCC_LG.gif)](https://www.paypal.com/donate?hosted_button_id=YBWSNXYWJJP2Q)

Github Sponsors:

[:heart: Sponsor](https://github.com/sponsors/alexmucde)

## Changes

v0.1.2:

* All tests enabled by default
* Connect and Disconnect all ECUs in DLT Viewer
* New command to measure power consumption and write to report summary
* Output found DLT message
* Fixed DLT find single word
* DLT file output also in reports directory
* Marker when test fails
* Test version output
* Test report with summary in folder reports
* One test report for all test jobs
* First version of test report
* Rearange UI
* Show list of commands

v0.1.1:

* Fixed stopping tests
* Update build script to Qt 5.15.2 and MSVC 2019

v0.1.0:

* Fix DLT Test output

v0.0.2:

* Automatic reconnect DLTTestRobot

v0.0.2:

* Repeat single test
* Run all tests
* Repeat all tests
* Stop tests

v0.0.1:

* Initial version

## Copyright

Alexander Wenzel <alex@eli2.de>

This code is licensed under GPLv3.
