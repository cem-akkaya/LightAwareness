# Light Awareness Plugin

[![Plugin version number](https://img.shields.io/github/v/release/cem-akkaya/LightAwareness?label=Version)](https://github.com/cem-akkaya/LightAwareness/releases/latest)
[![Unreal Engine Supported Versions](https://img.shields.io/badge/Unreal_Engine-5.1_%7C_5.2_%7C_5.3_%7C_5.4_%7C_5.5-9455CE?logo=unrealengine)](https://github.com/cem-akkaya/LightAwareness/releases)
[![License](https://img.shields.io/github/license/cem-akkaya/LightAwareness)](LICENSE)
![Download count](https://img.shields.io/github/downloads/cem-akkaya/LightAwareness/total)
[![Actively Maintained](https://img.shields.io/badge/Maintenance%20Level-Actively%20Maintained-green.svg)](https://gist.github.com/cheerfulstoic/d107229326a01ff0f333a1d3476e068d)

<img src="https://cemakkaya.com/ImageHost/splash.jpg" alt="plugin-light-awareness" width="100%"/>


## Overview

This is an Unreal Engine plugin to detect light conditions on any actor independent of Light Source Type, Screen Effects and similar.

This plugin is an actor component where snapshots a buffer image from the world with given variables. Process the buffer
image pixels from the memory and brute forces the brightest pixel in the light gem. This plugin is created on ingenious method of [Dark Mod](https://www.thedarkmod.com/main/) indie game,
re-creating approach in a more accurate calculation in Unreal 5+. 

You can define the processing method and efficiency on the component and on the blueprint level depending on the usage.
You can define your own rules in blueprints or C++ to get light status from the component or you can get the buffer image array on the tick to process further calculations
in blueprint or C++.

If you are trying to integrate this component on a custom class, integrate on a parent that is actor or you can just create an actor with LightAwareness component and attach to your character.

A demo with network enabled plugin in action can be found below link.
[Light Awareness Demo UE5 Project](https://github.com/cem-akkaya/LightAwarenessDemo)

If you have any bug or crash, please open an issue in the Github repo.\
If you have suggestions, questions or need help to use you can always contact [me](https://github.com/cem-akkaya)<br>

If you want to contribute, feel free to create a pull request.

## Features

- Detection of light status on any actor.
- Light detection sensitivity and optimization methods.
- Light detection direction choice for incoming sources.
- Lumen and global illumination support.
- Debug views and light thresholds for your game's global lightening conditions.
- Ability to integrate plugin into any actor on runtime.
- Ability to get light and buffer pixels from blueprints.
- Subsystem for accessing light status data and status update events.
- Integrated update methods for usage if suitable for your project as distance update.

## Examples

Some passages defined with various light sources and and dark zones:\
<img src="https://cemakkaya.com/ImageHost/example1.gif" alt="Animated GIF" width="200"/>
<img src="https://cemakkaya.com/ImageHost/example2.gif" alt="Animated GIF" width="200"/>
<img src="https://cemakkaya.com/ImageHost/example3.gif" alt="Animated GIF" width="200"/>
<img src="https://cemakkaya.com/ImageHost/example4.gif" alt="Animated GIF" width="200"/>


Some examples of light awareness plugin in action:
- A room with soft spotlight sources showing accuracy.
- Directional light and soft point light example.
- Direct light beams showing responsiveness.
- Finally a big soft light showing increments of light and global illumination.

## Installation
<img src="https://cemakkaya.com/ImageHost/plugin.jpg" alt="plugin-light-awareness" width="830"/>

Install it like any other Unreal Engine plugin.
- Download and place LightAwareness Plugin under  : Drive:\YOURPROJECTFOLDER\Plugins\LightAwareness
- Activate the plugin in your project plugins.
- On your actor add component "LightAwareness".
- Drag a pin from component to see available functions.

### Installation 5.3 & Down
- If you are using older than 5.4 and get material loading errors :
- - Download [this content](https://www.cemakkaya.com/FileHost/LightAwarenessContentFallback.zip) and overwrite to Drive:\YOURPROJECTFOLDER\Plugins\LightAwareness\Content
- - Rebuild your project, disable plugin if necessary

<img src="https://cemakkaya.com/ImageHost/node.jpg" alt="plugin-light-awareness" width="830"/>

## How to use it
<img src="https://cemakkaya.com/ImageHost/inspector.jpg" alt="inspector-light-awareness" width="830"/>

- On your actor add component "LightAwareness".
- Define the light gem using components scaling and offsets to your needs.
- Define sensitivity and method as you need.
- Call "Get Light Status" function to snapshot light level on object for manual request of status.
- Call Subsystem update event for accessing light changes in large scale environments and utilization.
- Engine Versions Under 5.2 Should use "Engine Version Fallback = true"

### Light Awareness Subsystem
<img src="https://cemakkaya.com/ImageHost/subsystem.jpg" alt="inspector-light-awareness" width="830"/>

- You can access LightAwareness GameInstance subsystem from blueprints
- Subsystem provides events when an actor with component registers, unregisters or updates its status.
- You can use this information to simply know how many light awareness responders are available at a given time in gameplay with its status as : Active or ActiveRendering
- Without the events you can simply use GetCurrentLightAwarenessResponders blueprint node to access array of components.
- You can select and filter GetCurrentLightAwarenessResponders node with currently active responders and active being rendered state ones.
- When a component is destroyed (kill or streamed out), component is unregistered from subsystem.
- Visibility status depends on the actor having at least one mesh component. There is no tracing methods used on the component since its costly, you can use additional tracing on event to determine object position or further gameplay/functionality logic.


A demo with network enabled plugin in action can be found below link.
[Light Awareness Demo UE5 Project](https://github.com/cem-akkaya/LightAwarenessDemo)

## FAQ
<details>
<summary><b>From Which gameplay objects can I get light status? </b></summary>

> From any object that is existing in the level, you can get a light status value.
> If you didn't attach the component, you can simply spawn an actor at location and attach the component in runtime and destroy.\
</details>

<details>
<summary><b>How reliable is the detection method? </b></summary>

> Depends on what settings you use to get buffer pixels and calculate them.
> Default settings are quite reliable on many levels and can be used for stealth games as well as other utilization purposes.\
> With default settings 32 Pixels are brute force searched and brightest pixel returned on tick.
> This is more than enough for getting an average light, however its recommended to do top direction pass if its not needed in your game.\

</details>

<details>
<summary><b>Can I get light from Emissive and Other sources?</b></summary>

> Yes you can. There is a "Global Illumination" option where environmental light effects are also included in light detection.\
> However Lumen in scene renders for small image sizes can have small value jumps in extreme light condition changes.

</details>



## License

This plugin is under [MIT license](LICENSE).

MIT License does allow commercial use. You can use, modify, and distribute the software in a commercial product without any restrictions.

However, you must include the original copyright notice and disclaimers.

## *Support Me*

If you like my plugin, please do support me with a coffee.

<a href="https://www.buymeacoffee.com/akkayaceq" target="_blank"><img src="https://cdn.buymeacoffee.com/buttons/default-yellow.png" alt="Buy Me A Coffee" height="41" width="174"></a>