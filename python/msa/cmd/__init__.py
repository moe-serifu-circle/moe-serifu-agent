

class ParamList:
    def __init__(self, tokens=None, options=None):
        self.tokens = tokens
        self.options = None

    def has_options(self):
        return self.options is not None

    def __repr__(self):
        return f"<ParamList at {hex(id(self))}>"

class Result:
    def __init__(self, status_code=None, return_value=None):
        self.status_code = status_code
        self.return_value = return_value

    def value(self):
        return self.status_code

    def status(self):
        return self.return_value

    def __repr__(self):
        return f"<Result at {hex(id(self))}>"

class StdResultCode:
    OK = 0
    ERROR = 1

    MSA_ERROR_LOG = 2

class Command:

    def __init__(self, invoke, describe, usage, options=None):
        self.invoke = invoke
        self.describe = describe
        self.usage = usage
        self.options = options


    def handler(self, **kwargs):
        raise NotImplementedError


    def __repr__(self):
        return f"<Command: {self.invoke} at {hex(id(self))}>"



class CommandManager:
    def __init__(self):



