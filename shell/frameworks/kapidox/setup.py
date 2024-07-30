from setuptools import setup, find_packages

setup(
        name='kapidox',
        version='6.5.0',
        description='KDE API documentation generation tools',
        maintainer='Olivier Churlaud',
        maintainer_email='olivier@churlaud.com',
        url='https://invent.kde.org/frameworks/kapidox',
        packages=find_packages(),
        python_requires=">=3.8",
        scripts=['kapidox/depdiagram_generate_all'],
        entry_points={
            "console_scripts": [
                "kapidox-generate = kapidox.kapidox_generate:main",
                "kapidox-depdiagram-prepare = kapidox.depdiagram_prepare:main",
                "kapidox-depdiagram-generate = kapidox.depdiagram_generate:main",
            ],
        },
        install_requires=["doxypypy", "doxyqml", "requests", "jinja2", "pyyaml"]
    )
