import requests

from msa.version import v as msa_version


def register_base_methods(api_wrapper):

    @api_wrapper.register_method()
    def ping(self, quiet=False):
        response = self.rest_client.get("/ping")

        if not response:
            return

        if response.status_code != 200:
            raise Exception(response.raw)

        if not quiet:
            print(response.text)

    @api_wrapper.register_method()
    def remote_command(self, text):
        response = self.rest_client.post("/remote_command", data={"message": text})

        if not response:
            return

        if response.status_code != 200:
            raise Exception(response.raw)

        print(response.text)

    @api_wrapper.register_method()
    def check_version(self, quiet=False):
        response = self.rest_client.get("/version")

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

    @api_wrapper.register_method()
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
