# Agent state initial design

*******

## Agent State

The state of the MSA can be one of several options, which determine how it acts.

### Idle State

This is the default state of the MSA and is entered into when it does not have
any active goals to pursue. It will spend some time waiting, but will then
choose one of some random frivolous activity to do. The MSA will move about the
active I/O device network as specified by the activity.

### Alerting State

The MSA will enter this state when it has discovered information that must be
shared with the user. The MSA will drop everything of a lower priority and rush
to alert the user. If a phone is present and is on the user's person, the MSA
will move to the phone to directly deliver the message.

If multiple alerts are to be delivered, they will form a priority queue which
will all be delivered at once.

The MSA will only exit the alert state once it has successfully delivered the
message and had it acknowledged by the user.

### Attentive State

The MSA will enter this state when it receives a summons from the user. It will
travel to the output device nearest the user and wait expectantly for a command
delivered to it via the I/O network.

### Debug State

In this state, the MSA provides a direct command and query interface via
command-line interface. A request to transition into this state overrides all
other commands and immediately presents the Command Line Interface. Prior to
entering debug state, the MSA will save its current state and resume it once
the debug state is exited.

### Ero State

This state is adult activity mode. The MSA will undergo a variety of
interactions and self-directed actions designed to appeal to the user without
growing too repetitive.

Age-restriction can be enabled such that this state cannot be transitioned into
without prior authorization.

This state must be enabled manually while the MSA is in attentive state; the
only way a transition to Ero state can be initiated is by explicit command from
the user or explicit scheduling by the user.

### Conversing State

In this state, the MSA will engage in conversation with the user. Each
conversation will have a goal, such as entertainment, cheering up the user, etc.
During the conversation, the sentiment of the user's responses will be analyzed
to determine whether the previous MSA output was good or bad, and common
patterns are stored for future use.

During this state, the MSA can attempt to learn facts about the user, such as
their hobbies, family, studies, work, daily life, etc. This information can then
be used during future conversations.
