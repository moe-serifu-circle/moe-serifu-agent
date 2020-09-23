import requests
import time

from msa.version import v as msa_version


async def ping(self, quiet=False):
    """
    Send's a ping to the daemon. If the ping succeeds, you should see a pong response.

    :async:
    :param bool quiet: Print's the response of `True`.
    :return: `None`
    """
    response = await self.client.get("/ping")

    if not response:
        return

    if response.status != "success":
        raise Exception(response.raw)

    if not quiet:
        print(response.text)


async def check_version(self, quiet=False):
    """
    Called automatically at startup, ensures that the cli version and the daemon versions match.

    :async:
    :param bool quiet: Print's the response of `True`.
    :return: `None`
    """
    response = await self.client.get("/version")

    if not response:
        print("Unable to verify server version. Exiting.")
        # we have been unable to connect to the server and should exit in this case
        exit(1)

    if response.status != "success":
        raise Exception(response.raw)

    if response.text != msa_version or not quiet:
        server_version = response.json["text"]
        print("Server Version:", server_version)
        print("Client Version:", msa_version)
        if server_version != msa_version:
            print("Warning: Client and server versions mismatch.\nExiting.", flush=True)
            exit(1)


async def get_version(self):
    """
    Fetches the daemon's version.

    :async:
    :return: `None`
    """
    response = await self.client.get("/version")

    if not response:
        print("Unable to verify server version. Exiting.")
        # we have been unable to connect to the server and should exit in this case
        exit(1)

    if response.status != "success":
        raise Exception(response.raw)

    return response.text


async def check_connection(self):
    """
    Raises an exception if the cli cannot contact the daemon.

    :async:
    :return: `None`
    """
    n = 0
    fail = 3
    while n < fail:
        try:
            await self.ping(quiet=True)
            if n > 0:
                print("There we go! Connection succeeded!")
            return
        except requests.exceptions.ConnectionError:
            n += 1

            if n == 1:
                print("Having trouble connecting... Trying again.")
            elif n == 2:
                print("Hmm, something must be up.")
            time.sleep(2)

    print(
        "Unfortunately I was unable to reach the msa daemon instance at {}".format(
            self.base_url
        ),
        flush=True,
    )
    print("Exiting.", flush=True)
    exit(1)


def register_base_methods(api_wrapper):

    api_wrapper.register_method()(ping)
    api_wrapper.register_method()(check_version)
    api_wrapper.register_method()(get_version)
    api_wrapper.register_method()(check_connection)
