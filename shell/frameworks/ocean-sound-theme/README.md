# Ocean Sound Theme

Ocean Sound Theme for Plasma

## Folders

- **ocean**: Contains the sounds that are used in the sound theme.
- **Project Files**: Contains the LMMS project file and the Audacity macro used for trimming the sounds.
- **LICENSES**: Contains the terms of all of the licenses on this repository.

## Intended usage

Each sound is intended to be used in a specific situation:

- **theme-demo**: A preview of the sound theme.

- **desktop-login**: Play this sound after the computer boots up and the Plasma session starts.

- **desktop-logout**: Play when exiting the Plasma session.

- **service-login**: Play when logging into an application. (e.g. Instant Messaging app).

- **service-logout**: Play when logging out of application. (e.g. Instant Messaging app).

- **audio-volume-change**: Feedback sound when changing volume.

- **dialog-warning**: Play when warning the user about something (e.g. "You have unsaved changes" dialog).

- **trash-empty**: Play after permanently deleting the contents of the trash folder.

- **message-new-instant**: Play for incoming messages.

- **dialog-question**: Play when asking the user a question (e.g. Dialog: Do you want to save?).

- **power-plug**: Play when plugging in the charger on the laptop or phone.

- **power-unplug**: Play when unplugging the charger.

- **device-added**: Play when a new device is inserted (e.g. New external drive, mouse, or bluetooth dongle).

- **device-removed**: Play when disconnecting a device.

- **battery-caution**: Play when the battery reaches a low level (e.g. 15%).

- **battery-low**: Play when the battery charge reaches a very low level and the computer is about to turn off (e.g. 5%).

- **battery-full**: Play when the battery charge reaches 100%.

- **dialog-information**: Play when giving the user some information (e.g. Dialog: You hid the title bar. Press ALT+F3 to enable it again).

- **bell-window-system**: Use when you need the computer to beep for some reason (e.g. When Audible bell is enabled in the accessibility settings).

- **outcome-success**: Used for actions with a successful outcome.

- **outcome-failure**: Used for actions that fail (Unlike errors, this is more generic. E.g. Can be used in a game).

- **dialog-error**: Play when an error occurs (e.g. Could not save file).

- **dialog-error-serious** and **dialog-error-critical**: Play when an unrecoverable error happens (e.g. The program has crashed).

- **completion-partial**: Play when text autocompletion only completes some words

- **completion-success**: Play when text autocompletion completes all words

- **completion-fail**: Play when text autocompletion fails to complete any words

- **completion-rotation**: Play when text autocompletion reaches the end of the page and starts from the top again

- **phone-incoming-call**: Play when receiving calls

- **alarm-clock-elapsed**: Play for alarms (e.g. timer or alarm clock)

- **complete-media-burn**: Play when optical media completes burning

- **complete-media-error**: Play when the optical media burning proccess is unsuccessful

- **media-insert-request**: Play when requesting the user to insert media

- **game-over-winner**: Play when the player wins in a game

- **game-over-loser**: Play when the player loses in a game

- **message-contact-in**: Play when someone in a instant messaging app logs in

- **message-contact-out**: Play when someone in a instant messaging app logs out

- **message-sent-instant**: Play when sending a message in an instant messaging app

- **message-highlight**: Play for a message in an instant messaging app that is important

- **message-attention**: Play for a message in an instant messaging app that needs attention

If you need a sound which is not provided by this pack, try to use Oxygen sounds as a fallback (e.g. minimize window sounds).

## If anyone wants to make more sounds or modify the existing ones, here's some important observations:

- All sounds must adhere to the sound naming spec: https://specifications.freedesktop.org/sound-naming-spec/sound-naming-spec-latest.html

- I'd love to hear an improved version 2 made by someone in the community, so if you want to do that, follow the instructions below. :D

- Use the Flatpak version of LMMS to get all plugins working.

- Most instruments are based on LMMS's presets, but they're (mostly) heavily modified. Getting familiar with ZynAddSubFX is a good starting point.

