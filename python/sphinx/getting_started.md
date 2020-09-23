# Getting Started

The goal of this getting started guide is to walk you through the basic usage of MSA. 

**Note:** This guide assumes that you have already installed the `moe-serifu-agent` package, if not, please see the 
[Installation](installation.html) page.

**Note**: This guide will explain how to run MSA from the `moe-serifu-agent` package. To run from source see 
[Running From Source](contributor_guide.html#running-from-source)

## Up and running

### Starting the daemon

Before we can start interacting with MSA we need to start it. To start MSA, run `moe-serifu-agent daemon` in a terminal. By default the daemon will run on port `8080`. 

### Connecting to the daemon with a client

#### The development Command Line Interface (also called the cli)

To start the cli, open a new terminal on the same computer running the daemon. Then run `moe-serifu-agent cli`. 
This should start the interactive cli. This tool is an interactive python interpreter for writing and uploading scripts. 
It is the lowest level way of interacting with MSA.

To exit at any time, press `Ctrl+c` or type `quit()`.


## Next Steps

Now that we have covered how to get up and running with MSA, here are a few more topics worth reading:
- [Customizing your MSA](configuration): Covers setting up a configuration file that you can use to customize the 
behavior of MSA.
- [Using the CLI](using_the_cli): A tutorial on using the cli to write scripts for MSA to run, and to build custom automations.
- [Extending MSA with plugins](plugins): Adding additional functionality through MSA plugins.

