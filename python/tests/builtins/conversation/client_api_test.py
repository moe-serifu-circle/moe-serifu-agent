import unittest
from unittest.mock import MagicMock, AsyncMock, patch

from msa.builtins.conversation.client_api import talk
from msa.api.api_clients import ApiResponse

from msa.utils.asyncio_utils import run_async


class ClientApiTest(unittest.TestCase):
    @patch("builtins.print")
    def test_talk_success(self, printMock):

        response = "my response!"
        slf = MagicMock()
        slf.client.post = AsyncMock(
            return_value=ApiResponse({"status": "success", "text": response})
        )
        input_message = "hello, how are you?"

        run_async(talk(slf, input_message))

        printMock.assert_called_once_with(response)

    @patch("builtins.print")
    def test_talk_failed_bad_response(self, printMock):

        response = {"msg": "my response!"}
        slf = MagicMock()
        slf.client.post = AsyncMock(
            return_value=ApiResponse({"status": "success", "json": response})
        )
        input_message = "hello, how are you?"

        run_async(talk(slf, input_message))
        printMock.assert_called_once_with("It seems there was an issue.")

    def test_talk_failed_good_response(self):

        response = "fake error message"
        slf = MagicMock()
        slf.client.post = AsyncMock(
            return_value=ApiResponse({"status": "failed", "text": response})
        )
        input_message = "hello, how are you?"

        with self.assertRaises(Exception) as cm:
            run_async(talk(slf, input_message))

        the_exception = cm.exception
        self.assertEqual(str(the_exception), f"Server Error: \n{response}")
