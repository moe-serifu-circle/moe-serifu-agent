# Python Port Asyncio Version

## CLI startup commands
- `python -m msa` : Starts msa in default cli mode
- `python -m msa cli` : Starts msa in default cli mode
- `python -m msa help` : lists availiable commands
- `python -m msa server`: starts msa server instance, try adding `--help` for more info
- `python -m msa client``: starts msa client instance, try adding `--help` for more info


# Run Modes
The client and server modes are intended for remote event propogation, run the server on one computer, and the client on another, and events will be propogated from the client, to the server, handled on the server, and propogated back to the client. This is useful if the client machine has limited resources.


