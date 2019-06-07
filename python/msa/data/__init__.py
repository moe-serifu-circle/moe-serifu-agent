from sqlalchemy_aio import ASYNCIO_STRATEGY

from sqlalchemy import (
    Column, Integer, MetaData, Table, Text, create_engine, select)
from sqlalchemy.schema import CreateTable, DropTable


#https://pypi.org/project/sqlalchemy-aio/

async def start_db_engine(app):
    engine = create_engine(
        # In-memory sqlite database cannot be accessed from different
        # threads, use file.
        'sqlite:///msa.db', strategy=ASYNCIO_STRATEGY
    )

    app["db"] = engine    

