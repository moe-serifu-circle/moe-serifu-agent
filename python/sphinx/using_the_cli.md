# Using the CLI

## `cli` basics
When you start the cli, you will be greated with something like the following:
```bash
>>>
```

At it's core the cli is stripped down python interactive interpreter. Similar to if you just ran `python` on your terminal. 
There are some notable differences:
- The presence of a `msa_api` object which is used for interacting with the daemon.
- Some "meta-commands" for the cli that allow special functionality within the cli. This includes recording scripts and saving them.

## Intro to meta-commands

Meta-commands are executed within the cli and do not directly touch the daemon.

Available meta commands can be listed by typing `# help` in the cli e.g.:
```bash
>>> # help
MSA Interpreter Help:
 Availiable Commands:
  # help: Show this help text
  # record <file name>: Begin recording commands to a script.
  # record stop: stop recording commands, save the script, and open to review.
```

**Note:** the result of this `help` meta-command is subject to change as meta-commands are added or removed.

See below for an example using meta-commands to record a script interactively and sending it to the daemon to run periodically.

## Intro to the `msa_api` object
When you start the cli a `msa_api` object is inserted into scope to facilitate interaction with the daemon process.



## `msa_api` Reference

```eval_rst
.. py:class:: msa_api

  .. automodule:: msa.api.base_methods
      :members:
      :show-inheritance:
  
  .. automodule:: msa.builtins.conversation.client_api
      :members:
      :show-inheritance:
  
  .. automodule:: msa.builtins.scripting.client_api
      :members:
      :show-inheritance:
  
  .. automodule:: msa.builtins.signals.client_api
      :members:
```

