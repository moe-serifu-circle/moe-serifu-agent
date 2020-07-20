from msa.builtins.signals import handlers
from msa.builtins.signals import server_api
from msa.builtins.signals import client_api

handler_factories = [
    handlers.StartupEventTrigger,
    handlers.NetworkPropagateEventHandler,
]


# entity_setup = None

entities = []


register_client_api = client_api.register_endpoints
register_server_api = server_api.register_routes
