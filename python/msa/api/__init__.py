from msa.version import v as msa_version
import requests


class MsaApi:
    def __init__(self, host="localhost", port=8080):
        self.host = host
        self.port = port
        self.base_url = "http://{}:{}".format(self.host, self.port)

    def ping(self):
        response = requests.get(self.base_url + "/ping")

        if response.status_code != 200:
            raise Exception(response.raw)

        print(response.text)

    def check_version(self, quiet=False):
        response = requests.get(self.base_url + "/version")

        if response.status_code != 200:
            raise Exception(response.raw)

        if response.text != msa_version or not quiet:
            print("Server Version:", response.text)
            print("Client Version:", msa_version)
            if response.text != msa_version:
                print("Warning: Client and server versions mismatch.\nExiting.")
                exit(1)



