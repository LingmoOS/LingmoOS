# KUserFeedback

Framework for collecting feedback from application users via telemetry and targeted surveys.

## Telemetry

* Extensible set of data sources for telemetry.
* Full control for the user on what data to contribute.

## Surveys

* Distribute surveys and offer users to participate in them.
* Survey targeting based on telemetry data.
* Allow the user to configure how often they want to participate in surveys.

## Components

This framework consists of the following components:
* Libraries for use in applications.
* QML bindings for the above.
* A server application.
* A management and analytics application.

## Integration
To use this framework in your application, on a high level you have to do
the following steps inside your program:
* Create an instance of KUserFeedback::Provider for the entire lifetime of your
  application, and configure it as described in its documentation.
* Add one or more data sources to the provider, choosing from the standard
  built-in ones, or by adding custom ones deriving from KUserFeedback::AbstractDataSource.
* Add a way to notify your users about surveys, and to encourage them to contriubute
  in the first place, by hooking a suitable user interface up with the corresponding
  signals in UserFeedback::Provider. KUserFeedback::NotificationPopup provides a
  reference widget for this purpose.
* Add a way for your users to configure what data to contribute, and how often
  to be bothered by surveys. KUserFeedback::FeedbackConfigDialog provides a
  reference implementation of a configuration dialog for this.

## Deployment
For information on how to deploy the server parts, please see the
[INSTALL](https://commits.kde.org/kuserfeedback?path=INSTALL) file.

## Usage Documentation
Information on how to use KUserFeedback from a product manager point of view can
be found in the user manual of UserFeedbackConsole. Start UserFeedbackConsole and
press `F1` to get to the user manual.

## Frequently Asked Questions

_Why does the telemetry data contain no unique user/installation/machine id?_

KUserFeedback is designed to be compliant with [KDE Telemetry Policy](https://community.kde.org/Policies/Telemetry_Policy),
which forbids the usage of unique identification.
If you are using KUserFeedback outside of the scope of that policy, it's of course
possible to add a custom data source generating and transmitting a unique id.

_How can duplicated submissions be detected without unique identification?_

KUserFeedback addresses this by using a fixed time interval for telemetry data submission.
If for example Provider::submissionInterval is set to one week, aggregating data in one
week intervals ensures that there's only at most one sample included from each installation.
The other way around, setting Provider::submissionInterval at least as high as the average
usage interval of your application ensures a substantial amount of installations have
submitted a sample in this interval. A frequently used application such e.g. an email
program can therefore get good results with a smaller value than e.g. a special purpose
application.

_How can developments of a user be tracked without unique identification?_

If you want to observe how usage patterns change as your users gain more experience
with the application, there are multiple ways to do that:
- Include a UsageTimeSource, and correlate the relevant usage data with how long
  the application has been used.
- It is possible to track arbitrary historic development using custom data sources
  via AbstractDataSource::reset. This method is called after each successful submission,
  allowing you to implement a "memory" for your data source, and e.g. submit differential
  rather than absolute values.
