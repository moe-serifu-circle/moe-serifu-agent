from typing import Dict, List
from tabulate import tabulate


async def upload_script(self, name, crontab=None, file_name=None, script_contents=None):
    """
    Uploads a script to the daemon instance.

    :async:
    :param name: The name the script should be referred to by on the daemon.
    :param crontab: (Optional) A crontab to run the script on. For help writing a crontab try https://crontab.guru/
    :param file_name: (Optional) A file name, relative to the current working directory, to upload to the daemon.
        If not provided, `script_contents` is required. The file must be UTF-8 encoded.
    :param script_contents: (Optional) A eval-able python string to send to the daemon as a script. If not provided,
        `file_name` is requied.
    :return: Response message from the daemon
    """
    if file_name and script_contents:
        raise Exception("MSA API - upload_script - cannot provide file_name and script_contents")

    if file_name:
        with open(file_name, "rb") as f:
            try:
                script_contents = f.read().decode("utf-8")
            except:
                raise Exception("MSA API - upload_script - failed to decode file, expects utf-8 encoding.")

    if crontab is not None:
        payload = {
            "name": name,
            "script_contents": script_contents,
            "crontab": crontab
        }
    else:
        payload = {
            "name": name,
            "script_contents": script_contents,
        }

    response = await self.client.post(
        "/scripting/script",
        payload=payload)

    if response.status != "success":
        raise Exception(response.json["message"])
    print(response.raw)


async def list_scripts(self) -> None:
    """
    Prints scripts that have been uploaded to the deamon.

    :return:
    :rtype: None
    """

    response = await self.client.get("/scripting/script")

    if response.status != "success":
        raise Exception(response.json["message"])

    scripts = response.json["scripts"]

    print(tabulate(scripts, tablefmt="fancy_grid", headers="keys"))


async def get_scripts(self) -> List[Dict]:
    """
    Fetches scripts uploaded to the daemon and returns them as a list of objects

    :param self:
    :return:
    """

    response = await self.client.get("/scripting/script")

    if response.status != "success":
        raise Exception(response.json["message"])

    scripts = response.json["scripts"]
    return scripts


def register_endpoints(api_binder):
    api_binder.register_method()(upload_script)
    api_binder.register_method()(list_scripts)
