from msa.api import get_api
from msa.api.context import ApiContext
from msa.core.config_manager import ConfigManager
from msa.utils.asyncio_utils import run_async


class MsaRestClientLoader:
    def __init__(self, host="localhost", port=8080, config_overrides={}):
        self.host = host
        self.port = port
        self.config_manager = ConfigManager(config_overrides)
        self.config = self.config_manager.get_config()

        self.api = None

    def load(self):
        if self.api is not None:
            return self.api

        self.api = get_api(
            ApiContext.rest,
            self.config["plugin_modules"],
            host=self.host,
            port=self.port,
        )

        run_async(self.api.client.connect())

        return self.api
