import unittest
import threading
import time

from msa.event.handler_sync import HandlerSync

class HandlerSyncTest(unittest.TestCase):

    def setUp(self):
        self.handler = HandlerSync()

    def test_suspend(self):

        assert not self.handler.suspend_flag

        self.handler.suspend()
        assert self.handler.suspend_flag

        self.handler.suspend()
        assert self.handler.suspend_flag

    def test_resume(self):
        assert not self.handler.suspend_flag

        self.handler.suspend()
        assert self.handler.suspend_flag

        self.handler.resume()
        assert not self.handler.suspend_flag

    def test_suspended__suspend_not_called(self):

        assert not self.handler.suspended()

        t = threading.Thread(target=lambda: self.handler.allow_pending_suspension())
        t.start()

        # wait to give the thread a chance to start
        time.sleep(0.1)

        assert not self.handler.suspended()

        # wait for thread to exit
        t.join()


    def test_suspended_and_allow_pending_suspensions__suspend_called(self):

        assert not self.handler.suspended()

        assert not self.handler.suspend_flag
        self.handler.suspend()
        assert self.handler.suspend_flag

        assert not self.handler.suspended()

        t = threading.Thread(target=lambda: self.handler.allow_pending_suspension())
        t.start()

        # wait to give the thread a chance to start
        # and enter wait loop
        time.sleep(0.1)

        assert self.handler.suspended()

        # resume so we can exit thread
        self.handler.resume()

        # wait for thread to exit
        t.join()


    def test_set_skip_edit_join(self):

        # Set syscall origin so we can clear it
        assert not self.handler.syscall_origin
        self.handler.set_skip_edt_join(True)
        assert self.handler.syscall_origin

        # clear syscall origin
        self.handler.set_skip_edt_join(False)
        assert not self.handler.syscall_origin





