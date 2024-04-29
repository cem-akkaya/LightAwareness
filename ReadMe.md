# Light Awareness Plugin

[![Plugin version number](https://img.shields.io/github/v/release/cem-akkaya/LightAwareness?label=Version)](https://github.com/cem-akkaya/LightAwareness/releases/latest)
[![Unreal Engine Supported Versions](https://img.shields.io/badge/Unreal_Engine-4.27_%7C_5.0_%7C_5.1_%7C_5.2_%7C_5.3-9455CE?logo=unrealengine)](https://github.com/cem-akkaya/LightAwareness/releases)
[![License](https://img.shields.io/github/license/cem-akkaya/LightAwareness)](LICENSE)
![Download count](https://img.shields.io/github/downloads/cem-akkaya/LightAwareness/total?label=Downloads)
[![Actively Maintained](https://img.shields.io/badge/Maintenance%20Level-Actively%20Maintained-green.svg)](https://gist.github.com/cheerfulstoic/d107229326a01ff0f333a1d3476e068d)

<img src="https://cemakkaya.com/ImageHost/splash.jpg" alt="plugin-light-awareness" width="100%"/>


## Overview

This is an Unreal Engine plugin to detect light conditions on any actor independent from Light Source Type, Screen Effects and similar.

This plugin is an actor component where snapshots a buffer image from the world with given variables. Process the buffer
image pixels from the memory and brute forces the brightest pixel in the light gem. This plugin is created on ingenious method of [Dark Mod](https://www.thedarkmod.com/main/) indie game,
re-creating approach in a more accurate calculation in Unreal 5+. 

You can define the processing method and efficiency on the component and on the blueprint level depending on the usage.
You define your own rules in blueprints or C++ to get light status from the component or you can get the buffer image array on the tick to process further calculations
in blueprint or C++.

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

## Examples

Some hand-made rooms defined with bounds and doors:\
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
- Download and place LightAwareness Plugin under  : Engine\Plugins
- Activate the plugin in your project plugins.
- On your actor add component "LightAwareness".
- Drag a pin from component to see available functions.

<img src="https://cemakkaya.com/ImageHost/node.jpg" alt="plugin-light-awareness" width="830"/>

## How to use it
<img src="https://cemakkaya.com/ImageHost/inspector.jpg" alt="inspector-light-awareness" width="830"/>

- On your actor add component "LightAwareness".
- Define the light gem using components scaling and offsets to your needs.
- Define sensitivity and method as you need.
- Call "Get Light Status" function to snapshot light level on object.


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

If you like my plugin, please do support me buy grabbing a coffee.

<a href="https://www.buymeacoffee.com/akkayaceq" target="_blank"><img src="https://cdn.buymeacoffee.com/buttons/default-yellow.png" alt="Buy Me A Coffee" height="41" width="174"></a>