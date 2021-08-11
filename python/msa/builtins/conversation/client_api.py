#!/usr/bin/env python
# -*- coding: utf-8 -*-


async def talk(self, input):
    """
    Interact with the conversation module builtin to MSA. Expects a natural language message, MSA should respond in kind.

    :async:
    :param input: A natural language message to send to MSA.
    :return: `None`
    """
    response = await self.client.post("/conversation/talk", payload={"input": input})

    if response.status == "failed":
        raise Exception("Server Error: \n" + response.text)

    response = response.text

    if response is None:
        print("It seems there was an issue.")
    else:
        print(response)


def register_endpoints(api_binder):
    api_binder.register_method()(talk)
