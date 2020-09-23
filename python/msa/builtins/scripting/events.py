from croniter import croniter
from schema import Schema, And, Or, Optional

from msa.core.event import Event


class ScriptDeletedEvent(Event):
    """
    ScriptDeletedEvent schema:

    name:
        The name of the script that was deleted

    """

    def __init__(self):
        super().__init__(
            priority=50,
            schema=Schema(
                {
                    "name": And(str, len),
                    "status": Or("success", "failure"),
                    Optional("reason"): And(str, len),
                }
            ),
        )


class TriggerDeleteScriptEvent(Event):
    """
    TriggerDeleteScriptEvent schema:

    name:
        The name of the script to deleted
    """

    def __init__(self):
        super().__init__(priority=50, schema=Schema({"name": And(str, len)}))


class TriggerGetScriptEvent(Event):
    """
    TriggerListScriptEvent schema:

    name:
        The name of the script to fetch
    """

    def __init__(self):
        super().__init__(priority=50, schema=Schema({"name": And(str, len)}))


class GetScriptEvent(Event):
    """
    TriggerListScriptEvent schema:

    id:
        The id of the script
    name:
        The name of the script
    crontab:
        The crontab of the script
    created:
        The created timestamp of the script
    last_edited:
        The last edited timestamp of the script
    last_run:
        The last run timestamp of the script
    scheduled_for:
        The next timestamp the script is scheduled to execute at.
    script_contents:
        Content of the script
    running:
        A boolean indicating whether or not the script is currently running.
    """

    def __init__(self):
        super().__init__(
            priority=50,
            schema=Schema(
                {
                    "id": int,
                    "name": And(str, len),
                    "crontab": Or(And(str, len), None),
                    "created": And(str, len),
                    "last_edited": And(str, len),
                    "last_run": Or(And(str, len), None),
                    "scheduled_for": Or(And(str, len), None),
                    "script_contents": And(str, len),
                    "running": bool,
                }
            ),
        )


class TriggerListScriptsEvent(Event):
    """
    TriggerListScriptEvent schema:

    *No schema is required.*
    """

    def __init__(self):
        super().__init__(priority=50, schema=Schema(None))


class ListScriptsEvent(Event):
    """
    ListScriptEvent schema:

    id:
        The id of the script
    name:
        The name of the script
    crontab:
        The crontab of the script
    created:
        The created timestamp of the script
    last_edited:
        The last edited timestamp of the script
    last_run:
        The last run timestamp of the script
    scheduled_for:
        The next timestamp the script is scheduled to execute at.
    running:
        A boolean indicating whether or not the script is currently running.
    """

    def __init__(self):
        super().__init__(
            priority=50,
            schema=Schema(
                {
                    "scripts": [
                        {
                            "id": int,
                            "name": And(str, len),
                            "crontab": Or(And(str, len), None),
                            "created": And(str, len),
                            "last_edited": And(str, len),
                            "last_run": Or(And(str, len), None),
                            "scheduled_for": Or(And(str, len), None),
                            "running": bool,
                        }
                    ]
                }
            ),
        )


class AddScriptEvent(Event):
    """
    AddScriptEvent schema:

    name:
        The name of the script, must not include any whitespace.
    crontab:
        The crontab interval this script should run on. Refer to https://en.wikipedia.org/wiki/Cron for
        details on syntax.
    scripts_contents:
        The contents of the script to be submitted.
    """

    def __init__(self):
        super().__init__(
            priority=100,
            schema=Schema(
                {
                    "name": And(str, len, lambda s: (sum(c.isspace() for c in s) == 0)),
                    Optional("crontab"): And(str, len, croniter.is_valid),
                    "script_contents": And(str, len),
                }
            ),
        )


class AddScriptFailedEvent(Event):
    """
    AddScriptFailedEvent schema:

    error:
        A friendly title for the  error, should be humanly readable
    description_verbose:
        A verbose explanation of what went wrong. May include stack traces, and is
        intended to be displayed as raw text.
    description:
        A humanly readable explanation for the error. May not include stack traces,
        and should be simple enough to use with a Text-to-Speech service.
    """

    def __init__(self):
        super().__init__(
            priority=20,
            schema=Schema(
                {
                    "error": And(str, len),
                    "description": And(str, len),
                    "description_verbose": And(str, len),
                }
            ),
        )


class TriggerScriptRunEvent(Event):
    """
    TriggerScriptRunEvent schema:

    name:
        The name of the script that should be run
    """

    def __init__(self):
        super().__init__(priority=50, schema=Schema({"name": And(str, len)}))


class RunScriptResultEvent(Event):
    """
    RunScriptResultEvent schema:

    name:
        The name of the script that was run
    error:
        A string describing any errors, can be None or a non-empty string.
    log:
        A string containing any log messages produced by the script.
    """

    def __init__(self):
        super().__init__(
            priority=50,
            schema=Schema(
                {
                    "name": And(str, len),
                    "error": Or(None, And(str, len)),
                    "log": And(str),
                }
            ),
        )
