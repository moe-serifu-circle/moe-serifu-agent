import threading


class Handler:
    """A primitive object for managing whether or not an
    event handler should run."""

    def __init__(self) -> None:
        """Set up a new Handler object."""
        self.suspend_lock = threading.Lock()
        self.resume_cond = threading.Condition(self.suspend_lock)
        self.suspend_flag = False
        self.in_wait_loop = False
        self.syscall_origin = False

    def dispose(self) -> None:
        """Dispose of this handler object's locks."""
        del self.resume_cond
        del self.suspend_lock

    def suspend(self) -> None:
        """Suspend the handler."""
        self.suspend_lock.acquire()
        self.suspend_flag = True
        self.suspend_lock.release()

    def resume(self) -> None:
        """Resume the handler."""
        self.suspend_lock.acquire()
        self.suspend_flag = False

        self.resume_cond.notify_all()
        self.suspend_lock.release()

    def suspended(self) -> bool:
        """Return a boolean indicating whether or not a handler
        is suspended and in a wait loop.

        This method does acquire a lock and as such may block.
        """
        self.suspend_lock.acquire()
        suspended = self.in_wait_loop
        self.suspend_lock.release()
        return suspended

    def set_syscall_origin(self) -> None:
        """Sets syscall_origin to True."""
        self.syscall_origin = True

    def clear_syscall_origin(self) -> None:
        """Sets syscall_origin to False."""
        self.syscall_origin = False

    def interrupt_point(self):
        """Indicate a point where the handler can be interrupted
        and suspended."""
        self.suspend_lock.acquire()
        while(self.suspend_flag):
            self.in_wait_loop = True
            self.resume_cond.wait()
        self.in_wait_loop = False
        self.suspend_lock.release()


