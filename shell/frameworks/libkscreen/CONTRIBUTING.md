# Contributing to libkscreen

 - [Code of Conduct](#code-of-conduct)
 - [Submission Guideline](#submission-guideline)
 - [Commit Message Guideline](#commit-message-guideline)
 - [Contact](#contact)

## Code of Conduct
The [KDE Community Code of Conduct][kde-coc] is applied.

You can reach out to the [Commmunity Working Group][community-working-group] if you have questions about the Code of Conduct or if you want to get help on solving an issue with another contributor or maintainer.

## Issues
Please reports issues and suggestions using [KDE's bugzilla][bugzilla]

## Submission Guideline
The project follows the [Frameworks Coding Style][frameworks-style].

All non-trivial patches need to go through [code review][gitlab-reviews].

Commits are applied on top of master and cherry-picked to release branches if appropriate. Use `-x` when cherry-picking. Larger changes should be split up into smaller logical commits.

libkscreen is released as part of Plasma. See the [Plasma schedule][plasma-schedule] for information on the release schedule.

## Commit Message Guideline
Please follow the [KDE Commit Policy][commit-policy]. In particular please make sure to use appropriate [commit keywords][commit-policy-keywords].

## Contact
Real-time communication about the project happens on the IRC channel `#plasma` on irc.libera.chat and the bridged Matrix room `#plasma:kde.org`.

Emails about the project can be sent to the [plasma-devel][plasma-devel] mailing list.

[kde-coc]: https://kde.org/code-of-conduct
[community-working-group]: https://ev.kde.org/workinggroups/cwg.php
[frameworks-style]: https://community.kde.org/Policies/Frameworks_Coding_Style
[gitlab-reviews]: https://invent.kde.org/plasma/kscreen
[plasma-schedule]: https://community.kde.org/Schedules/Plasma_5
[commit-policy]: https://community.kde.org/Policies/Commit_Policy
[bugzilla]: https://bugs.kde.org/describecomponents.cgi?product=KScreen
[commit-policy-keywords]: https://community.kde.org/Policies/Commit_Policy#Special_keywords_in_GIT_and_SVN_log_messages
[plasma-devel]: https://mail.kde.org/mailman/listinfo/plasma-devel
