# GNOME PWA list

This repository is a place to maintain a list of Progressive Web Apps that are
suitable for inclusion with GNOME Software. The goals of this initiative are:

1. To facilitate the discovery of useful apps by pre-populating a list of web
   apps in GNOME Software
2. To enable users to take advantage of the powerful features of PWAs, such as
   offline functionality and desktop integration
3. To make the GNOME platform competitive with other platforms which already
   make PWAs first-class citizens

There is more context available in the discussion on [this issue](https://gitlab.gnome.org/GNOME/gnome-software/-/issues/1575).

## Criteria for inclusion

To be eligible for inclusion, PWAs must:

- either be released under an open source license (see below) or appear in the
  popular apps category [here](https://www.findpwa.com/list/top-apps) or [here](https://appsco.pe/toplist)
- be served over HTTPS
- include a [web app manifest](https://developer.mozilla.org/en-US/docs/Web/Manifest) with an
  icon, name, and a `display` value of `fullscreen`, `standalone`, or
  `minimal-ui`. The app having an Install button in Chrome means the app meets
  and exceeds these criteria.
- be fully functional in GNOME Web
- not duplicate a native (non-Electron) app available for GNOME

## Process for adding to the list

1. Determine that the app meets all the criteria above.
2. Gather the information for each of the columns listed below.
2. If the license is [FSF-approved or OSI-approved](https://spdx.org/licenses/),
   the app will go in `gnome-pwa-list-foss.csv`, otherwise `gnome-pwa-list-proprietary.csv`.
4. Add a new line in the format "URL,license,categories,content-rating,adaptive,custom-summary" to
   either CSV file as appropriate.
5. Re-build the AppStream xml; see below.
6. Open a merge request [here](https://gitlab.gnome.org/mwleeds/gnome-pwa-list/-/merge_requests).

| Column  | Example value | Accepted values |
|---------|---------------|-----------------|
| URL     | https://app.diagrams.net/ | the starting URL of the app |
| license | Apache-2.0 | the license(s) of the client side software in a format conforming to the AppStream [spec](https://www.freedesktop.org/software/appstream/docs/chap-Metadata.html#tag-project_license) |
| categories | Graphics;Office | one or more main or additional categories as defined [here](https://specifications.freedesktop.org/menu-spec/latest/apa.html) |
| content-rating | social-info=intense;social-audio=moderate | a set of key-value pairs which are OARS 1.1 attributes and values; see [here](https://hughsie.github.io/oars/generate.html). "social-info=none" is appropriate if the service does not even store IP addresses |
| adaptive | adaptive | either "adaptive" or "not-adaptive" depending on whether mobile devices are supported |
| custom-summary | "Telegram web app based on the open API" | a one-sentence summary to use instead of the one in the manifest, or an empty string |

## Re-generating AppStream xml

The script `pwa-metainfo-generator.py` can take the CSV files mentioned above
and generate AppStream metainfo for them, which enables software centers to
display them with rich metadata such as screenshots.

1. Install the `requests` and `bs4` python3 modules using pip3 or your package manager
2. To regenerate the FOSS xml: `python3 pwa-metainfo-generator.py gnome-pwa-list-foss.csv`
3. To regenerate the proprietary xml: `python3 pwa-metainfo-generator.py gnome-pwa-list-proprietary.csv`
4. You can check the validity using e.g. `appstreamcli validate
   gnome-pwa-list-foss.xml`. It will complain about missing descriptions but
   GNOME Software handles those gracefully.

