#!/usr/bin/env python
# -*- coding: utf-8 -*-
from msa.core.event_handler import EventHandler
from msa.core import get_supervisor
from msa.builtins.conversation.events import ConversationOutputEvent, IntentEvent
from msa.builtins.signals import events as signal_events
from aiocron import crontab
import asyncio

try:
    import feedparser
except:
    print("rss_feed plugin, requires the feedparser library to be installed.")
    exit()


class RssPollingHandler(EventHandler):
    """
    Handles ConversationInputEvents
    """

    def __init__(self, loop, event_bus, logger, config=None):
        super().__init__(loop, event_bus, logger, config)

        self.event_bus.subscribe(IntentEvent, self.handle_request_feed)

        self.feed = None

    def schedule(self):
        return [(self.config.get("poll_crontab", "0 * * * *"), self.check_feed)]

    async def check_feed(self, time):
        self.feed = await self.loop.run_in_executor(None, self.fetch_feeds)

    def fetch_feeds(self):
        return feedparser.parse(self.config["feed_url"])

    async def handle_request_feed(self, event):

        if event.data["type"] != "rss_feed_request":
            return

        if self.feed is None:
            await self.check_feed(None)

        title = self.feed["feed"]["title"]

        output = f"{title}:\n"
        for entry in self.feed["entries"][:10]:
            title = entry["title"]
            link = entry["link"]

            output += f"• {title}: {link}\n"

        new_event = (
            ConversationOutputEvent().init({"output": output}).network_propagate()
        )

        get_supervisor().fire_event(new_event)
