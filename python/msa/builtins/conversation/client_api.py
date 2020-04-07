#!/usr/bin/env python
# -*- coding: utf-8 -*-



def register_endpoints(api_binder):

    @api_binder.register_method()
    async def talk(self, input):
        response = await self.client.post(
            "/conversation/talk",
            payload={"input": input})

        if response.status_code != 200:
            raise Exception(response.raw)

        try:
            print(response.text)
        except:
            print("It seems there was an issue.")

