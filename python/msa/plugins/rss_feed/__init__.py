from schema import Schema, And
from msa.plugins.rss_feed import handlers

handler_factories = [handlers.RssPollingHandler]

entities_list = []


config_schema = Schema({"feed_url": And(str, len), "poll_crontab": "* * * * *"})
