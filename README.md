# Crashday ASI Pluginkit
This a pack of various ASI plugins for Crashday in one repository.

This repository was created so that instead of creating many small repositories for plugins, there would be one big one with different small plugins. Some plugins have their own repositories, you can read about it below.

## Plugins list
- [Widescreen Fix](#widescreen-fix)
- [Adjustable Afterburner](#adjustable-afterburner)
- [Tutorial Events](#tutorial-events)
- [Car Lights On/Off](#car-lights-onoff)
- [Improved Transmission System](#improved-transmission-system)
- [Discord Rich Presence](#discord-rich-presence)

## Widescreen Fix
Date Release: _September 25th, 2024_

Current version: _1.1.0_

Supported game versions: _1.0, 1.1, 1.2_

An ASI plugin to improve widescreen support for Crashday (2006). This fix automatically corrects the image in the game based on the aspect ratio of your monitor. Also, as a bonus, this plugin increases the drawing distance.

This plugin has own separated repository. Click [here](https://github.com/St1ngLeR/CD_WidescreenFix/)

## Adjustable Afterburner
Date Release: _September 30th, 2024_

Current version: _1.0.0_

Supported game versions: _1.1, 1.2_

This ASI plugin returns ability to change afterburner values as it was made in 1.0 version.

[Download the latest plugin version](https://github.com/St1ngLeR/Crashday-ASI-Pluginkit/releases/tag/CD_Aftbur_v1.0.0)

[Go to plugin source code folder](../../tree/master/CD_Aftbur)

## Tutorial Events
Date Release: _October 4th, 2024_

Current version: _1.0.0_

Supported game versions: _1.2_

This plugin restores tutorial events that were "hidden" in the game release. The tracks to the events were recreated from screenshots. Added support for English and Russian languages ​​for these events.

[Download the latest plugin version](https://github.com/St1ngLeR/Crashday-ASI-Pluginkit/releases/tag/CD_TutorialEvents_v1.0.0)

[Go to plugin source code folder](../../tree/master/CD_TutorialEvents)

## Car Lights On/Off
Date Release: _November 28th, 2024_

Current version: _1.0.1_

Supported game versions: _1.2_

This plugin restores cut feature of turning car lights on/off. The default key is "L". To change the key you need to create file `CD_CarLightsOnOff.ini` in the folder with installed plugin with contents:
```
[CD_CarLightsOnOff]
KeyActivation=*your value here*
```
Check decimal [Virtual Key codes](http://cherrytree.at/misc/vk.htm) to use them for this plugin.

[Download the latest plugin version](https://github.com/St1ngLeR/Crashday-ASI-Pluginkit/releases/tag/CD_CarLightsOnOff_v1.0.1)

[Go to plugin source code folder](../../tree/master/CD_CarLightsOnOff)

## Improved Transmission System
Date Release: _February 19th, 2025_

Current version: _1.1.0_

Supported game versions: _1.2_

This plugin restores the ability to play with manual transmissions using a new menu. It also allows you to improve some aspects of gear shifting.
```
[CD_GearBox]
Engine.EnableNeutralGear=true - Adds switching to neutral gear with automatic transmission. This option also fixes the "speedbug" in the game.
HUD.EnableGearIndicator=true - Adds a gear indicator in speedometer along with the transmission type (AT or MT).
KeyEvent.ShiftDown=*your value here* (Default - 162)
KeyEvent.ShiftUp=*your value here* (Default - 160)
```
Check decimal [Virtual Key codes](http://cherrytree.at/misc/vk.htm) to use them for this plugin.

[Download the latest plugin version](https://github.com/St1ngLeR/Crashday-ASI-Pluginkit/releases/tag/CD_GearBox_v1.1.0)

[Go to plugin source code folder](../../tree/master/CD_GearBox)

## Discord Rich Presence
Date Release: _March 19th, 2025_

Current version: _1.0.0_

Supported game versions: _1.2_

This plugin allows you to display information about the game in Discord. In the Discord profile during the game, the following is shown: being in the multiplayer lobby, in the track editor, the choice of car (with car name) and information about the current race (mode, track name and other information).

[Download the latest plugin version](https://github.com/St1ngLeR/Crashday-ASI-Pluginkit/releases/tag/CD_DiscordRPC_v1.0.0)

[Go to plugin source code folder](../../tree/master/CD_DiscordRPC)
