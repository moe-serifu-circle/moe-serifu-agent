from msa.version import v as msa_version
import requests
import time


class MsaApi:
    def __init__(self, host="localhost", port=8080, script_mode=False):
        self.host = host
        self.port = port
        self.script_mode = script_mode
        self.base_url = "http://{}:{}".format(self.host, self.port)

    def _wrap_api_call(self, func, *args, **kwargs):
        n = 0
        fail = 3
        while True:
            try:
                return func(*args, **kwargs)
            except requests.exceptions.ConnectionError:
                n += 1

                if n == 1:
                    print("This is taking longer than expected, we seem to be having some connection troubles. Trying again.")
                elif n == 2:
                    print("Hmm, something must be up.")

            print("Unfortunately, I was unable to read the msa daemon instance at {}".format(self.base_url))
            print("Please check your connection and try again")
            return None

    def ping(self, quiet=False):
        response = self._wrap_api_call(requests.get, self.base_url + "/ping")

        if not response:
            return

        if response.status_code != 200:
            raise Exception(response.raw)

        if not quiet:
            print(response.text)

    def remote_command(self, text):
        response = self._wrap_api_call(
            requests.post,
            self.base_url + "/remote_command",
            data={"message": text})

        if not response:
            return

        if response.status_code != 200:
            raise Exception(response.raw)

        print(response.text)

    def check_version(self, quiet=False):
        response = self._wrap_api_call(requests.get, self.base_url + "/version")

        if not response:
            print("Unable to verify server version. Exiting.")
            # we have been unable to connect to the server and should exit in this case
            exit(1)

        if response.status_code != 200:
            raise Exception(response.raw)

        if response.text != msa_version or not quiet:
            print("Server Version:", response.text)
            print("Client Version:", msa_version)
            if response.text != msa_version:
                print("Warning: Client and server versions mismatch.\nExiting.", flush=True)
                exit(1)

    def check_connection(self):
        n = 0
        fail = 3
        while n < fail:
            try:
                self.ping(quiet=True)
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


        print("Unfortunately I was unable to reach the msa daemon instance at {}".format(self.base_url), flush=True)
        print("Exiting.", flush=True)
        exit(1)



