# Contributing

This document lays out some of the ways that you can help out with Moe Serifu Agent and
other projects under Moe Serifu Circle.

## Team Structure
Efforts towards MSC projects are lead by a team of volunteers. These volunteers maintain
the project, and are responsible for deciding whether to accepting PRs or request changes
on them.

These project maintainers have the final say on whether contributions and PRs meet the
guidelines outlined in this document; at MSC, we believe that communication is key to a
well-functioning team and quality software, so we very much encourage contributors to feel
free to reach out to our maintainers to get help on any issues that are unclear.

## Finding What to Work On

### Discord Server

The number one best way to coordinate helping out with Moe Serifu Circle's projects is to
hop on to [the circle's discord server](https://discord.gg/URAA8SF) and ask what needs
doing. It will be the most up to date source of active development efforts.

Folks on the discord with an `engineers` or `contributors` role will likely be able to
quickly help point you in the right direction.

Join the server, let us know that you want to help, and we'll get you something to work
on.

### GitHub Issues

All of the items in our backlog are located in the [GitHub Issues](https://github.com/moe-serifu-circle/moe-serifu-agent/issues)
page for the project; from there, you should be able to get a pretty good idea of what
needs to be worked on.

Of course, the recommendation will always be to coordinate activities with project
maintainers when possible, but if a PR is specifically connected to an extant issue,
it is very likely that it will be accepted.

Do note that we use the assignment field of Issues to show who is working on something,
but the status of that field may not always be up to date. The most recent information
on what tasks are already in-flight can always be found by asking the active engineers
at [the circle's discord server](https://discord.gg/URAA8SF).

## Contribution Guidelines

### All contributors MUST sign a License Agreement

In order to ensure that MSC's projects are available for all to use and can be available
for everyone's benefit, we require that all contributors sign a Contributor License
Agreement before any contributions are accepted.

This agreement essentially states that while you will keep ownership of the code and
contributions that you produce and submit to our projects, you grant permission to Moe
Serifu Circle to do whatever we see fit with your contribution, including relicensing it
for other people to use.

You only need to submit the agreement once; this will let you work on all MSC projects.

You can find instructions for filling out the form as well as the form itself
[here](https://goo.gl/forms/hu1hTNZs1xj2mFnF2). If you are filling it out manually, make
sure to send it to the email address listed there or to send it directly to a member of MSC's
board of directors. If you use the online form, note that it will not notify us when you
submit it; you will need to tell us that you submitted it.

### Contributions Should Be Associated with Issues

Every PR should be associated with an existing project backlog item or GitHub Issue. In
some rare instances, this may not be required. In that case, ensure that the PR submission
includes a detailed description of the change and the motive for the change.

It would also be a good idea to notify a project maintainer beforehand if you plan on
submitting a PR without an associated Issue.

PRs that are not associated with an issue and that do not have reasonable justification given
in their description are likely to be rejected or sent back for changes.

### Unit Tests and Comments

All PRs that contain changes to code should be accompanied by unit tests that cover the
change. If unit testing does not apply to the situation given in the PR, reasoning should
be given in the PR description; notifying a project maintainer of the lack of testability
prior to submitting the PR is also advisable.

All PRs must include properly commented code. The exact definition of what constitutes
'properly commented' is not well-defined, but at a minimum, there should be a module-level
or class-level docstring explaining what the module and/or class does.

PRs that do not follow the above guidelines are likely to be rejected or sent back for
changes.

### Style

Please be sure to follow coding style practices established in a project. PRs are likely
to be rejected or sent back for changes if the style is drastically different from the
one established in the project.

If there is a significant reason for the style differing from convention, make sure to
include the reason either in a code comment or in the PR submission.

For this project, the following guidelines are in effect:

* Use `snake_case` for the names of functions, local variables, and modules.
* Use `CamelCase` for the names of classes.
* In general, PEP-8 should be followed, except for where it negatively impacts readibility
and maintainibility in the code. The cases where PEP-8 is explicitly permitted to be ignored,
in general, are as follows:
  * Global exception Handlers that catch all exceptions are permitted (at the top-level
  of execution only). Ensure that they properly send the exceptions somewhere (e.g.
  logging), and that they don't just swallow them.
  * Shadowing global imports with local identifiers (e.g. it's okay to have a parameter
  called `id`).
  * A single-line comment that comes after a line of code is permitted to do so with any
  number of spaces. For example, rather than `x += 4  # adds four` (the standard required
  by PEP-8), it is permissible to do `x += 4 # adds four`, or any other number of spaces
  within reason.


### Architecture Changes

The architecture of the systems in this project should be considered fixed. MSC is not
opposed to accepting changes to its projects' architectures, but please be aware that
this will usually not be a trivial change.

To ensure that all engineers are on the same page, major architectural changes should
be brought up directly with the engineering team. The easiest way to do that is on
[the circle's discord server](https://discord.gg/URAA8SF) Discord server.

PRs that necessitate large changes to architecture that are submitted without prior
discussion will be rejected; the maintaining team may request further action be taken to split the task into smaller sub-tasks.

