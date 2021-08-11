from schema import Schema, Optional, And, Or

from msa.plugins.notifications import handlers

handler_factories = [handlers.SendNotificationEventHandler]

entities_list = []

config_schema = Schema(
    {
        Optional("preferred_provider"): Or("pushbullet", "email", "slack"),
        "providers": {
            Optional("pushbullet"): {"token": And(str, len)},
            Optional("email"): {
                "host": And(str, len),
                "port": int,
                "from": And(str, len),
                Optional("password"): And(str, len),
                Optional("username"): And(str, len),
                Optional("ssl"): bool,
                Optional("tls"): bool,
            },
            Optional("slack"): {
                "webhook_url": And(str, len),
                Optional("username"): And(str, len),
                Optional("icon_url"): And(str, len),
                Optional("icon_emoji"): And(str, len),
            },
        },
    }
)
