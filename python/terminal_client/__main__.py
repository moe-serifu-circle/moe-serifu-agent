#!/usr/bin/env python
# -*- coding: utf-8 -*-

import asyncio
import aiohttp
from prompt_toolkit.shortcuts.prompt import PromptSession
from prompt_toolkit.eventloop.defaults import use_asyncio_event_loop

import msa
from msa.api import get_api, run_async
from msa.api.context import ApiContext

use_asyncio_event_loop()

loop = asyncio.get_event_loop()
host = "localhost"
port = 8080
plugins = []

# a very simple conversational client
api = get_api(ApiContext.rest, plugins, host=host, port=port)


async def startup_check():

    await api.client.connect()

    try:
        await api.check_connection()

        expected_version = "0.1.0"
        server_version = await api.get_version()
        if expected_version != server_version:
            print("Server version does not match required version!")
            print("Expected version:", expected_version)
            print("Server version:  ", server_version)
            exit(1)
    finally:
        await api.client.disconnect()


run_async(startup_check())

# start websocket connection

async def interact(api, state):
    try:
        while True:
            if not "prompt_session" in state:
                prompt_session = PromptSession()
                state["prompt_session"] = prompt_session
            else:
                prompt_session = state["prompt_session"]


            try:
                text = await prompt_session.prompt(">>> ", async_=True)

                print(await api.get_version())
            except (KeyboardInterrupt, EOFError):
                print("Shutting down...")
                await api.client.stop()
                return 
    finally:
        await ws_client.client.stop()




ws_client = get_api(ApiContext.websocket, plugins, interact=interact, loop=loop, host=host, port=port)
ws_client.client.start()



    


