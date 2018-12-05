# Architectural Overview

> Note: This section is very techy. If you are not interested or knowledgeable in programming or how the internals of 
> the MSA work, this section is likely not for you.

## Built-in Modules

### Command Registry

The Command Registry is the heart and soul of the command system. When a user enters text, and the TTY module propagates
a `TextInputEvent`, the Command Registry attempts to parse the input into an invoke keyword and a list of parameters.
If the first token in the input matches the invoke keyword of a registered command type, the Command Registry will 
propagate a new event for the registered command type to handle. 

The Command Registry also handles listening and displaying text for help queries.

### Command

### Echo

### Time

The time module propogates a `TimeEvent` at the beginning of every minute.

### TTY

The TTY module enable input and output from the terminal. The TTY modules input handler listsens to the TTY for terminal
input and generates a `TextInputEvent` for other modules to handle.



