# Contributor Guide

## Running from source

### Installation
1. If you do not have it already, please install the python package manager `poetry`: See [poetry installation instructions](https://python-poetry.org/docs/#installation).
2. Clone the repository `git clone https://github.com/moe-serifu-circle/moe-serifu-agent.git`
3. Open a terminal and navigate to the location you cloned the repository to.
4. Run `poetry install` and `poetry shell` to install the python requirements and enter a virtual environment.

### Starting the daemon
1. Open a terminal and navigate to the location you cloned the repository to.
2. Run `python -m msa daemon` to start the daemon. The daemon should now be hosted on [http://localhost:8080](http://localhost:8080).

### Starting the Developer CLI 

Note: In order to use the CLI you must start the daemon first in another terminal.

1. Open a terminal and navigate to the location you cloned the repository to.
2. Run `python -m msa cli` to start the cli which will connect to the daemon. You should be greeted with a python interpreter looking interface, this is the developer cli.


## Plugin Development

**[STUB]**



