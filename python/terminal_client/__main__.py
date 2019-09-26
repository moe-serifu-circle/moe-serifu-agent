#!/usr/bin/env python
# -*- coding: utf-8 -*-

import asyncio
import aiohttp

import msa
from msa.api import MsaApiWrapper 

# a very simple conversational client
msa_api = MsaApiWrapper().get_api()

msa_api.check_connection()

expected_version = "0.2.0"
server_version = msa_api.get_version()
if expected_version != server_version:
    print("Server version does not match required version!")
    print("Expected version:", expected_version)
    print("Server version:  ", server_version)
    exit(1)


# start websocket connection

async def listen_for_messages(ws):


async def connect():
    async with aiohttp.ClientSession() as session:
        async with session.ws_connect('http://localhost:8080') as ws:
            await listen_for_messages(ws)

loop = asyncio.get_loop()
loop.run_until_complete(connect())
loop.run_until_complete(asyncio.sleep(0))
loop.close()


    


