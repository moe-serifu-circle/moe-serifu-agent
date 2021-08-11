from enum import Enum


class ServerResponseType(Enum):
    success = "success"
    failure = "failure"


class ServerResponse:
    def __init__(self, response_status: ServerResponseType):
        self.data = {"status": response_status.value}

    def get_data(self):
        return self.data


class ServerResponseText(ServerResponse):
    def __init__(self, response_status: ServerResponseType, text):
        super().__init__(response_status)
        self.data["text"] = text


class ServerResponseJson(ServerResponse):
    def __init__(self, response_status: ServerResponseType, payload):
        super().__init__(response_status)
        self.data["payload"] = payload
