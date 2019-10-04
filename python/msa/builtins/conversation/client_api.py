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
            json = response.json()

            if json["type"] == "response":
                print(json["text"])
            elif json["type"] == "error":
                print("Server Error:", json["message"])
        except:
            print("It seems there was an issue.")

