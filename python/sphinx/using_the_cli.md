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
The `msa_api` object can be used to interface with the daemon process. 

### Basic usage
Below you can find a list of methods available on `msa_api`. 

The methods marked `async` below need to be prefixed with `await`. For example:
```python
>>> await msa_api.talk("hello")
```

### Recording and uploading scripts

One of the meta-commands available in the cli is the `# record` command.

As in the following example, it is possible to record an commands and save them as a script which can then be uploaded
to the daemon.

```python
$ moe-serifu-agent
>>> # record test.py                                                                                                                                                                                                                  
>>> await msa_api.talk("Hello, how are you?")                                                                                                                                                                                         
I am well thank you. What can I do for you?
>>> await msa_api.talk("Please turn on the livingroom light.")                                                                                                                                                                        
I am afraid I don't know what to say.
>>> # record stop                                                                                                                                                                                                                     
Opening test.py
```

Now if we `ls` the current directory, we should see a new file `test.py` has been created, containing the following:
```python
await msa_api.talk("Hello, how are you?")
await msa_api.talk("Please turn on the livingroom light.")
```

We can also upload the script to the daemon and schedule it to run every so often.
```python
>>> await msa_api.upload_script("demo_script", crontab="* * * * *", file_name="test.py")
demo_script was sucessfully uploaded
```

The test script we uploaded should run once every minute. Unfortunately this is not very useful as we cannot see the 
conversation response from MSA because the daemon doesn't know where to send the response. Future tutorials will walk
through some more interesting uses for uploaded scripts such as triggering events.

One final note on uploading scripts is that they are saved to MSA's database and are reloaded at startup.


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