- This sound theme is meant to sound soft, so if you want to make more sounds based on this, I suggest increasing the attack, release, and adding reverb (but not too much, unless we're talking about the login sound). This will help you keep the whole theme consistent.

- This sound theme is NOT meant to be modern or minimalistic. Plasma itself is not minimalistic, so it wouldn't make sense to make a minimalistic sound theme. My inspirations were KDE 3 and Windows Vista sounds.

- You may notice some tracks are disabled. The reason for that is that at some point I made the sound with the disabled instrument but then changed my mind. I kept them as backup. There's also an unused instrument for the "trash" sound. That instrument didn't fit very well with the rest of the theme, but I chose to leave it there just in case.

- As of the date I'm writing this, ZynAddSubFX does not seem to allow per-note stereo mixing. Use automation tracks to control the Pan value instead.

- You may notice there's always a detuned version of each sound. I did this to make the instruments sound fuller. The "Ultra Detune" in the login sound is a stylistic choice meant to make the sound feel more nostalgic, like a song playing from an old vinyl disc. Have you ever noticed the pitch changes as the disc wobbles and the speed slows down? :P

- Pay attention to phase cancellation and always test the sounds in a mono speaker. Most sounds (but not all, like a couple of the "completion" sounds) are totally fine in that regard. If you make changes to this project, make sure it's still okay. If you make new sounds and you wanna make sure there's no phasing problems, you can do the following test after exporting all the sounds into one file (I'm using Audacity 3.1.3):

1. Normalize the track to something consistent like -12dB. Effects >> Normalize - Select "Remove DC offset" and "Normalize peak amplitude to". This is not the level you'll want for the final sounds, but you need a baseline loudness level to be able to compare the volume.
2. Make two copies of the main track so you can compare both (Select, copy, click outside and paste)
3. Split the stereo channels in one of the tracks (leave the other as a backup). Click on the track name over the "Silence" and "Solo" buttons and click on "Split stereo track" (**DO NOT** select "Split Stereo to Mono", that will make two identical tracks and when you combine them again you WILL get phase cancelation and lose the stereo panning, always use "Split stereo track")
4. Select one of the audio channels and use the "Invert" effect. Effects > Invert
5. Combine both channels again into one stereo track by clicking on the same place as step 3 and selecting "Make stereo track"
6. Mix the tracks down to mono (both the inverted and the backup). Go to Tracks >> Mix >> Mix Stereo Down to Mono
7. Solo your inverted track, play it. Then do the same with the original. Check which version has the highest volume. Pay attention to the dB meter. The version suffering with phase cancellation will have a lower volume, and the spectrometer will also show that difference. You'll also notice some instruments sound muffled or completely disappear in the problematic version.

There's also the "Invert" effect in LMMS but I noticed I get more phase cancellation problems with it enabled.

- The LMMS export process seems to be non-deterministic, which means that every time you export a sound it can create a slightly different waveform. I noticed sometimes one side of the stereo track is slightly more silent, or sometimes I get a popping noise. At least with LMMS 1.2.2, there's a popping sound at the start of every sound when you export them. To work around that, I added some empty space before each sound, that way the popping gets reduced, but it doesn't go away in every case. If I re-export a few times the pop can go away. There also have been situations where the popping was only noticed after normalizing the sounds in post, so I had to re-export the sounds again (!!!). Keep that in mind and always remember to include the empty space while exporting to avoid the popping sound! You can remove the silence in post. Also remember to check the levels of each channel and, if the volume is too different between them, you can re-export or use the Loudness Normalization plugin in Audacity to normalize each channel independently.

- In post, all sounds got normalized in Audacity using the ReplayGain plugin set at -3dB and I also manually edited the ending of a few sounds to add a Fade Out effect and remove any unwanted artifacts, like noise or clicks with the DeClicker plugin. Download ReplayGain: https://forum.audacityteam.org/t/replaygain-plug-in/22589; Download DeClicker: https://forum.audacityteam.org/t/updated-de-clicker-and-new-de-esser-for-speech/34283. You can find instructions about how you can install a Nyquist plugin here: https://manual.audacityteam.org/man/nyquist_plug_in_installer.html

- In LMMS I exported the sounds at 96000hz 32-bit .wav (for greater flexibility while editing) and when editing I converted them in Audacity to 48000hz 16bit WAV. **Important:** Do not export directly to OGA in Audacity. For some reason, even at the highest quality setting it produces very low bitrate sounds, at around 60-70kbps. Use the following ffmpeg command in a terminal to convert all WAV sounds in the current directory to OGA, targeting 320kbps VBR encoding at 48000hz: `for i in *.wav; do ffmpeg -i "$i" -c:a libvorbis -q:a 9 -ar 48000 "${i%.*}.oga"; done`

## Automate cutting and removing silence from sound using this Audacity macro

Below is the macro I used to automate part of the editing process. It sets the project to the right sample rate, cuts the beginning of each sound (leaving a little bit of silence at the beginning) and then removes silence at the end. To use it, just paste it in a .txt file, save, and import it into the macros window in Audacity. Then, import all tracks and run the macro. Note that I hard-coded the amount of tracks it will select, so, if it causes any trouble, change the number in TrackCount="20" to the number of tracks you imported into Audacity.

SetProject:Rate="48000"
Select:End="2" Mode="Set" RelativeTo="ProjectStart" Start="0" Track="0" TrackCount="20"
Delete:
SelectAll:
TruncateSilence:Action="Truncate Detected Silence" Compress="50" Independent="0" Minimum="1" Threshold="-70" Truncate="0"
