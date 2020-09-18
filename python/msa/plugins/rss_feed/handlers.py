#!/usr/bin/env python
# -*- coding: utf-8 -*-
from msa.core.event_handler import EventHandler
from msa.core import get_supervisor
from msa.builtins.conversation.events import ConversationOutputEvent
from msa.builtins.signals import events as signal_events
from aiocron import crontab

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

    async def init(self):
        print("rss plugin loaded")
        while True:
            await crontab(self.config["poll_crontab"]).next()
            feed = await self.loop.run_in_executor(None, self.fetch_feeds)

            title = feed["feed"]["title"]

            output = f"Latest in Anime News by Crunchyroll: {title}"

            new_event = ConversationOutputEvent().init({
                "output": output
            })

    def fetch_feeds(self):
        return feedparser.parse(self.config["feed_url"])
