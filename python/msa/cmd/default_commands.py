import msa
from msa.cmd import Command, Result, StdResultCode



class KillCommand(Command):
    def __init__(self):
        super().__init__("KILL", "It shuts down this MSA instance", "")


    def handler(self, handle, param_list, sync):
        print("Right away, $USER_TITLE, I will terminate my EDT for you now!")

        result_code = msa.stop()

        if result_code is not 0:

            if result_code is not StdResultCode.MSA_ERROR_LOG:
                print(f"Shutdown error: {result_code}")

            return Result(result_code=StdResultCode.ERROR, return_value=f"Shutdow error: {result_code}")

        return Result(result_code=StdResultCode.OK)


