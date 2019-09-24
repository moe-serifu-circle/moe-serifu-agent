import requests


def register_endpoints(api_binder):

    @api_binder.register_method()
    def upload_script(self, name, crontab, file_name=None, script_contents=None):

        if file_name and script_contents:
            raise Exception("MSA API - upload_script - cannot provide file_name and script_contents")

        if file_name:
            with open(file_name, "rb") as f:
                script_contents = f.read()

        response = self.rest_client.post(
            "/scripting/script",
            data={
                "name": name,
                "script_contents": script_contents,
                "crontab": crontab
            })

        if not response:
            return
        if response.status_code != 200:
            raise Exception(response.raw)
        print(response.text)

