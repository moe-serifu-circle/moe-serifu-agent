# Moe Serifu Agent
## Overview
[![Build Status](https://megumin.moeserifu.moe/api/badges/moe-serifu-circle/moe-serifu-agent/status.svg?branch=master)](https://megumin.moeserifu.moe/api/badges/moe-serifu-circle/moe-serifu-agent)
![logo](https://raw.githubusercontent.com/wiki/dekarrin/moe-serifu-agent/assets/logo/logo-en-700w.png "MSA Logo")

[Project Website](http://www.moeserifu.moe)

Moe Serifu Agent (MSA) is an event-driven personal assistant system that
presents itself as existing in a particular location (like a house or a
smartphone) and performs various tasks as directed by the user.

At a high-level, this system provides an anime-themed character that exists in
cyberspace. It runs around the location it's installed in and appears at the
end-users' beck and call in order to perform whatever services are needed,
including timed reminders, checking and reporting on the state of its location,
conversation, and performing in an entertainment role.

As an example, a user might tell the MSA to greet them when they return from
work, or to wake them up in a customized way in the mornings. With its plugin
API, new sensors and interfaces can be added to allow the MSA to interact with
the world in just about any way the user desires.

*******

## Anime AI

The MSA project is inspired by various fictional artificial entities, such as
the Virtual Intelligences from the Mass Effect Series, the Persocoms from the
Chobits series, the Tachikoma from the Ghost in the Shell series, and the
AnthroPCs from the Questionable Content webcomic. The primary goal of the
project is to create a system that carries out commands for the user and that
gives the appearance of being an independent intelligent entity.

The anime theme was chosen because the author believes that the demograph that
consumes anime tends to have a lower barrier to their willing suspension of
disbelief in ascribing emotions to fictional characters than that of the
general population.

![mascot-vsign](https://raw.githubusercontent.com/wiki/dekarrin/moe-serifu-agent/assets/mascot/vsign-150w.png "Masa-Chan")

## Exchangable Personality and Appearance

The MSA system at its core represents itself as an anime-themed character. An
intelligent agent system is used to determine how to accomplish goals set by the
user, as well as to control the character's state, including the appearance of
emotions and how to react to events. The AI is driven partially by a personality
module, which can be exchanged in order to make the character act differently.
Different personality modules are created with different behaviors in mind; each
would fall under a different anime character archetype, such as tsundere,
kuudere, yandere, deredere, etc.

An avatar of the character is presented to the end-user for interfacing with the
system. This initial project narrows the goal of the avatar system to exist
purely in cyberspace; there is no physical device (such as a robotic assembly)
that the MSA can manipulate, although this functionality could certainly be
added using the plugin system.

This MSA avatar can be interacted with using a variety of methods including
voice recognition and via command-line interface, and it is shown to the user as
a 3D model or 2D character on whichever devices are included in an instance of
the system.

The specific details regarding what the avatar looks like visually, how it
sounds, and how it demonstrates emotions are controlled by an avatar module
within the MSA. This module can be exchanged with other such modules in order to
change the appearance of the avatar.

A personality module and avatar module are intended to be combined into a set
and distributed as a complete 'character pack', though there is nothing in the
system design that would prevent the personality module of one pack from being
used with the avatar module of another.

## Physical Representation

In a complete MSA installation, a device (such as a screen/monitor) is set up in
each of the rooms that it is to be interacted with. The MSA maintains a 'room'
that the character resides in, and the character 'travels' between rooms by its
avatar exiting a device and entering another one in an adjacent physical room.
In general, the avatar will only travel between adjacent devices, e.g. if the
system is set up such that device A is next to device B which is next to device
C, then in order to travel from device A to device C, the avatar will move from
A to B, then B to C.

Additionally, the user may download an app that allows their mobile device to be
used as an output device. In this case, the avatar could travel directly to the
user in order to interact with them. The MSA system would use a variety of
sensors in order to detect the physical location of the mobile device and track
which other output devices it should be considered adjacent to.

![mascot-chibi](https://raw.githubusercontent.com/wiki/dekarrin/moe-serifu-agent/assets/mascot/chibi-100w.png "Masa-Chan Chibi")
