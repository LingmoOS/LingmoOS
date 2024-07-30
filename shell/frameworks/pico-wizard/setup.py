# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: 2021 Anupam Basak <anupam.basak27@gmail.com>

import setuptools

setuptools.setup(
    name="pico-wizard",
    version="0.1.0",
    author="Anupam Basak",
    author_email="anupam.basak27@gmail.com",
    description="A Post Installation Configuration tool",
    long_description="A Post Installation Configuration tool for Linux",
    long_description_content_type="text/plain",
    scripts=[
        "files/pico-wizard-script-runner",
        'files/pico-wizard-session',
        'files/pico-wizard-wrapper',
    ],
    data_files = [
        ('share/wayland-sessions', ['files/pico-wizard-session.desktop']),
        ('/etc/pico-wizard', ['files/pico-wizard.conf']),
    ],
    entry_points={
        "console_scripts": [
            "pico-wizard = PicoWizard.__main__:__main__",
        ]
    },
    url="https://invent.kde.org/plasma/pico-wizard/pico-wizard",
    project_urls={
        "Bug Tracker": "https://bugs.kde.org/enter_bug.cgi?product=PicoWizard",
        "Documentation": "https://invent.kde.org/plasma/pico-wizard/pico-wizard",
        "Source Code": "https://invent.kde.org/plasma/pico-wizard/pico-wizard",
    },
    packages=setuptools.find_packages(),
    classifiers=[
        "Programming Language :: Python :: 3",
        "License :: OSI Approved :: GNU General Public License v3 or later (GPLv3+)"
    ],
    license="GPLv3+",
    install_requires=[
        "systemd-python",
        "PySide2",
    ],
    python_requires=">=3.6",
    package_data = {
        "": [
            "*.qml",
            "**/*.qml",
            "**/*.svg",
            "**/*.svg.license",
            "**/*.sh",
            "**/qmldir",
            "PicoWizard/**/*.svg"
        ]
    },
    include_package_data=True,
) 
