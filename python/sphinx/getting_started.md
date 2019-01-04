# Getting Started

The goal of this getting started guide is to walk you through the basic usage of MSA. 

**Note:** This guide assumes that you have already installed the `moe-serifu-agent` package, if not, please see the 
[Installation](installation) page.

**Note**: This guide will explain how to run MSA from the `moe-serifu-agent` package. To run from source see 
[Running From Source](contributor_guide#running-from-source)

## Up and running

To start MSA run, `moe-serifu-agent` in a terminal. You will be presented with a prompt. Interacting with the prompt is 
the most basic way to interact with MSA. 

To find available commands type `help` e.g.

```bash
>> help
Available Commands:
echo: Echos provided text back through the terminal
quit: Shuts down the current Moe Serifu Agent instance
help: Prints available commands and information about command usage.
```

To read the help text for a specific command, type `help [name of a command]` e.g.
```bash
>> help echo
Help text for command 'echo':
Usage: 'echo [text]'
Options: No available options.
Description: Echos provided text back through the terminal
```


To exit at any time, press `Ctrl+c` or type `quit`.


Now that we have covered how to get up and running with MSA, here are a few more topics worth reading:
- [Customizing your MSA](configuration): Covers setting up a configuration file that you can use to customize the 
behavior of MSA.
- [Built-in Commands](builtin_commands): Describes each of the builtin commands and what you can do with them.
- [Extending MSA with plugins](plugins): Adding additional functionality through MSA plugins.

