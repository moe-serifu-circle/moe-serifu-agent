#!/usr/bin/env python
# -*- coding: utf-8 -*-

async def talk(self, input):
    """
    Interact with the conversation module builtin to MSA. Expects a natural language message, MSA should respond in kind.

    :async:
    :param input: A natural language message to send to MSA.
    :return: `None`
    """
    response = await self.client.post(
        "/conversation/talk",
        payload={"input": input})

    if response.status_code != 200:
        raise Exception(response.raw)

    try:
        print(response.text)
    except:
        print("It seems there was an issue.")


def register_endpoints(api_binder):
    api_binder.register_method()(talk)


